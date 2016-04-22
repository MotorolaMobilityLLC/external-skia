/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkWriteBuffer.h"
#include "SkBitmap.h"
#include "SkBitmapHeap.h"
#include "SkData.h"
#include "SkPixelRef.h"
#include "SkPtrRecorder.h"
#include "SkStream.h"
#include "SkTypeface.h"

SkWriteBuffer::SkWriteBuffer(uint32_t flags)
    : fFlags(flags)
    , fFactorySet(nullptr)
    , fBitmapHeap(nullptr)
    , fTFSet(nullptr) {
}

SkWriteBuffer::SkWriteBuffer(void* storage, size_t storageSize, uint32_t flags)
    : fFlags(flags)
    , fFactorySet(nullptr)
    , fWriter(storage, storageSize)
    , fBitmapHeap(nullptr)
    , fTFSet(nullptr) {
}

SkWriteBuffer::~SkWriteBuffer() {
    SkSafeUnref(fFactorySet);
    SkSafeUnref(fBitmapHeap);
    SkSafeUnref(fTFSet);
}

void SkWriteBuffer::writeByteArray(const void* data, size_t size) {
    fWriter.write32(SkToU32(size));
    fWriter.writePad(data, size);
}

void SkWriteBuffer::writeBool(bool value) {
    fWriter.writeBool(value);
}

void SkWriteBuffer::writeScalar(SkScalar value) {
    fWriter.writeScalar(value);
}

void SkWriteBuffer::writeScalarArray(const SkScalar* value, uint32_t count) {
    fWriter.write32(count);
    fWriter.write(value, count * sizeof(SkScalar));
}

void SkWriteBuffer::writeInt(int32_t value) {
    fWriter.write32(value);
}

void SkWriteBuffer::writeIntArray(const int32_t* value, uint32_t count) {
    fWriter.write32(count);
    fWriter.write(value, count * sizeof(int32_t));
}

void SkWriteBuffer::writeUInt(uint32_t value) {
    fWriter.write32(value);
}

void SkWriteBuffer::write32(int32_t value) {
    fWriter.write32(value);
}

void SkWriteBuffer::writeString(const char* value) {
    fWriter.writeString(value);
}

void SkWriteBuffer::writeEncodedString(const void* value, size_t byteLength,
                                              SkPaint::TextEncoding encoding) {
    fWriter.writeInt(encoding);
    fWriter.writeInt(SkToU32(byteLength));
    fWriter.write(value, byteLength);
}


void SkWriteBuffer::writeColor(const SkColor& color) {
    fWriter.write32(color);
}

void SkWriteBuffer::writeColorArray(const SkColor* color, uint32_t count) {
    fWriter.write32(count);
    fWriter.write(color, count * sizeof(SkColor));
}

void SkWriteBuffer::writePoint(const SkPoint& point) {
    fWriter.writeScalar(point.fX);
    fWriter.writeScalar(point.fY);
}

void SkWriteBuffer::writePointArray(const SkPoint* point, uint32_t count) {
    fWriter.write32(count);
    fWriter.write(point, count * sizeof(SkPoint));
}

void SkWriteBuffer::writeMatrix(const SkMatrix& matrix) {
    fWriter.writeMatrix(matrix);
}

void SkWriteBuffer::writeIRect(const SkIRect& rect) {
    fWriter.write(&rect, sizeof(SkIRect));
}

void SkWriteBuffer::writeRect(const SkRect& rect) {
    fWriter.writeRect(rect);
}

void SkWriteBuffer::writeRegion(const SkRegion& region) {
    fWriter.writeRegion(region);
}

void SkWriteBuffer::writePath(const SkPath& path) {
    fWriter.writePath(path);
}

size_t SkWriteBuffer::writeStream(SkStream* stream, size_t length) {
    fWriter.write32(SkToU32(length));
    size_t bytesWritten = fWriter.readFromStream(stream, length);
    if (bytesWritten < length) {
        fWriter.reservePad(length - bytesWritten);
    }
    return bytesWritten;
}

bool SkWriteBuffer::writeToStream(SkWStream* stream) {
    return fWriter.writeToStream(stream);
}

static void write_encoded_bitmap(SkWriteBuffer* buffer, SkData* data,
                                 const SkIPoint& origin) {
    buffer->writeUInt(SkToU32(data->size()));
    buffer->getWriter32()->writePad(data->data(), data->size());
    buffer->write32(origin.fX);
    buffer->write32(origin.fY);
}

void SkWriteBuffer::writeBitmap(const SkBitmap& bitmap) {
    // Record the width and height. This way if readBitmap fails a dummy bitmap can be drawn at the
    // right size.
    this->writeInt(bitmap.width());
    this->writeInt(bitmap.height());

    // Record information about the bitmap in one of three ways, in order of priority:
    // 1. If there is an SkBitmapHeap, store it in the heap. The client can avoid serializing the
    //    bitmap entirely or serialize it later as desired. A boolean value of true will be written
    //    to the stream to signify that a heap was used.
    // 2. If there is a function for encoding bitmaps, use it to write an encoded version of the
    //    bitmap. After writing a boolean value of false, signifying that a heap was not used, write
    //    the size of the encoded data. A non-zero size signifies that encoded data was written.
    // 3. Call SkBitmap::flatten. After writing a boolean value of false, signifying that a heap was
    //    not used, write a zero to signify that the data was not encoded.
    bool useBitmapHeap = fBitmapHeap != nullptr;
    // Write a bool: true if the SkBitmapHeap is to be used, in which case the reader must use an
    // SkBitmapHeapReader to read the SkBitmap. False if the bitmap was serialized another way.
    this->writeBool(useBitmapHeap);
    if (useBitmapHeap) {
        SkASSERT(nullptr == fPixelSerializer);
        int32_t slot = fBitmapHeap->insert(bitmap);
        fWriter.write32(slot);
        // crbug.com/155875
        // The generation ID is not required information. We write it to prevent collisions
        // in SkFlatDictionary.  It is possible to get a collision when a previously
        // unflattened (i.e. stale) instance of a similar flattenable is in the dictionary
        // and the instance currently being written is re-using the same slot from the
        // bitmap heap.
        fWriter.write32(bitmap.getGenerationID());
        return;
    }

    SkPixelRef* pixelRef = bitmap.pixelRef();
    if (pixelRef) {
        // see if the pixelref already has an encoded version
        SkAutoDataUnref existingData(pixelRef->refEncodedData());
        if (existingData.get() != nullptr) {
            // Assumes that if the client did not set a serializer, they are
            // happy to get the encoded data.
            if (!fPixelSerializer || fPixelSerializer->useEncodedData(existingData->data(),
                                                                      existingData->size())) {
                write_encoded_bitmap(this, existingData, bitmap.pixelRefOrigin());
                return;
            }
        }

        // see if the caller wants to manually encode
        SkAutoPixmapUnlock result;
        if (fPixelSerializer && bitmap.requestLock(&result)) {
            SkASSERT(nullptr == fBitmapHeap);
            SkAutoDataUnref data(fPixelSerializer->encode(result.pixmap()));
            if (data.get() != nullptr) {
                // if we have to "encode" the bitmap, then we assume there is no
                // offset to share, since we are effectively creating a new pixelref
                write_encoded_bitmap(this, data, SkIPoint::Make(0, 0));
                return;
            }
        }
    }

    this->writeUInt(0); // signal raw pixels
    SkBitmap::WriteRawPixels(this, bitmap);
}

void SkWriteBuffer::writeImage(const SkImage* image) {
    this->writeInt(image->width());
    this->writeInt(image->height());

    SkAutoTUnref<SkData> encoded(image->encode(this->getPixelSerializer()));
    if (encoded && encoded->size() > 0) {
        write_encoded_bitmap(this, encoded, SkIPoint::Make(0, 0));
        return;
    }

    this->writeUInt(0); // signal no pixels (in place of the size of the encoded data)
}

void SkWriteBuffer::writeTypeface(SkTypeface* obj) {
    if (nullptr == obj || nullptr == fTFSet) {
        fWriter.write32(0);
    } else {
        fWriter.write32(fTFSet->add(obj));
    }
}

SkFactorySet* SkWriteBuffer::setFactoryRecorder(SkFactorySet* rec) {
    SkRefCnt_SafeAssign(fFactorySet, rec);
    return rec;
}

SkRefCntSet* SkWriteBuffer::setTypefaceRecorder(SkRefCntSet* rec) {
    SkRefCnt_SafeAssign(fTFSet, rec);
    return rec;
}

void SkWriteBuffer::setBitmapHeap(SkBitmapHeap* bitmapHeap) {
    SkRefCnt_SafeAssign(fBitmapHeap, bitmapHeap);
    if (bitmapHeap != nullptr) {
        SkASSERT(nullptr == fPixelSerializer);
        fPixelSerializer.reset(nullptr);
    }
}

void SkWriteBuffer::setPixelSerializer(SkPixelSerializer* serializer) {
    fPixelSerializer.reset(serializer);
    if (serializer) {
        serializer->ref();
        SkASSERT(nullptr == fBitmapHeap);
        SkSafeUnref(fBitmapHeap);
        fBitmapHeap = nullptr;
    }
}

void SkWriteBuffer::writeFlattenable(const SkFlattenable* flattenable) {
    /*
     *  The first 32 bits tell us...
     *       0: failure to write the flattenable
     *      >0: index (1-based) into fFactorySet or fFlattenableDict or
     *          the first character of a string
     */
    if (nullptr == flattenable) {
        this->write32(0);
        return;
    }

    /*
     *  We can write 1 of 2 versions of the flattenable:
     *  1.  index into fFactorySet : This assumes the writer will later
     *      resolve the function-ptrs into strings for its reader. SkPicture
     *      does exactly this, by writing a table of names (matching the indices)
     *      up front in its serialized form.
     *  2.  string name of the flattenable or index into fFlattenableDict:  We
     *      store the string to allow the reader to specify its own factories
     *      after write time.  In order to improve compression, if we have
     *      already written the string, we write its index instead.
     */
    if (fFactorySet) {
        SkFlattenable::Factory factory = flattenable->getFactory();
        SkASSERT(factory);
        this->write32(fFactorySet->add(factory));
    } else {
        const char* name = flattenable->getTypeName();
        SkASSERT(name);
        SkString key(name);
        if (uint32_t* indexPtr = fFlattenableDict.find(key)) {
            // We will write the index as a 32-bit int.  We want the first byte
            // that we send to be zero - this will act as a sentinel that we
            // have an index (not a string).  This means that we will send the
            // the index shifted left by 8.  The remaining 24-bits should be
            // plenty to store the index.  Note that this strategy depends on
            // being little endian.
            SkASSERT(0 == *indexPtr >> 24);
            this->write32(*indexPtr << 8);
        } else {
            // Otherwise write the string.  Clients should not use the empty
            // string as a name, or we will have a problem.
            SkASSERT(strcmp("", name));
            this->writeString(name);

            // Add key to dictionary.
            fFlattenableDict.set(key, fFlattenableDict.count() + 1);
        }
    }

    // make room for the size of the flattened object
    (void)fWriter.reserve(sizeof(uint32_t));
    // record the current size, so we can subtract after the object writes.
    size_t offset = fWriter.bytesWritten();
    // now flatten the object
    flattenable->flatten(*this);
    size_t objSize = fWriter.bytesWritten() - offset;
    // record the obj's size
    fWriter.overwriteTAt(offset - sizeof(uint32_t), SkToU32(objSize));
}
