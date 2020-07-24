/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/GrRingBuffer.h"

#include "src/gpu/GrContextPriv.h"
#include "src/gpu/GrGpu.h"
#include "src/gpu/GrResourceProvider.h"

// Get offset into buffer that has enough space for size
// Returns fTotalSize if no space
size_t GrRingBuffer::getAllocationOffset(size_t size) {
    // capture current state locally (because fTail could be overwritten by the completion handler)
    size_t head, tail;
    head = fHead;
    tail = fTail;

    // The head and tail indices increment without bound, wrapping with overflow,
    // so we need to mod them down to the actual bounds of the allocation to determine
    // which blocks are available.
    size_t modHead = head & (fTotalSize - 1);
    size_t modTail = tail & (fTotalSize - 1);

    bool full = (head != tail && modHead == modTail);

    if (full) {
        return fTotalSize;
    }

    // case 1: free space lies at the beginning and/or the end of the buffer
    if (modHead >= modTail) {
        // check for room at the end
        if (fTotalSize - modHead < size) {
            // no room at the end, check the beginning
            if (modTail < size) {
                // no room at the beginning
                return fTotalSize;
            }
            // we are going to allocate from the beginning, adjust head to '0' position
            head += fTotalSize - modHead;
            modHead = 0;
        }
        // case 2: free space lies in the middle of the buffer, check for room there
    } else if (modTail - modHead < size) {
        // no room in the middle
        return fTotalSize;
    }

    fHead = GrAlignTo(head + size, fAlignment);
    return modHead;
}

GrRingBuffer::Slice GrRingBuffer::suballocate(size_t size) {
    if (fCurrentBuffer) {
        size_t offset = this->getAllocationOffset(size);
        if (offset < fTotalSize) {
            return { fCurrentBuffer.get(), offset };
        }

        // Try to grow allocation (old allocation will age out).
        fTotalSize *= 2;
    }

    GrResourceProvider* resourceProvider = fGpu->getContext()->priv().resourceProvider();
    fCurrentBuffer = resourceProvider->createBuffer(fTotalSize, fType, kDynamic_GrAccessPattern);

    SkASSERT(fCurrentBuffer);
    fTrackedBuffers.push_back(fCurrentBuffer);
    fHead = 0;
    fTail = 0;
    fGenID++;
    size_t offset = this->getAllocationOffset(size);
    SkASSERT(offset < fTotalSize);
    return { fCurrentBuffer.get(), offset };
}

// used when current command buffer/command list is submitted
void GrRingBuffer::startSubmit(GrRingBuffer::SubmitData* submitData) {
    submitData->fTrackedBuffers = std::move(fTrackedBuffers);
    submitData->fLastHead = fHead;
    submitData->fGenID = fGenID;
    // add current buffer to be tracked for next submit
    fTrackedBuffers.push_back(fCurrentBuffer);
}

// used when current command buffer/command list is completed
void GrRingBuffer::finishSubmit(const GrRingBuffer::SubmitData& submitData) {
    if (submitData.fGenID == fGenID) {
        fTail = submitData.fLastHead;
    }
}
