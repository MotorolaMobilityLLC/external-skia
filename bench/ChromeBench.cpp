/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "bench/Benchmark.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkPaint.h"
#include "include/core/SkString.h"

/**
   Benchmarks that try to emulate a particular Skia call pattern observed in Chrome.
*/

/// blitRect() calls emitted by Chrome while scrolling through gmail: count, width, height.
int gmailScrollingRectSpec [431*3] = {
      1, 1254, 1160,
      1, 64, 112,
      1, 1034, 261,
      1, 1166, 1,
      1, 1166, 20,
      1, 1254, 40,
      1, 140, 20,
      1, 22, 30,
      1, 22, 39,
      1, 294, 29,
      1, 336, 25,
      1, 336, 5,
      1, 37, 3,
      1, 37, 4,
      1, 37, 5,
      1, 41, 29,
      1, 57, 15,
      1, 72, 5,
      1, 72, 8,
      1, 76, 29,
      1, 981, 88,
      1, 990, 2,
      1, 990, 6,
      2, 220, 88,
      2, 294, 1,
      2, 37, 6,
      2, 391, 55,
      2, 57, 11,
      2, 57, 14,
      2, 57, 7,
      2, 981, 30,
      2, 990, 15,
      2, 990, 19,
      3, 114, 16,
      3, 1166, 39,
      3, 1254, 154,
      3, 12, 12,
      3, 162, 7,
      3, 164, 479,
      3, 167, 449,
      3, 16, 24,
      3, 204, 497,
      3, 205, 434,
      3, 220, 1127,
      3, 220, 1132,
      3, 220, 931,
      3, 220, 933,
      3, 220, 934,
      3, 297, 8,
      3, 72, 25,
      3, 87, 30,
      3, 981, 1,
      3, 981, 126,
      3, 990, 27,
      3, 990, 36,
      3, 991, 29,
      4, 1254, 306,
      4, 1254, 36,
      4, 1, 1,
      4, 1, 14,
      4, 1, 19,
      4, 1, 7,
      4, 21, 21,
      4, 220, 30,
      4, 46, 949,
      4, 509, 30,
      4, 57, 2,
      4, 57, 6,
      4, 990, 11,
      5, 13, 8,
      5, 198, 24,
      5, 24, 24,
      5, 25, 24,
      5, 2, 24,
      5, 37, 33,
      5, 57, 4,
      5, 599, 24,
      5, 90, 24,
      5, 981, 19,
      5, 990, 23,
      5, 990, 8,
      6, 101, 29,
      6, 117, 29,
      6, 1254, 88,
      6, 139, 29,
      6, 13, 12,
      6, 15, 15,
      6, 164, 25,
      6, 16, 16,
      6, 198, 7,
      6, 1, 12,
      6, 1, 15,
      6, 1, 27,
      6, 220, 936,
      6, 24, 7,
      6, 25, 7,
      6, 2, 7,
      6, 326, 29,
      6, 336, 29,
      6, 599, 7,
      6, 86, 29,
      6, 90, 7,
      6, 96, 29,
      6, 991, 31,
      7, 198, 12,
      7, 198, 20,
      7, 198, 33,
      7, 198, 35,
      7, 24, 12,
      7, 24, 20,
      7, 24, 33,
      7, 24, 35,
      7, 25, 12,
      7, 25, 20,
      7, 25, 33,
      7, 25, 35,
      7, 2, 12,
      7, 2, 20,
      7, 2, 33,
      7, 2, 35,
      7, 304, 1,
      7, 38, 29,
      7, 51, 29,
      7, 599, 12,
      7, 599, 20,
      7, 599, 33,
      7, 599, 35,
      7, 90, 12,
      7, 90, 20,
      7, 90, 33,
      7, 90, 35,
      8, 13, 5,
      8, 198, 13,
      8, 198, 23,
      8, 220, 1,
      8, 24, 13,
      8, 24, 23,
      8, 25, 13,
      8, 25, 23,
      8, 2, 13,
      8, 2, 23,
      8, 329, 28,
      8, 57, 10,
      8, 599, 13,
      8, 599, 23,
      8, 90, 13,
      8, 90, 23,
      9, 198, 17,
      9, 198, 19,
      9, 198, 37,
      9, 198, 5,
      9, 198, 8,
      9, 24, 17,
      9, 24, 19,
      9, 24, 37,
      9, 24, 5,
      9, 24, 8,
      9, 25, 17,
      9, 25, 19,
      9, 25, 37,
      9, 25, 5,
      9, 25, 8,
      9, 2, 17,
      9, 2, 19,
      9, 2, 37,
      9, 2, 5,
      9, 2, 8,
      9, 599, 17,
      9, 599, 19,
      9, 599, 37,
      9, 599, 5,
      9, 599, 8,
      9, 72, 29,
      9, 90, 17,
      9, 90, 19,
      9, 90, 37,
      9, 90, 5,
      9, 90, 8,
     10, 13, 11,
     10, 13, 9,
     10, 198, 26,
     10, 198, 28,
     10, 1, 23,
     10, 1, 4,
     10, 1, 6,
     10, 24, 26,
     10, 24, 28,
     10, 25, 26,
     10, 25, 28,
     10, 26, 24,
     10, 2, 26,
     10, 2, 28,
     10, 599, 26,
     10, 599, 28,
     10, 90, 26,
     10, 90, 28,
     11, 198, 27,
     11, 24, 27,
     11, 25, 27,
     11, 2, 27,
     11, 599, 27,
     11, 90, 27,
     12, 198, 14,
     12, 198, 21,
     12, 198, 3,
     12, 1, 11,
     12, 1, 2,
     12, 1, 8,
     12, 24, 14,
     12, 24, 21,
     12, 24, 3,
     12, 25, 14,
     12, 25, 21,
     12, 25, 3,
     12, 26, 7,
     12, 2, 14,
     12, 2, 21,
     12, 2, 3,
     12, 329, 14,
     12, 38, 2,
     12, 599, 14,
     12, 599, 21,
     12, 599, 3,
     12, 90, 14,
     12, 90, 21,
     12, 90, 3,
     13, 198, 11,
     13, 198, 15,
     13, 198, 31,
     13, 24, 11,
     13, 24, 15,
     13, 24, 31,
     13, 25, 11,
     13, 25, 15,
     13, 25, 31,
     13, 2, 11,
     13, 2, 15,
     13, 2, 31,
     13, 57, 13,
     13, 599, 11,
     13, 599, 15,
     13, 599, 31,
     13, 71, 29,
     13, 90, 11,
     13, 90, 15,
     13, 90, 31,
     14, 13, 2,
     14, 198, 10,
     14, 24, 10,
     14, 25, 10,
     14, 26, 12,
     14, 26, 20,
     14, 26, 33,
     14, 26, 35,
     14, 2, 10,
     14, 336, 1,
     14, 45, 29,
     14, 599, 10,
     14, 63, 29,
     14, 90, 10,
     15, 13, 3,
     15, 198, 2,
     15, 198, 29,
     15, 198, 34,
     15, 24, 2,
     15, 24, 29,
     15, 24, 34,
     15, 25, 2,
     15, 25, 29,
     15, 25, 34,
     15, 2, 2,
     15, 2, 29,
     15, 2, 34,
     15, 599, 2,
     15, 599, 29,
     15, 599, 34,
     15, 90, 2,
     15, 90, 29,
     15, 90, 34,
     16, 13, 4,
     16, 13, 6,
     16, 198, 16,
     16, 198, 9,
     16, 1, 10,
     16, 24, 16,
     16, 24, 9,
     16, 25, 16,
     16, 25, 9,
     16, 26, 13,
     16, 26, 23,
     16, 2, 16,
     16, 2, 9,
     16, 599, 16,
     16, 599, 9,
     16, 90, 16,
     16, 90, 9,
     17, 13, 7,
     17, 198, 18,
     17, 24, 18,
     17, 25, 18,
     17, 2, 18,
     17, 599, 18,
     17, 90, 18,
     18, 198, 22,
     18, 198, 32,
     18, 198, 36,
     18, 198, 4,
     18, 24, 22,
     18, 24, 32,
     18, 24, 36,
     18, 24, 4,
     18, 25, 22,
     18, 25, 32,
     18, 25, 36,
     18, 25, 4,
     18, 26, 17,
     18, 26, 19,
     18, 26, 37,
     18, 26, 5,
     18, 26, 8,
     18, 2, 22,
     18, 2, 32,
     18, 2, 36,
     18, 2, 4,
     18, 599, 22,
     18, 599, 32,
     18, 599, 36,
     18, 599, 4,
     18, 90, 22,
     18, 90, 32,
     18, 90, 36,
     18, 90, 4,
     19, 13, 10,
     20, 1254, 30,
     20, 16, 1007,
     20, 26, 26,
     20, 26, 28,
     21, 198, 6,
     21, 24, 6,
     21, 25, 6,
     21, 2, 6,
     21, 599, 6,
     21, 90, 6,
     22, 198, 38,
     22, 22, 40,
     22, 24, 38,
     22, 25, 38,
     22, 26, 27,
     22, 2, 38,
     22, 599, 38,
     22, 90, 38,
     23, 1254, 1160,
     24, 220, 930,
     24, 26, 14,
     24, 26, 21,
     24, 26, 3,
     26, 11, 11,
     26, 1, 13,
     26, 26, 11,
     26, 26, 15,
     26, 26, 31,
     28, 26, 10,
     30, 176, 60,
     30, 26, 2,
     30, 26, 29,
     30, 26, 34,
     32, 26, 16,
     32, 26, 9,
     34, 26, 18,
     36, 26, 22,
     36, 26, 32,
     36, 26, 36,
     36, 26, 4,
     36, 37, 26,
     42, 26, 6,
     43, 115, 29,
     44, 198, 25,
     44, 24, 25,
     44, 25, 25,
     44, 26, 38,
     44, 2, 25,
     44, 599, 25,
     44, 90, 25,
     46, 22, 1,
     47, 198, 30,
     47, 25, 30,
     47, 2, 30,
     47, 599, 30,
     47, 90, 30,
     48, 24, 30,
     52, 176, 30,
     58, 140, 24,
     58, 4, 30,
     63, 990, 29,
     64, 1254, 1,
     88, 26, 25,
     92, 198, 39,
     92, 25, 39,
     92, 2, 39,
     92, 599, 39,
     92, 90, 39,
     93, 24, 39,
     94, 26, 30,
    108, 1254, 1051,
    117, 140, 1,
    119, 160, 1,
    126, 1, 29,
    132, 135, 16,
    147, 72, 16,
    184, 26, 39,
    238, 990, 1,
    376, 11, 1007,
    380, 11, 487,
   1389, 1034, 1007,
   1870, 57, 16,
   4034, 1, 16,
   8521, 198, 40,
   8521, 25, 40,
   8521, 2, 40,
   8521, 599, 40,
   8521, 90, 40,
   8543, 24, 40,
   8883, 13, 13,
  17042, 26, 40,
  17664, 198, 1,
  17664, 25, 1,
  17664, 2, 1,
  17664, 599, 1,
  17664, 90, 1,
  17710, 24, 1,
  35328, 26, 1,
};

/// Emulates the mix of rects blitted by gmail during scrolling
class ScrollGmailBench : public Benchmark {
    enum {
        W = 1254,
        H = 1160,
        N = 431
    };
public:
    ScrollGmailBench()  { }

protected:

    virtual const char* onGetName() { return "chrome_scrollGmail"; }
    virtual void onDraw(int loops, SkCanvas* canvas) {
        SkDEBUGCODE(this->validateBounds(canvas));
        SkPaint paint;
        this->setupPaint(&paint);
        for (int i = 0; i < N; i++) {
            SkRect current;
            setRectangle(current, i);
            for (int j = 0; j < loops * gmailScrollingRectSpec[i*3]; j++) {
                canvas->drawRect(current, paint);
            }
        }
    }
    virtual SkIPoint onGetSize() { return SkIPoint::Make(W, H); }

    void setRectangle(SkRect& current, int i) {
        current.set(0, 0,
                    SkIntToScalar(gmailScrollingRectSpec[i*3+1]), SkIntToScalar(gmailScrollingRectSpec[i*3+2]));
    }
    void validateBounds(SkCanvas* canvas) {
#ifdef SK_DEBUG
        SkIRect bounds = canvas->getDeviceClipBounds();
        SkASSERT(bounds.right()-bounds.left() >= W);
        SkASSERT(bounds.bottom()-bounds.top() >= H);
#endif
    }


private:
    typedef Benchmark INHERITED;
};

// Disabled this benchmark: it takes 15x longer than any other benchmark
// and is probably not giving us important information.
// DEF_BENCH(return new ScrollGmailBench);
