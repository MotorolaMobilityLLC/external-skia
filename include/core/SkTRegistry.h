
/*
 * Copyright 2009 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkTRegistry_DEFINED
#define SkTRegistry_DEFINED

#include "SkTypes.h"

/** Template class that registers itself (in the constructor) into a linked-list
    and provides a function-pointer. This can be used to auto-register a set of
    services, e.g. a set of image codecs.
 */
template <typename T, typename P> class SkTRegistry : SkNoncopyable {
public:
    typedef T (*Factory)(P);

    SkTRegistry(Factory fact) {
#ifdef SK_BUILD_FOR_ANDROID
        // work-around for double-initialization bug
        {
            SkTRegistry* reg = gHead;
            while (reg) {
                if (reg == this) {
                    return;
                }
                reg = reg->fChain;
            }
        }
#endif
        fFact = fact;
        fChain = gHead;
        gHead = this;
    }

    static const SkTRegistry* Head() { return gHead; }

    const SkTRegistry* next() const { return fChain; }
    Factory factory() const { return fFact; }

private:
    Factory      fFact;
    SkTRegistry* fChain;

    static SkTRegistry* gHead;
};

// The caller still needs to declare an instance of this somewhere
template <typename T, typename P> SkTRegistry<T, P>* SkTRegistry<T, P>::gHead;

#endif
