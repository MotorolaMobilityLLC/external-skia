/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkFontMgr_FontConfigInterface_DEFINED
#define SkFontMgr_FontConfigInterface_DEFINED

#include "SkTypes.h"
#include "SkRefCnt.h"

class SkFontMgr;
class SkFontConfigInterface;

/** Creates a SkFontMgr which wraps a SkFontConfigInterface. */
#ifdef SK_LEGACY_FONTMGR_FACTORY
SK_API SkFontMgr* SkFontMgr_New_FCI(sk_sp<SkFontConfigInterface> fci);
#else
SK_API sk_sp<SkFontMgr> SkFontMgr_New_FCI(sk_sp<SkFontConfigInterface> fci);
#endif

#endif // #ifndef SkFontMgr_FontConfigInterface_DEFINED
