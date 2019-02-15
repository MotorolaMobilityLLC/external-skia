/*
* Copyright 2019 Google LLC
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "SkParticleEffect.h"

#include "Resources.h"
#include "SkAnimTimer.h"
#include "SkCanvas.h"
#include "SkColorData.h"
#include "SkNx.h"
#include "SkPaint.h"
#include "SkParticleAffector.h"
#include "SkParticleEmitter.h"
#include "SkReflected.h"
#include "SkRSXform.h"

void SkParticleEffectParams::visitFields(SkFieldVisitor* v) {
    v->visit("MaxCount", fMaxCount);
    v->visit("Duration", fEffectDuration);
    v->visit("Rate", fRate);
    v->visit("Life", fLifetime);
    v->visit("StartColor", fStartColor);
    v->visit("EndColor", fEndColor);

    v->visit("Image", fImage);
    v->visit("ImageCols", fImageCols);
    v->visit("ImageRows", fImageRows);

    v->visit("Emitter", fEmitter);

    v->visit("Spawn", fSpawnAffectors);
    v->visit("Update", fUpdateAffectors);
}

SkParticleEffect::SkParticleEffect(sk_sp<SkParticleEffectParams> params, const SkRandom& random)
        : fParams(std::move(params))
        , fRandom(random)
        , fLooping(false)
        , fSpawnTime(-1.0)
        , fCount(0)
        , fLastTime(-1.0)
        , fSpawnRemainder(0.0f) {
    this->setCapacity(fParams->fMaxCount);

    // Load image, determine sprite rect size
    fImage = GetResourceAsImage(fParams->fImage.c_str());
    if (!fImage) {
        uint32_t whitePixel = ~0;
        SkPixmap pmap(SkImageInfo::MakeN32Premul(1, 1), &whitePixel, sizeof(uint32_t));
        fImage = SkImage::MakeRasterCopy(pmap);
    }
    int w = fImage->width();
    int h = fImage->height();
    SkASSERT(w % fParams->fImageCols == 0);
    SkASSERT(h % fParams->fImageRows == 0);
    fImageRect = SkRect::MakeIWH(w / fParams->fImageCols, h / fParams->fImageRows);
}

void SkParticleEffect::start(const SkAnimTimer& timer, bool looping) {
    fCount = 0;
    fLastTime = fSpawnTime = timer.secs();
    fSpawnRemainder = 0.0f;
    fLooping = looping;
}

void SkParticleEffect::update(const SkAnimTimer& timer) {
    if (!timer.isRunning() || !this->isAlive()) {
        return;
    }

    // Handle user edits to fMaxCount
    if (fParams->fMaxCount != fCapacity) {
        this->setCapacity(fParams->fMaxCount);
    }

    double now = timer.secs();
    float deltaTime = static_cast<float>(now - fLastTime);
    fLastTime = now;

    Sk4f startColor = Sk4f::Load(fParams->fStartColor.vec());
    Sk4f colorScale = Sk4f::Load(fParams->fEndColor.vec()) - startColor;

    SkParticleUpdateParams updateParams;
    updateParams.fDeltaTime = deltaTime;
    updateParams.fRandom = &fRandom;

    // Remove particles that have reached their end of life
    for (int i = 0; i < fCount; ++i) {
        if (now > fParticles[i].fTimeOfDeath) {
            // NOTE: This is fast, but doesn't preserve drawing order. Could be a problem...
            fParticles[i]   = fParticles[fCount - 1];
            fSpriteRects[i] = fSpriteRects[fCount - 1];
            fColors[i]      = fColors[fCount - 1];
            --i;
            --fCount;
        }
    }

    // Spawn new particles
    float desired = fParams->fRate * deltaTime + fSpawnRemainder;
    int numToSpawn = sk_float_round2int(desired);
    fSpawnRemainder = desired - numToSpawn;
    numToSpawn = SkTPin(numToSpawn, 0, fParams->fMaxCount - fCount);
    if (fParams->fEmitter) {
        // No, this isn't "stable", but spawn affectors are only run once anyway.
        // Would it ever make sense to give the same random to all particles spawned on a given
        // frame? Having a hard time thinking when that would be useful.
        updateParams.fStableRandom = &fRandom;
        // ... and this isn't "particle" t, it's effect t.
        double t = (now - fSpawnTime) / fParams->fEffectDuration;
        updateParams.fParticleT = static_cast<float>(fLooping ? fmod(t, 1.0) : SkTPin(t, 0.0, 1.0));

        for (int i = 0; i < numToSpawn; ++i) {
            fParticles[fCount].fTimeOfBirth = now;
            fParticles[fCount].fTimeOfDeath = now + fParams->fLifetime.eval(updateParams.fParticleT,
                                                                            fRandom);
            fParticles[fCount].fPV.fPose = fParams->fEmitter->emit(fRandom);
            fParticles[fCount].fPV.fVelocity.fLinear = { 0.0f, 0.0f };
            fParticles[fCount].fPV.fVelocity.fAngular = 0.0f;

            fParticles[fCount].fStableRandom = fRandom;
            fSpriteRects[fCount] = this->spriteRect(0);
            fCount++;
        }

        // Apply spawn affectors
        for (int i = fCount - numToSpawn; i < fCount; ++i) {
            for (auto affector : fParams->fSpawnAffectors) {
                if (affector) {
                    affector->apply(updateParams, fParticles[i].fPV);
                }
            }
        }
    }

    // Apply update rules
    for (int i = 0; i < fCount; ++i) {
        // Compute fraction of lifetime that's elapsed
        float t = static_cast<float>((now - fParticles[i].fTimeOfBirth) /
            (fParticles[i].fTimeOfDeath - fParticles[i].fTimeOfBirth));

        SkRandom stableRandom = fParticles[i].fStableRandom;
        updateParams.fStableRandom = &stableRandom;
        updateParams.fParticleT = t;

        // Set sprite rect by lifetime
        int frame = static_cast<int>(t * this->spriteCount() + 0.5);
        frame = SkTPin(frame, 0, this->spriteCount() - 1);
        fSpriteRects[i] = this->spriteRect(frame);

        // Set color by lifetime
        fColors[i] = Sk4f_toL32(swizzle_rb(startColor + (colorScale * t)));

        for (auto affector : fParams->fUpdateAffectors) {
            if (affector) {
                affector->apply(updateParams, fParticles[i].fPV);
            }
        }

        // Integrate position / orientation
        fParticles[i].fPV.fPose.fPosition += fParticles[i].fPV.fVelocity.fLinear * deltaTime;

        SkScalar c, s = SkScalarSinCos(fParticles[i].fPV.fVelocity.fAngular * deltaTime, &c);
        SkVector oldHeading = fParticles[i].fPV.fPose.fHeading;
        fParticles[i].fPV.fPose.fHeading = { oldHeading.fX * c - oldHeading.fY * s,
                                             oldHeading.fX * s + oldHeading.fY * c };
    }

    // Re-generate all xforms
    SkPoint ofs = this->spriteCenter();
    for (int i = 0; i < fCount; ++i) {
        fXforms[i] = fParticles[i].fPV.fPose.asRSXform(ofs);
    }

    // Mark effect as dead if we've reached the end (and are not looping)
    if (!fLooping && (now - fSpawnTime) > fParams->fEffectDuration) {
        fSpawnTime = -1.0;
    }
}

void SkParticleEffect::draw(SkCanvas* canvas) {
    if (this->isAlive()) {
        SkPaint paint;
        paint.setFilterQuality(SkFilterQuality::kMedium_SkFilterQuality);
        canvas->drawAtlas(fImage, fXforms.get(), fSpriteRects.get(), fColors.get(), fCount,
                          SkBlendMode::kModulate, nullptr, &paint);
    }
}

void SkParticleEffect::setCapacity(int capacity) {
    fParticles.realloc(capacity);
    fXforms.realloc(capacity);
    fSpriteRects.realloc(capacity);
    fColors.realloc(capacity);

    fCapacity = capacity;
    fCount = SkTMin(fCount, fCapacity);
}
