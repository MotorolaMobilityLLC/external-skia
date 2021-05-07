/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "modules/androidkit/src/SurfaceThread.h"

#include <pthread.h>
#include <android/looper.h>
#include "include/core/SkTypes.h"


SurfaceThread::SurfaceThread() {
    pipe(fPipe);
    pthread_create(&fThread, nullptr, pthread_main, this);
}

void SurfaceThread::postMessage(const Message& message) const {
    write(fPipe[1], &message, sizeof(message));
}

void SurfaceThread::readMessage(Message* message) const {
    read(fPipe[0], message, sizeof(Message));
}

int SurfaceThread::message_callback(int /* fd */, int /* events */, void* data) {
    auto surfaceThread = (SurfaceThread*)data;
    Message message;
    surfaceThread->readMessage(&message);
    // get target surface from Message

    switch (message.fType) {
        case kSurfaceCreated: {
            SkDebugf("surface created");
            break;
        }
        case kSurfaceChanged: {
            SkDebugf("surface changed");
            break;
        }
        case kSurfaceDestroyed: {
            SkDebugf("surface destroyed");
            break;
        }
        case kAllSurfacesDestroyed: {
            // TODO: join thread here, post message from release()
            SkDebugf("surface destroyed, kill thread");
            return 0;
        }
        default: {
            // do nothing
        }
    }

    return 1;  // continue receiving callbacks
}

void* SurfaceThread::pthread_main(void* arg) {
    auto surfaceThread = (SurfaceThread*)arg;
    // Looper setup
    ALooper* looper = ALooper_prepare(ALOOPER_PREPARE_ALLOW_NON_CALLBACKS);
    ALooper_addFd(looper, surfaceThread->fPipe[0], 1, ALOOPER_EVENT_INPUT,
               surfaceThread->message_callback, surfaceThread);

    while (true) {
        const int ident = ALooper_pollAll(0, nullptr, nullptr, nullptr);

        if (ident >= 0) {
            SkDebugf("Unhandled ALooper_pollAll ident=%d !", ident);
        }
    }
    return nullptr;
}
