
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkReadBuffer_DEFINED
#define SkReadBuffer_DEFINED

#include "SkBitmapHeap.h"
#include "SkColorFilter.h"
#include "SkData.h"
#include "SkDrawLooper.h"
#include "SkImageFilter.h"
#include "SkMaskFilter.h"
#include "SkPath.h"
#include "SkPathEffect.h"
#include "SkPicture.h"
#include "SkPixelRef.h"
#include "SkRasterizer.h"
#include "SkReadBuffer.h"
#include "SkReader32.h"
#include "SkRefCnt.h"
#include "SkShader.h"
#include "SkUnitMapper.h"
#include "SkWriteBuffer.h"
#include "SkXfermode.h"

class SkBitmap;

#if defined(SK_DEBUG) && defined(SK_BUILD_FOR_MAC)
    #define DEBUG_NON_DETERMINISTIC_ASSERT
#endif

class SkReadBuffer {
public:
    SkReadBuffer();
    SkReadBuffer(const void* data, size_t size);
    SkReadBuffer(SkStream* stream);
    virtual ~SkReadBuffer();

    enum Flags {
        kCrossProcess_Flag  = 1 << 0,
        kScalarIsFloat_Flag = 1 << 1,
        kPtrIs64Bit_Flag    = 1 << 2,
        kValidation_Flag    = 1 << 3,
    };

    void setFlags(uint32_t flags) { fFlags = flags; }
    uint32_t getFlags() const { return fFlags; }

    bool isCrossProcess() const {
        return this->isValidating() || SkToBool(fFlags & kCrossProcess_Flag);
    }
    bool isScalarFloat() const { return SkToBool(fFlags & kScalarIsFloat_Flag); }
    bool isPtr64Bit() const { return SkToBool(fFlags & kPtrIs64Bit_Flag); }
    bool isValidating() const { return SkToBool(fFlags & kValidation_Flag); }

    SkReader32* getReader32() { return &fReader; }

    uint32_t size() { return fReader.size(); }
    uint32_t offset() { return fReader.offset(); }
    bool eof() { return fReader.eof(); }
    const void* skip(size_t size) { return fReader.skip(size); }

    // primitives
    virtual bool readBool();
    virtual SkColor readColor();
    virtual SkFixed readFixed();
    virtual int32_t readInt();
    virtual SkScalar readScalar();
    virtual uint32_t readUInt();
    virtual int32_t read32();

    void* readFunctionPtr() {
        void* ptr;
        this->readByteArray(&ptr, sizeof(ptr));
        return ptr;
    }

    // strings -- the caller is responsible for freeing the string contents
    virtual void readString(SkString* string);
    virtual void* readEncodedString(size_t* length, SkPaint::TextEncoding encoding);

    // common data structures
    virtual void readPoint(SkPoint* point);
    SkPoint readPoint() { SkPoint p; this->readPoint(&p); return p; }
    virtual void readMatrix(SkMatrix* matrix);
    virtual void readIRect(SkIRect* rect);
    virtual void readRect(SkRect* rect);
    virtual void readRegion(SkRegion* region);
    virtual void readPath(SkPath* path);
    void readPaint(SkPaint* paint) { paint->unflatten(*this); }

    virtual SkFlattenable* readFlattenable(SkFlattenable::Type);
    template <typename T> T* readFlattenable() {
        return (T*) this->readFlattenable(T::GetFlattenableType());
    }
    SkColorFilter* readColorFilter() { return this->readFlattenable<SkColorFilter>(); }
    SkDrawLooper*  readDrawLooper()  { return this->readFlattenable<SkDrawLooper>(); }
    SkImageFilter* readImageFilter() { return this->readFlattenable<SkImageFilter>(); }
    SkMaskFilter*  readMaskFilter()  { return this->readFlattenable<SkMaskFilter>(); }
    SkPathEffect*  readPathEffect()  { return this->readFlattenable<SkPathEffect>(); }
    SkPixelRef*    readPixelRef()    { return this->readFlattenable<SkPixelRef>(); }
    SkRasterizer*  readRasterizer()  { return this->readFlattenable<SkRasterizer>(); }
    SkShader*      readShader()      { return this->readFlattenable<SkShader>(); }
    SkUnitMapper*  readUnitMapper()  { return this->readFlattenable<SkUnitMapper>(); }
    SkXfermode*    readXfermode()    { return this->readFlattenable<SkXfermode>(); }


    // binary data and arrays
    virtual bool readByteArray(void* value, size_t size);
    virtual bool readColorArray(SkColor* colors, size_t size);
    virtual bool readIntArray(int32_t* values, size_t size);
    virtual bool readPointArray(SkPoint* points, size_t size);
    virtual bool readScalarArray(SkScalar* values, size_t size);

    SkData* readByteArrayAsData() {
        size_t len = this->getArrayCount();
        if (!this->validateAvailable(len)) {
            return SkData::NewEmpty();
        }
        void* buffer = sk_malloc_throw(len);
        this->readByteArray(buffer, len);
        return SkData::NewFromMalloc(buffer, len);
    }

    // helpers to get info about arrays and binary data
    virtual uint32_t getArrayCount();

    virtual void readBitmap(SkBitmap* bitmap);
    virtual SkTypeface* readTypeface();

    void setBitmapStorage(SkBitmapHeapReader* bitmapStorage) {
        SkRefCnt_SafeAssign(fBitmapStorage, bitmapStorage);
    }

    void setTypefaceArray(SkTypeface* array[], int count) {
        fTFArray = array;
        fTFCount = count;
    }

    /**
     *  Call this with a pre-loaded array of Factories, in the same order as
     *  were created/written by the writer. SkPicture uses this.
     */
    void setFactoryPlayback(SkFlattenable::Factory array[], int count) {
        fFactoryTDArray = NULL;
        fFactoryArray = array;
        fFactoryCount = count;
    }

    /**
     *  Call this with an initially empty array, so the reader can cache each
     *  factory it sees by name. Used by the pipe code in conjunction with
     *  SkWriteBuffer::setNamedFactoryRecorder.
     */
    void setFactoryArray(SkTDArray<SkFlattenable::Factory>* array) {
        fFactoryTDArray = array;
        fFactoryArray = NULL;
        fFactoryCount = 0;
    }

    /**
     *  Provide a function to decode an SkBitmap from encoded data. Only used if the writer
     *  encoded the SkBitmap. If the proper decoder cannot be used, a red bitmap with the
     *  appropriate size will be used.
     */
    void setBitmapDecoder(SkPicture::InstallPixelRefProc bitmapDecoder) {
        fBitmapDecoder = bitmapDecoder;
    }

    // Default impelementations don't check anything.
    virtual bool validate(bool isValid) { return true; }
    virtual bool isValid() const { return true; }
    virtual bool validateAvailable(size_t size) { return true; }

private:
    bool readArray(void* value, size_t size, size_t elementSize);

    uint32_t fFlags;

    SkReader32 fReader;
    void* fMemoryPtr;

    SkBitmapHeapReader* fBitmapStorage;
    SkTypeface** fTFArray;
    int        fTFCount;

    SkTDArray<SkFlattenable::Factory>* fFactoryTDArray;
    SkFlattenable::Factory* fFactoryArray;
    int                     fFactoryCount;

    SkPicture::InstallPixelRefProc fBitmapDecoder;

#ifdef DEBUG_NON_DETERMINISTIC_ASSERT
    // Debugging counter to keep track of how many bitmaps we
    // have decoded.
    int fDecodedBitmapIndex;
#endif // DEBUG_NON_DETERMINISTIC_ASSERT
};

#endif // SkReadBuffer_DEFINED
