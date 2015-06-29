/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkSharedMutex.h"

#include "SkAtomics.h"
#include "SkSemaphore.h"
#include "SkTypes.h"

// The fQueueCounts fields holds many counts in an int32_t in order to make managing them atomic.
// These three counts must be the same size, so each gets 10 bits. The 10 bits represent
// the log of the count which is 1024.
//
// The three counts held in fQueueCounts are:
// * Shared - the number of shared lock holders currently running.
// * WaitingExclusive - the number of threads waiting for an exclusive lock.
// * WaitingShared - the number of threads waiting to run while waiting for an exclusive thread
//   to finish.
static const int kLogThreadCount = 10;

enum {
    kSharedOffset          = (0 * kLogThreadCount),
    kWaitingExlusiveOffset = (1 * kLogThreadCount),
    kWaitingSharedOffset   = (2 * kLogThreadCount),
    kSharedMask            = ((1 << kLogThreadCount) - 1) << kSharedOffset,
    kWaitingExclusiveMask  = ((1 << kLogThreadCount) - 1) << kWaitingExlusiveOffset,
    kWaitingSharedMask     = ((1 << kLogThreadCount) - 1) << kWaitingSharedOffset,
};

SkSharedMutex::SkSharedMutex() : fQueueCounts(0) { }

void SkSharedMutex::acquire() {
    // Increment the count of exclusive queue waiters.
    int32_t oldQueueCounts = fQueueCounts.fetch_add(1 << kWaitingExlusiveOffset,
                                                    sk_memory_order_acquire);

    // If there are no other exclusive waiters and no shared threads are running then run
    // else wait.
    if ((oldQueueCounts & kWaitingExclusiveMask) > 0 || (oldQueueCounts & kSharedMask) > 0) {
        fExclusiveQueue.wait();
    }
}

void SkSharedMutex::release() {
    int32_t oldQueueCounts = fQueueCounts.load(sk_memory_order_relaxed);
    int32_t waitingShared;
    int32_t newQueueCounts;
    do {
        newQueueCounts = oldQueueCounts;

        // Decrement exclusive waiters.
        newQueueCounts -= 1 << kWaitingExlusiveOffset;

        // The number of threads waiting to acquire a shared lock.
        waitingShared = (oldQueueCounts & kWaitingSharedMask) >> kWaitingSharedOffset;

        // If there are any move the counts of all the shared waiters to actual shared. They are
        // going to run next.
        if (waitingShared > 0) {

            // Set waiting shared to zero.
            newQueueCounts &= ~kWaitingSharedMask;

            // Because this is the exclusive release, then there are zero readers. So, the bits
            // for shared locks should be zero. Since those bits are zero, we can just |= in the
            // waitingShared count instead of clearing with an &= and then |= the count.
            newQueueCounts |= waitingShared << kSharedOffset;
        }

    } while (!fQueueCounts.compare_exchange(&oldQueueCounts, newQueueCounts,
                                            sk_memory_order_release, sk_memory_order_relaxed));

    if (waitingShared > 0) {
        // Run all the shared.
        fSharedQueue.signal(waitingShared);
    } else if ((newQueueCounts & kWaitingExclusiveMask) > 0) {
        // Run a single exclusive waiter.
        fExclusiveQueue.signal();
    }
}

void SkSharedMutex::acquireShared() {
    int32_t oldQueueCounts = fQueueCounts.load(sk_memory_order_relaxed);
    int32_t newQueueCounts;
    do {
        newQueueCounts = oldQueueCounts;
        // If there are waiting exclusives then this shared lock waits else it runs.
        if ((newQueueCounts & kWaitingExclusiveMask) > 0) {
            newQueueCounts += 1 << kWaitingSharedOffset;
        } else {
            newQueueCounts += 1 << kSharedOffset;
        }
    } while (!fQueueCounts.compare_exchange(&oldQueueCounts, newQueueCounts,
                                            sk_memory_order_acquire, sk_memory_order_relaxed));

    // If there are waiting exclusives, then this shared waits until after it runs.
    if ((newQueueCounts & kWaitingExclusiveMask) > 0) {
        fSharedQueue.wait();
    }
}

void SkSharedMutex::releaseShared() {
    // Decrement the shared count.
    int32_t oldQueueCounts = fQueueCounts.fetch_add(-1 << kSharedOffset,
                                                    sk_memory_order_release);

    // If shared count is going to zero (because the old count == 1) and there are exclusive
    // waiters, then run a single exclusive waiter.
    if (((oldQueueCounts & kSharedMask) >> kSharedOffset) == 1
        && (oldQueueCounts & kWaitingExclusiveMask) > 0) {
        fExclusiveQueue.signal();
    }
}
