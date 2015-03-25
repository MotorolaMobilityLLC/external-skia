
/*
 * Copyright 2010 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkPDFPage_DEFINED
#define SkPDFPage_DEFINED

#include "SkPDFTypes.h"
#include "SkPDFStream.h"
#include "SkRefCnt.h"
#include "SkTDArray.h"

class SkPDFCatalog;
class SkPDFDevice;
class SkWStream;

/** \class SkPDFPage

    A SkPDFPage contains meta information about a page, is used in the page
    tree and points to the content of the page.
*/
class SkPDFPage : public SkPDFDict {
    SK_DECLARE_INST_COUNT(SkPDFPage)
public:
    /** Create a PDF page with the passed PDF device.  The device need not
     *  have content on it yet.
     *  @param content    The page content.
     */
    explicit SkPDFPage(const SkPDFDevice* content);
    ~SkPDFPage();

    /** Before a page and its contents can be sized and emitted, it must
     *  be finalized.  No changes to the PDFDevice will be honored after
     *  finalizePage has been called.
     */
    void finalizePage();

    /** Add destinations for this page to the supplied dictionary.
     *  @param dict       Dictionary to add destinations to.
     */
    void appendDestinations(SkPDFDict* dict);

    /** Generate a page tree for the passed vector of pages.  New objects are
     *  added to the catalog.  The pageTree vector is populated with all of
     *  the 'Pages' dictionaries as well as the 'Page' objects.  Page trees
     *  have both parent and children links, creating reference cycles, so
     *  it must be torn down explicitly.  The first page is not added to
     *  the pageTree dictionary array so the caller can handle it specially.
     *  @param pages      The ordered vector of page objects.
     *  @param pageTree   An output vector with all of the internal and leaf
     *                    nodes of the pageTree.
     *  @param rootNode   An output parameter set to the root node.
     */
    static void GeneratePageTree(const SkTDArray<SkPDFPage*>& pages,
                                 SkTDArray<SkPDFDict*>* pageTree,
                                 SkPDFDict** rootNode);

    /** Get the fonts used on this page.
     */
    const SkTDArray<SkPDFFont*>& getFontResources() const;

    /** Returns a SkPDFGlyphSetMap which represents glyph usage of every font
     *  that shows on this page.
     */
    const SkPDFGlyphSetMap& getFontGlyphUsage() const;

    SkPDFObject* getContentStream() const;

private:
    // Multiple pages may reference the content.
    SkAutoTUnref<const SkPDFDevice> fDevice;

    // Once the content is finalized, put it into a stream for output.
    SkAutoTUnref<SkPDFStream> fContentStream;
    typedef SkPDFDict INHERITED;
};

#endif
