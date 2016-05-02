/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkOncePtr_DEFINED
#define SkOncePtr_DEFINED

#include "../private/SkAtomics.h"
#include <memory>

// Use this to create a global static pointer that's intialized exactly once when you call get().
#define SK_DECLARE_STATIC_ONCE_PTR(type, name) namespace {} static SkBaseOncePtr<type> name;

template <typename T>
class SkBaseOncePtr {
public:
    template <typename F>
    T* get(const F& f) const {
        uintptr_t state = sk_atomic_load(&fState, sk_memory_order_acquire);
        if (state < 2) {
            if (state == 0) {
                // It looks like no one has tried to create our pointer yet.
                // We try to claim that task by atomically swapping our state from '0' to '1'.
                // See SkOnce.h for why we use an acquire memory order here rather than relaxed.
                if (sk_atomic_compare_exchange(
                    &fState, &state, (uintptr_t)1, sk_memory_order_acquire, sk_memory_order_acquire)) {
                    // We've claimed it.  Create our pointer and store it into fState.
                    state = (uintptr_t)f();
                    SkASSERT(state > 1);
                    sk_atomic_store(&fState, state, sk_memory_order_release);
                } else {
                    // Someone else claimed it.
                    // We fall through to the spin loop just below to wait for them to finish.
                }
            }

            while (state == 1) {
                // State '1' is our busy-but-not-done state.
                // Some other thread has claimed the job of creating our pointer.
                // We just need to wait for it to finish.
                state = sk_atomic_load(&fState, sk_memory_order_acquire);
            }

            // We shouldn't be able to get here without having created our pointer.
            SkASSERT(state > 1);
        }
        return (T*)state;
    }

    operator T*() const {
        auto state = sk_atomic_load(&fState, sk_memory_order_acquire);
        return state < 2 ? nullptr : (T*)state;
        // TODO: If state == 1 spin until it's not?
    }

    // fState == 0 --> we have not created our ptr yet
    // fState == 1 --> someone is in the middle of creating our ptr
    // else        --> (T*)fState is our ptr
    mutable uintptr_t fState;
};

#endif//SkOncePtr_DEFINED
