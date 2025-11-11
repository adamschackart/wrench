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
 * ~~ [ string utils ] ~~ *
--------------------------------------------------------------------------------
*/

static void util_StringUtil_isUpper(WrenVM* vm)
{
    for (const char* s = wrenGetSlotString(vm, 1); *s; s++)
    {
        if (wrench_toupper(*s) != *s)
        {
            wrenSetSlotBool(vm, 0, false);
            return;
        }
    }

    wrenSetSlotBool(vm, 0, true);
}

static void util_StringUtil_isLower(WrenVM* vm)
{
    for (const char* s = wrenGetSlotString(vm, 1); *s; s++)
    {
        if (wrench_tolower(*s) != *s)
        {
            wrenSetSlotBool(vm, 0, false);
            return;
        }
    }

    wrenSetSlotBool(vm, 0, true);
}

static void util_StringUtil_toUpper(WrenVM* vm)
{
    const char* s = wrenGetSlotString(vm, 1);
    const size_t length = wrench_strlen(s);

    char* p = (char*)wrench_malloc(length + 1);

    if (p == NULL)
    {
        wrenSetSlotString(vm, 0, "Out of memory - failed to copy string.");
        wrenAbortFiber(vm, 0);

        return;
    }

    for (size_t i = 0; i < length; i++)
    {
        p[i] = wrench_toupper(s[i]);
    }

    p[length] = '\0';

    wrenSetSlotString(vm, 0, (const char*)p);
    wrench_free(p);
}

static void util_StringUtil_toLower(WrenVM* vm)
{
    const char* s = wrenGetSlotString(vm, 1);
    const size_t length = wrench_strlen(s);

    char* p = (char*)wrench_malloc(length + 1);

    if (p == NULL)
    {
        wrenSetSlotString(vm, 0, "Out of memory - failed to copy string.");
        wrenAbortFiber(vm, 0);

        return;
    }

    for (size_t i = 0; i < length; i++)
    {
        p[i] = wrench_tolower(s[i]);
    }

    p[length] = '\0';

    wrenSetSlotString(vm, 0, (const char*)p);
    wrench_free(p);
}

static void util_StringUtil_caseCompare(WrenVM* vm)
{
    const char* a = wrenGetSlotString(vm, 1);
    const char* b = wrenGetSlotString(vm, 2);

    wrenSetSlotInt(vm, 0, wrench_strcasecmp(a, b));
}

static void util_StringUtil_compare(WrenVM* vm)
{
    const char* a = wrenGetSlotString(vm, 1);
    const char* b = wrenGetSlotString(vm, 2);

    wrenSetSlotInt(vm, 0, wrench_strcmp(a, b));
}

static void util_StringUtil_caseEquals(WrenVM* vm)
{
    const char* a = wrenGetSlotString(vm, 1);
    const char* b = wrenGetSlotString(vm, 2);

    wrenSetSlotBool(vm, 0, wrench_strcasecmp(a, b) == 0);
}

static void util_StringUtil_equals(WrenVM* vm)
{
    const char* a = wrenGetSlotString(vm, 1);
    const char* b = wrenGetSlotString(vm, 2);

    wrenSetSlotBool(vm, 0, wrench_strcmp(a, b) == 0);
}

/*
================================================================================
 * ~~ [ (un)hook ] ~~ *
--------------------------------------------------------------------------------
*/

#if WRENCH_UTIL_EXTENDED
    /*
     * Enable user extension of stdlib modules.
     */
    #ifndef __UTIL_EX_INL__
    #include <util_ex.inl>
    #endif
#else
    static bool utilWrenInitEx(WrenVM* vm)
    {
        return true;
    }

    static void utilWrenQuitEx(void)
    {
        //
    }

    static bool utilStringUtilWrenInitEx(WrenVM* vm)
    {
        return true;
    }
#endif /* WRENCH_UTIL_EXTENDED */

WRENCH_EXPORT bool utilWrenInit(WrenVM* vm)
{
    if (!wrenBeginModule(vm, "util")) { return false; } else
    {
        WREN_BEGIN_CLASS_EX(util, StringUtil, NULL, NULL);
        {
            WREN_METHOD(util, StringUtil, true, isUpper, "(string)", "(_)");
            WREN_METHOD(util, StringUtil, true, isLower, "(string)", "(_)");

            WREN_METHOD(util, StringUtil, true, toUpper, "(string)", "(_)");
            WREN_METHOD(util, StringUtil, true, toLower, "(string)", "(_)");

            WREN_METHOD(util, StringUtil, true, caseCompare, "(a, b)", "(_,_)");
            WREN_METHOD(util, StringUtil, true, compare, "(a, b)", "(_,_)");

            WREN_METHOD(util, StringUtil, true, caseEquals, "(a, b)", "(_,_)");
            WREN_METHOD(util, StringUtil, true, equals, "(a, b)", "(_,_)");

            if (!utilStringUtilWrenInitEx(vm))
            {
                return false;
            }
        }
        WREN_END_CLASS();
    }

    if (!utilWrenInitEx(vm))
    {
        return false;
    }

    return wrenEndModule(vm);
}

WRENCH_EXPORT void utilWrenQuit(void)
{
    utilWrenQuitEx();
}
