/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrGpuResourceRef.h"
#include "GrGpuResource.h"

GrGpuResourceRef::GrGpuResourceRef() {
    fResource = NULL;
    fOwnRef = false;
    fPendingIO = false;
    fIOType = kNone_IOType;
}

GrGpuResourceRef::GrGpuResourceRef(GrGpuResource* resource, IOType ioType) {
    fResource = NULL;
    fOwnRef = false;
    fPendingIO = false;
    this->setResource(resource, ioType);
}

GrGpuResourceRef::~GrGpuResourceRef() {
    if (fOwnRef) {
        SkASSERT(fResource);
        fResource->unref();
    }
    if (fPendingIO) {
        switch (fIOType) {
            case kNone_IOType:
                SkFAIL("Shouldn't get here if fIOType is kNone.");
                break;
            case kRead_IOType:
                fResource->completedRead();
                break;
            case kWrite_IOType:
                fResource->completedWrite();
                break;
            case kRW_IOType:
                fResource->completedRead();
                fResource->completedWrite();
                break;
        }
    }
}

void GrGpuResourceRef::reset() {
    SkASSERT(!fPendingIO);
    SkASSERT(SkToBool(fResource) == fOwnRef);
    if (fOwnRef) {
        fResource->unref();
        fOwnRef = false;
        fResource = NULL;
        fIOType = kNone_IOType;
    }
}

void GrGpuResourceRef::setResource(GrGpuResource* resource, IOType ioType) {
    SkASSERT(!fPendingIO);
    SkASSERT(SkToBool(fResource) == fOwnRef);
    SkSafeUnref(fResource);
    if (NULL == resource) {
        fResource = NULL;
        fOwnRef = false;
        fIOType = kNone_IOType;
    } else {
        SkASSERT(kNone_IOType != ioType);
        fResource = resource;
        fOwnRef = true;
        fIOType = ioType;
    }
}

void GrGpuResourceRef::markPendingIO() const {
    // This should only be called when the owning GrProgramElement gets its first
    // pendingExecution ref.
    SkASSERT(!fPendingIO);
    SkASSERT(fResource);
    fPendingIO = true;
    switch (fIOType) {
        case kNone_IOType:
            SkFAIL("GrGpuResourceRef with neither reads nor writes?");
            break;
        case kRead_IOType:
            fResource->addPendingRead();
            break;
        case kWrite_IOType:
            fResource->addPendingWrite();
            break;
        case kRW_IOType:
            fResource->addPendingRead();
            fResource->addPendingWrite();
            break;
    }
}

void GrGpuResourceRef::pendingIOComplete() const {
    // This should only be called when the owner's pending executions have ocurred but it is still
    // reffed.
    SkASSERT(fOwnRef);
    SkASSERT(fPendingIO);
    switch (fIOType) {
        case kNone_IOType:
            SkFAIL("GrGpuResourceRef with neither reads nor writes?");
            break;
        case kRead_IOType:
            fResource->completedRead();
            break;
        case kWrite_IOType:
            fResource->completedWrite();
            break;
        case kRW_IOType:
            fResource->completedRead();
            fResource->completedWrite();
            break;

    }
    fPendingIO = false;
}

void GrGpuResourceRef::removeRef() const {
    // This should only be called once, when the owners last ref goes away and
    // there is a pending execution.
    SkASSERT(fOwnRef);
    SkASSERT(fPendingIO);
    SkASSERT(kNone_IOType != fIOType);
    SkASSERT(fResource);
    fResource->unref();
    fOwnRef = false;
}
