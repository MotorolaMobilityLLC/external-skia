/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrTextureAccess.h"
#include "GrColor.h"
#include "GrTexture.h"

GrTextureAccess::GrTextureAccess() {}

GrTextureAccess::GrTextureAccess(GrTexture* texture, const GrTextureParams& params) {
    this->reset(texture, params);
}

GrTextureAccess::GrTextureAccess(GrTexture* texture,
                                 GrTextureParams::FilterMode filterMode,
                                 SkShader::TileMode tileXAndY,
                                 GrShaderFlags visibility,
                                 GrSLPrecision precision) {
    this->reset(texture, filterMode, tileXAndY, visibility, precision);
}

void GrTextureAccess::reset(GrTexture* texture,
                            const GrTextureParams& params,
                            GrShaderFlags visibility,
                            GrSLPrecision precision) {
    SkASSERT(texture);
    fTexture.set(SkRef(texture), kRead_GrIOType);
    fParams = params;
    fVisibility = visibility;
    fPrecision = precision;
}

void GrTextureAccess::reset(GrTexture* texture,
                            GrTextureParams::FilterMode filterMode,
                            SkShader::TileMode tileXAndY,
                            GrShaderFlags visibility,
                            GrSLPrecision precision) {
    SkASSERT(texture);
    fTexture.set(SkRef(texture), kRead_GrIOType);
    fParams.reset(tileXAndY, filterMode);
    fVisibility = visibility;
    fPrecision = precision;
}
