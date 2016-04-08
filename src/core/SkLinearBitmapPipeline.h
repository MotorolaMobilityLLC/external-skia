/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkLinearBitmapPipeline_DEFINED
#define SkLinearBitmapPipeline_DEFINED

#include "SkColor.h"
#include "SkImageInfo.h"
#include "SkMatrix.h"
#include "SkShader.h"

class SkLinearBitmapPipeline {
public:
    SkLinearBitmapPipeline(
        const SkMatrix& inverse,
        SkFilterQuality filterQuality,
        SkShader::TileMode xTile, SkShader::TileMode yTile,
        float postAlpha,
        const SkPixmap& srcPixmap);
    ~SkLinearBitmapPipeline();

    void shadeSpan4f(int x, int y, SkPM4f* dst, int count);

    template<typename Base, size_t kSize, typename Next = void>
    class Stage {
    public:
        Stage() : fIsInitialized{false} {}
        ~Stage();

        template<typename Variant, typename... Args>
        void initStage(Next* next, Args&& ... args);

        template<typename Variant, typename... Args>
        void initSink(Args&& ... args);

        template <typename To, typename From>
        To* getInterface();

        // Copy this stage to `cloneToStage` with `next` as its next stage
        // (not necessarily the same as our next, you see), returning `cloneToStage`.
        // Note: There is no cloneSinkTo method because the code usually places the top part of
        // the pipeline on a new sampler.
        Base* cloneStageTo(Next* next, Stage* cloneToStage) const;

        Base* get() const { return reinterpret_cast<Base*>(&fSpace); }
        Base* operator->() const { return this->get(); }
        Base& operator*() const { return *(this->get()); }

    private:
        std::function<void (Next*, void*)> fStageCloner;
        struct SK_STRUCT_ALIGN(16) Space {
            char space[kSize];
        };
        bool fIsInitialized;
        mutable Space fSpace;
    };

    class PointProcessorInterface;
    class SampleProcessorInterface;
    class BlendProcessorInterface;
    class DestinationInterface;

    // These values were generated by the assert above in Stage::init{Sink|Stage}.
    using MatrixStage  = Stage<PointProcessorInterface, 160, PointProcessorInterface>;
    using TileStage    = Stage<PointProcessorInterface, 160, SampleProcessorInterface>;
    using SampleStage  = Stage<SampleProcessorInterface, 100, BlendProcessorInterface>;
    using BlenderStage = Stage<BlendProcessorInterface, 80>;

private:
    PointProcessorInterface* fFirstStage;
    MatrixStage              fMatrixStage;
    TileStage                fTileStage;
    SampleStage              fSampleStage;
    BlenderStage             fBlenderStage;
    DestinationInterface*    fLastStage;
};

#endif  // SkLinearBitmapPipeline_DEFINED
