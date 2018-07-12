/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkFontParameters_DEFINED
#define SkFontParameters_DEFINED

#include "SkScalar.h"
#include "SkTypes.h"

struct SkFontParameters {
    struct Variation {
        // Parameters in a variation font axis.
        struct Axis {
            // Four character identifier of the font axis (weight, width, slant, italic...).
            SkFourByteTag tag;
            // Minimum value supported by this axis.
            float min;
            // Default value set by this axis.
            float def;
            // Maximum value supported by this axis. The maximum can equal the minimum.
            float max;
            // Return whether this axis is recommended to be remain hidden in user interfaces.
            bool isHidden() const { return flags & HIDDEN; }
            // Set this axis to be remain hidden in user interfaces.
            void setHidden(bool hidden) { flags = hidden ? (flags | HIDDEN) : (flags & ~HIDDEN); }
        private:
            static constexpr uint16_t HIDDEN = 0x0001;
            // Attributes for a font axis.
            uint16_t flags;
        };
    };
};

#endif
