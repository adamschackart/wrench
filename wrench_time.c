/* -----------------------------------------------------------------------------
--- Copyright (c) 2012-2026 Adam Schackart / "AJ Hackman", all rights reserved.
--- Distributed under the BSD license v2 (opensource.org/licenses/BSD-3-Clause)
----------------------------------------------------------------------------- */

#ifndef WRENCH_IMPLEMENTATION
#define WRENCH_IMPLEMENTATION 1
#endif
#include <wrench.h>

/*
================================================================================
 * ~~ [ date ] ~~ *
--------------------------------------------------------------------------------
*/

static bool time_Date_giveStrings = true;

static void time_Date_giveStrings_get(WrenVM* vm)
{
    wrenSetSlotBool(vm, 0, time_Date_giveStrings);
}

static void time_Date_giveStrings_set(WrenVM* vm)
{
    time_Date_giveStrings = wrenGetSlotBool(vm, 1);
}

static bool time_Clock_localTime = true;

static void time_Clock_localTime_get(WrenVM* vm)
{
    wrenSetSlotBool(vm, 0, time_Clock_localTime);
}

static void time_Clock_localTime_set(WrenVM* vm)
{
    time_Clock_localTime = wrenGetSlotBool(vm, 1);
}

static void time_Date_year(WrenVM* vm)
{
    struct tm* time_info;
    time_t raw_time;

    wrench_time(&raw_time);

    if (time_Clock_localTime)
    {
        time_info = wrench_localtime(&raw_time);
    }
    else
    {
        time_info = wrench_gmtime(&raw_time);
    }

    wrench_assert(time_info != NULL, "");

    if (time_Date_giveStrings)
    {
        char s[1024];
        wrench_strftime(s, sizeof(s), "%Y", time_info);

        wrenSetSlotString(vm, 0, (const char*)s);
    }
    else
    {
        wrenSetSlotInt(vm, 0, time_info->tm_year + 1900);
    }
}

static void time_Date_month(WrenVM* vm)
{
    struct tm* time_info;
    time_t raw_time;

    wrench_time(&raw_time);

    if (time_Clock_localTime)
    {
        time_info = wrench_localtime(&raw_time);
    }
    else
    {
        time_info = wrench_gmtime(&raw_time);
    }

    wrench_assert(time_info != NULL, "");

    if (time_Date_giveStrings)
    {
        char s[1024];
        wrench_strftime(s, sizeof(s), "%B", time_info);

        wrenSetSlotString(vm, 0, (const char*)s);
    }
    else
    {
        wrenSetSlotInt(vm, 0, time_info->tm_mon);
    }
}

static void time_Date_day(WrenVM* vm)
{
    struct tm* time_info;
    time_t raw_time;

    wrench_time(&raw_time);

    if (time_Clock_localTime)
    {
        time_info = wrench_localtime(&raw_time);
    }
    else
    {
        time_info = wrench_gmtime(&raw_time);
    }

    wrench_assert(time_info != NULL, "");

    if (time_Date_giveStrings)
    {
        char s[1024];
        wrench_strftime(s, sizeof(s), "%d", time_info);

        wrenSetSlotString(vm, 0, (const char*)s);
    }
    else
    {
        wrenSetSlotInt(vm, 0, time_info->tm_mday);
    }
}

static void time_Date_weekday(WrenVM* vm)
{
    struct tm* time_info;
    time_t raw_time;

    wrench_time(&raw_time);

    if (time_Clock_localTime)
    {
        time_info = wrench_localtime(&raw_time);
    }
    else
    {
        time_info = wrench_gmtime(&raw_time);
    }

    wrench_assert(time_info != NULL, "");

    if (time_Date_giveStrings)
    {
        char s[1024];
        wrench_strftime(s, sizeof(s), "%A", time_info);

        wrenSetSlotString(vm, 0, (const char*)s);
    }
    else
    {
        wrenSetSlotInt(vm, 0, time_info->tm_wday);
    }
}

static void time_Date_date(WrenVM* vm)
{
    struct tm* time_info;
    time_t raw_time;

    wrench_time(&raw_time);

    if (time_Clock_localTime)
    {
        time_info = wrench_localtime(&raw_time);
    }
    else
    {
        time_info = wrench_gmtime(&raw_time);
    }

    wrench_assert(time_info != NULL, "");
    char s[1024];

    wrench_strftime(s, sizeof(s), "%Y/%m/%d", time_info);
    wrenSetSlotString(vm, 0, (const char*)s);
}

/*
================================================================================
 * ~~ [ clock ] ~~ *
--------------------------------------------------------------------------------
*/

static void time_Clock_hour(WrenVM* vm)
{
    struct tm* time_info;
    time_t raw_time;

    wrench_time(&raw_time);

    if (time_Clock_localTime)
    {
        time_info = wrench_localtime(&raw_time);
    }
    else
    {
        time_info = wrench_gmtime(&raw_time);
    }

    wrench_assert(time_info != NULL, "");
    wrenSetSlotInt(vm, 0, time_info->tm_hour);
}

static void time_Clock_minute(WrenVM* vm)
{
    struct tm* time_info;
    time_t raw_time;

    wrench_time(&raw_time);

    if (time_Clock_localTime)
    {
        time_info = wrench_localtime(&raw_time);
    }
    else
    {
        time_info = wrench_gmtime(&raw_time);
    }

    wrench_assert(time_info != NULL, "");
    wrenSetSlotInt(vm, 0, time_info->tm_min);
}

static void time_Clock_second(WrenVM* vm)
{
    struct tm* time_info;
    time_t raw_time;

    wrench_time(&raw_time);

    if (time_Clock_localTime)
    {
        time_info = wrench_localtime(&raw_time);
    }
    else
    {
        time_info = wrench_gmtime(&raw_time);
    }

    wrench_assert(time_info != NULL, "");
    wrenSetSlotInt(vm, 0, time_info->tm_sec);
}

static void time_Clock_time(WrenVM* vm)
{
    struct tm* time_info;
    time_t raw_time;

    wrench_time(&raw_time);

    if (time_Clock_localTime)
    {
        time_info = wrench_localtime(&raw_time);
    }
    else
    {
        time_info = wrench_gmtime(&raw_time);
    }

    wrench_assert(time_info != NULL, "");

    char s[1024];
    const bool as24hour = wrenGetSlotBool(vm, 1);

    if (as24hour)
    {
        wrench_strftime(s, sizeof(s), "%H:%M:%S", time_info);
    }
    else
    {
        wrench_strftime(s, sizeof(s), "%I:%M:%S %p", time_info);
    }

    wrenSetSlotString(vm, 0, (const char*)s);
}

/*
================================================================================
 * ~~ [ timer ] ~~ *
--------------------------------------------------------------------------------
*/

static void time_Timer_sleepMS(WrenVM* vm)
{
    const int milliseconds = wrenGetSlotInt(vm, 1);

    #if _WIN32
    {
        Sleep(milliseconds);
    }
    #elif _POSIX_C_SOURCE >= 199309L || __APPLE__
    {
        struct timespec ts;
        ts.tv_sec = milliseconds / 1000;
        ts.tv_nsec = (milliseconds % 1000) * 1000000;
        wrench_nanosleep(&ts, NULL);
    }
    #else
    {
        wrench_usleep(milliseconds * 1000);
    }
    #endif
}

/* TODO: Patch `System.clock` in `project.wren` to use this code instead of the C
 * `clock` function (which can be imprecise) and just call it in `Timer.seconds`.
 */
static uint64_t wrench_internal_timer_frequency;
static bool wrench_internal_monotonic_timer;

static uint64_t time_Timer_frequency(void)
{
    wrench_assert(wrench_internal_timer_frequency != UINT64_C(0), "");
    wrench_assert(wrench_internal_monotonic_timer, "");

    return wrench_internal_timer_frequency;
}

static uint64_t time_Timer_counter(void)
{
    #if _WIN32
    {
        LARGE_INTEGER i;

        if (QueryPerformanceCounter(&i) == FALSE)
        {
            // TODO
        }

        return i.QuadPart;
    }
    #else
    {
        if (wrench_internal_monotonic_timer)
        {
            #if _POSIX_TIMERS
            {
                struct timespec now;
                wrench_memset(&now, 0, sizeof(now));

                #if CLOCK_MONOTONIC_RAW
                if (wrench_clock_gettime(CLOCK_MONOTONIC_RAW, &now) == 0)
                #else
                if (wrench_clock_gettime(CLOCK_MONOTONIC, &now) == 0)
                #endif
                {
                    // TODO
                }

                return now.tv_sec * 1000000000 + now.tv_nsec;
            }
            #else
            {
                WRENCH_STUB(); return UINT64_C(0);
            }
            #endif
        }
        else
        {
            struct timeval now;
            wrench_memset(&now, 0, sizeof(now));

            if (wrench_gettimeofday(&now, NULL))
            {
                // TODO
            }

            return now.tv_sec * 1000000 + now.tv_usec;
        }
    }
    #endif
}

static void time_Timer_seconds(WrenVM* vm)
{
    const double freq = (double)time_Timer_frequency();
    const double nticks = (double)time_Timer_counter();

    wrenSetSlotDouble(vm, 0, nticks / freq);
}

/*
================================================================================
 * ~~ [ (un)hook ] ~~ *
--------------------------------------------------------------------------------
*/

#if WRENCH_TIME_EXTENDED
    /*
     * Enable user extension of stdlib modules.
     */
    #ifndef __TIME_EX_INL__
    #include <time_ex.inl>
    #endif
#else
    static bool timeWrenInitEx(WrenVM* vm)
    {
        return true;
    }

    static void timeWrenQuitEx(void)
    {
        //
    }

    static bool timeDateWrenInitEx(WrenVM* vm)
    {
        return true;
    }

    static bool timeClockWrenInitEx(WrenVM* vm)
    {
        return true;
    }

    static bool timeTimerWrenInitEx(WrenVM* vm)
    {
        return true;
    }
#endif /* WRENCH_TIME_EXTENDED */

WRENCH_EXPORT bool timeWrenInit(WrenVM* vm)
{
    if (wrench_internal_timer_frequency == UINT64_C(0))
    {
        #if _WIN32
        {
            LARGE_INTEGER i;

            if (QueryPerformanceFrequency(&i) == FALSE)
            {
                wrenSetErrorString(vm, "QueryPerformanceFrequency failed");
                return false;
            }
            else
            {
                wrench_internal_timer_frequency = i.QuadPart;
                wrench_internal_monotonic_timer = true;
            }
        }
        #else
        {
            #if _POSIX_TIMERS
            {
                struct timespec start_ts;
                wrench_memset(&start_ts, 0, sizeof(start_ts));

                #if CLOCK_MONOTONIC_RAW
                if (wrench_clock_gettime(CLOCK_MONOTONIC_RAW, &start_ts) == 0)
                #else
                if (wrench_clock_gettime(CLOCK_MONOTONIC, &start_ts) == 0)
                #endif
                {
                    // Nanoseconds.
                    wrench_internal_timer_frequency = UINT64_C(1000000000);
                    wrench_internal_monotonic_timer = true;
                }
                else
                {
                    // Microseconds.
                    wrench_internal_timer_frequency = UINT64_C(1000000);
                }
            }
            #else
            {
                // Microseconds.
                wrench_internal_timer_frequency = UINT64_C(1000000);
            }
            #endif
        }
        #endif
    }

    if (!wrenBeginModule(vm, "time")) { return false; } else
    {
        WREN_BEGIN_CLASS_EX(time, Date, NULL, NULL);
        {
            WREN_PROPERTY(time, Date, true, giveStrings);

            WREN_METHOD(time, Date, true, year, "", "");
            WREN_METHOD(time, Date, true, month, "", "");
            WREN_METHOD(time, Date, true, day, "", "");
            WREN_METHOD(time, Date, true, weekday, "", "");

            // YYYY/MM/DD
            WREN_METHOD(time, Date, true, date, "", "");

            if (!timeDateWrenInitEx(vm))
            {
                return false;
            }
        }
        WREN_END_CLASS();

        WREN_BEGIN_CLASS_EX(time, Clock, NULL, NULL);
        {
            // If false, Date and Clock give Greenwich Mean Time (aka Zulu Time).
            WREN_PROPERTY(time, Clock, true, localTime);

            WREN_METHOD(time, Clock, true, hour, "", "");
            WREN_METHOD(time, Clock, true, minute, "", "");
            WREN_METHOD(time, Clock, true, second, "", "");

            if (!wrenCode(vm,

            "static isAM { hour < 12 }\n"
            "static isPM { hour > 11 }\n"

            )) { return false; }

            /* HH:MM:SS (AM/PM)
             */
            WREN_METHOD(time, Clock, true, time, "(as24hour)", "(_)");
            WREN_CODE("static time { time(true) }");

            if (!timeClockWrenInitEx(vm))
            {
                return false;
            }
        }
        WREN_END_CLASS();

        WREN_BEGIN_CLASS_EX(time, Timer, NULL, NULL);
        {
            WREN_METHOD(time, Timer, true, sleepMS, "(milliseconds)", "(_)");
            WREN_METHOD(time, Timer, true, seconds, "", "");

            if (!timeTimerWrenInitEx(vm))
            {
                return false;
            }
        }
        WREN_END_CLASS();
    }

    if (!timeWrenInitEx(vm))
    {
        return false;
    }

    return wrenEndModule(vm);
}

WRENCH_EXPORT void timeWrenQuit(void)
{
    timeWrenQuitEx();
}
