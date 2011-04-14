/*
    Copyright 2010 Google Inc.

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

         http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
 */


#ifndef GrNoncopyable_DEFINED
#define GrNoncopyable_DEFINED

#include "GrTypes.h"

/**
 *  Base for classes that want to disallow copying themselves. It makes its
 *  copy-constructor and assignment operators private (and unimplemented).
 */
class GR_API GrNoncopyable {
public:
    GrNoncopyable() {}

private:
    // illegal
    GrNoncopyable(const GrNoncopyable&);
    GrNoncopyable& operator=(const GrNoncopyable&);
};

#endif

