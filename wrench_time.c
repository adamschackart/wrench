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
