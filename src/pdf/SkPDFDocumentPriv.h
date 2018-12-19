/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkPDFDocumentPriv_DEFINED
#define SkPDFDocumentPriv_DEFINED

#include "SkCanvas.h"
#include "SkPDFCanon.h"
#include "SkPDFDocument.h"
#include "SkPDFFont.h"
#include "SkPDFMetadata.h"
#include "SkPDFTag.h"
#include "SkUUID.h"

#include <atomic>

class SkPDFDevice;
class SkExecutor;

const char* SkPDFGetNodeIdKey();

struct SkPDFFileOffset {
    int fValue;
};

struct SkPDFOffsetMap {
    void set(SkPDFIndirectReference, SkPDFFileOffset);
    SkPDFFileOffset get(SkPDFIndirectReference r);
    std::vector<SkPDFFileOffset> fOffsets;
};

// Logically part of SkPDFDocument (like SkPDFCanon), but separate to
// keep similar functionality together.
struct SkPDFObjectSerializer {
    SkPDFOffsetMap fOffsets;
    size_t fBaseOffset = SIZE_MAX;

    SkPDFObjectSerializer();
    ~SkPDFObjectSerializer();

    void beginObject(SkPDFIndirectReference, SkWStream*);
    void endObject(SkWStream*);
    void serializeHeader(SkWStream*);
    void serializeFooter(SkWStream*,
                         SkPDFIndirectReference infoDict,
                         SkPDFIndirectReference docCatalog,
                         SkUUID uuid);
    SkPDFFileOffset offset(SkWStream*);

    SkPDFObjectSerializer(SkPDFObjectSerializer&&) = delete;
    SkPDFObjectSerializer& operator=(SkPDFObjectSerializer&&) = delete;
    SkPDFObjectSerializer(const SkPDFObjectSerializer&) = delete;
    SkPDFObjectSerializer& operator=(const SkPDFObjectSerializer&) = delete;
};

/** Concrete implementation of SkDocument that creates PDF files. This
    class does not produced linearized or optimized PDFs; instead it
    it attempts to use a minimum amount of RAM. */
class SkPDFDocument : public SkDocument {
public:
    SkPDFDocument(SkWStream*, SkPDF::Metadata);
    ~SkPDFDocument() override;
    SkCanvas* onBeginPage(SkScalar, SkScalar) override;
    void onEndPage() override;
    void onClose(SkWStream*) override;
    void onAbort() override;

    /**
       Serialize the object, as well as any other objects it
       indirectly refers to.  If any any other objects have been added
       to the SkPDFObjNumMap without serializing them, they will be
       serialized as well.

       It might go without saying that objects should not be changed
       after calling serialize, since those changes will be too late.
     */
//    SkPDFIndirectReference serialize(const sk_sp<SkPDFObject>&);
    SkPDFIndirectReference emit(const SkPDFObject&, SkPDFIndirectReference);
    SkPDFIndirectReference emit(const SkPDFObject& o) { return this->emit(o, this->reserveRef()); }
    SkPDFCanon* canon() { return &fCanon; }
    const SkPDF::Metadata& metadata() const { return fMetadata; }

    SkPDFIndirectReference getPage(size_t pageIndex) const;
    // Returns -1 if no mark ID.
    int getMarkIdForNodeId(int nodeId);

    SkPDFIndirectReference reserveRef() { return SkPDFIndirectReference{fNextObjectNumber++}; }
    SkWStream* beginObject(SkPDFIndirectReference);
    void endObject();

    SkExecutor* executor() const { return fExecutor; }
    size_t currentPageIndex() { return fPages.size(); }
    size_t pageCount() { return fPageRefs.size(); }

private:
    SkPDFObjectSerializer fObjectSerializer;
    SkPDFCanon fCanon;
    SkCanvas fCanvas;
    std::vector<sk_sp<SkPDFDict>> fPages;
    std::vector<SkPDFIndirectReference> fPageRefs;
    SkPDFDict fDests;
    sk_sp<SkPDFDevice> fPageDevice;
    std::atomic<int> fNextObjectNumber = {1};
    SkUUID fUUID;
    SkPDFIndirectReference fInfoDict;
    SkPDFIndirectReference fXMP;
    SkPDF::Metadata fMetadata;
    SkScalar fRasterScale = 1;
    SkScalar fInverseRasterScale = 1;
    SkExecutor* fExecutor = nullptr;

    // For tagged PDFs.
    SkPDFTagTree fTagTree;

    SkMutex fMutex;
    SkSemaphore fSemaphore;

    void reset();
};

#endif  // SkPDFDocumentPriv_DEFINED
