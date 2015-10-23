/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkString.h"
#include "SkTime.h"

void SkTime::DateTime::toISO8601(SkString* dst) const {
    if (dst) {
        int timeZoneMinutes = SkToInt(fTimeZoneMinutes);
        char timezoneSign = timeZoneMinutes >= 0 ? '+' : '-';
        int timeZoneHours = SkTAbs(timeZoneMinutes) / 60;
        timeZoneMinutes = SkTAbs(timeZoneMinutes) % 60;
        dst->printf("%04u-%02u-%02uT%02u:%02u:%02u%c%02d:%02d",
                    static_cast<unsigned>(fYear), static_cast<unsigned>(fMonth),
                    static_cast<unsigned>(fDay), static_cast<unsigned>(fHour),
                    static_cast<unsigned>(fMinute),
                    static_cast<unsigned>(fSecond), timezoneSign, timeZoneHours,
                    timeZoneMinutes);
    }
}

// TODO: use std::chrono on Mach too, when available.

#if defined(__MACH__)
    #include "SkOncePtr.h"
    #include <mach/mach_time.h>
    SK_DECLARE_STATIC_ONCE_PTR(double, ns_per_tick);
    double SkTime::GetNSecs() {
        uint64_t ticks = mach_absolute_time();
        return ticks * *ns_per_tick.get([]{
            mach_timebase_info_data_t timebase;
            (void)mach_timebase_info(&timebase);
            return new double(timebase.numer * 1.0 / timebase.denom);
        });
    }
#else
    #include <chrono>
    double SkTime::GetNSecs() {
        auto now = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::nano> ns = now.time_since_epoch();
        return ns.count();
    }
#endif
