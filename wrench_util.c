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
 * ~~ [ number utilities ] ~~ *
--------------------------------------------------------------------------------
*/

static const char* wrench_internal_hex4(const int value)
{
    static const char* hex_string_digits[16] =
    {
        "0", "1", "2", "3", "4", "5", "6", "7",
        "8", "9", "A", "B", "C", "D", "E", "F",
    };

    wrench_assert(value < 16, "%i", value);
    return hex_string_digits[value];
}

static void util_NumUtil_hex4(WrenVM* vm)
{
    wrenSetSlotString(vm, 0, wrench_internal_hex4(wrenGetSlotInt(vm, 1)));
}

static const char* wrench_internal_hex8(const int value)
{
    static char s[4];

    wrench_memcpy(s + 0, wrench_internal_hex4((value >> 4) & 0xF), 1);
    wrench_memcpy(s + 1, wrench_internal_hex4((value >> 0) & 0xF), 1);

    s[2] = '\0';
    return (const char*)s;
}

static void util_NumUtil_hex8(WrenVM* vm)
{
    wrenSetSlotString(vm, 0, wrench_internal_hex8(wrenGetSlotInt(vm, 1)));
}

static const char* wrench_internal_hex16(const int value)
{
    static char s[8];

    wrench_memcpy(s + 0, wrench_internal_hex8((value >> 8) & 0xFF), 2);
    wrench_memcpy(s + 2, wrench_internal_hex8((value >> 0) & 0xFF), 2);

    s[5] = '\0';
    return (const char*)s;
}

static void util_NumUtil_hex16(WrenVM* vm)
{
    wrenSetSlotString(vm, 0, wrench_internal_hex16(wrenGetSlotInt(vm, 1)));
}

static const char* wrench_internal_hex32(const int value)
{
    static char s[16];

    wrench_memcpy(s + 0, wrench_internal_hex16((value >> 16) & 0xFFFF), 4);
    wrench_memcpy(s + 4, wrench_internal_hex16((value >>  0) & 0xFFFF), 4);

    s[9] = '\0';
    return (const char*)s;
}

static void util_NumUtil_hex32(WrenVM* vm)
{
    wrenSetSlotString(vm, 0, wrench_internal_hex32(wrenGetSlotInt(vm, 1)));
}

/*
================================================================================
 * ~~ [ string utilities ] ~~ *
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

    char* p = (char*)wrenStackMalloc(vm, length + 1);

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
    wrenStackFree(vm, p, length + 1);
}

static void util_StringUtil_toLower(WrenVM* vm)
{
    const char* s = wrenGetSlotString(vm, 1);
    const size_t length = wrench_strlen(s);

    char* p = (char*)wrenStackMalloc(vm, length + 1);

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
    wrenStackFree(vm, p, length + 1);
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

    static bool utilListUtilWrenInitEx(WrenVM* vm)
    {
        return true;
    }

    static bool utilNumUtilWrenInitEx(WrenVM* vm)
    {
        return true;
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
        WREN_BEGIN_CLASS_EX(util, ListUtil, NULL, NULL);
        {
            WREN_CODE("static reverse(list) { list[-1..0] }");

            if (!utilListUtilWrenInitEx(vm))
            {
                return false;
            }
        }
        WREN_END_CLASS();

        WREN_BEGIN_CLASS_EX(util, NumUtil, NULL, NULL);
        {
            if (1)
            {
                WREN_METHOD(util, NumUtil, true, hex4, "(num)", "(_)");
            }
            else
            {
                if (!wrenCode(vm,

                "static hex4(num) {\n"
                    "var digits = ["
                        " \"0\", \"1\", \"2\", \"3\", \"4\", \"5\", \"6\", \"7\","
                        " \"8\", \"9\", \"A\", \"B\", \"C\", \"D\", \"E\", \"F\" "
                    "]\n"

                    #if WRENCH_DEBUG
                    "if (num >= 16) {\n"
                        "Fiber.abort(\"%(num) is too large\")\n"
                    "}\n"
                    #endif

                    "return digits[num]\n"
                "}\n"

                )) { return false; }
            }

            if (0) // XXX TODO FIXME: This is faster but not threadsafe.
            {
                WREN_METHOD(util, NumUtil, true, hex8, "(num)", "(_)");
            }
            else
            {
                if (!wrenCode(vm,

                "static hex8(num) {\n"
                    "var hi = hex4((num >> 4) & 0xF)\n"
                    "var lo = hex4((num >> 0) & 0xF)\n"

                    "return hi + lo\n"
                "}\n"

                )) { return false; }
            }

            if (0) // XXX TODO FIXME: This is faster but not threadsafe.
            {
                WREN_METHOD(util, NumUtil, true, hex16, "(num)", "(_)");
            }
            else
            {
                if (!wrenCode(vm,

                "static hex16(num) {\n"
                    "var hi = hex8((num >> 8) & 0xFF)\n"
                    "var lo = hex8((num >> 0) & 0xFF)\n"

                    "return hi + lo\n"
                "}\n"

                )) { return false; }
            }

            // XXX TODO FIXME: WRENCH_NUM_(IS/TO)_INT behaves incorrectly with large values.
            if (0)
            {
                WREN_METHOD(util, NumUtil, true, hex32, "(num)", "(_)");
            }
            else
            {
                if (!wrenCode(vm,

                "static hex32(num) {\n"
                    "var hi = hex16((num >> 16) & 0xFFFF)\n"
                    "var lo = hex16((num >>  0) & 0xFFFF)\n"

                    "return hi + lo\n"
                "}\n"

                )) { return false; }
            }

            if (!wrenCode(vm,

            /* Behaves the same as Python's hex() function.
             */
            "static hex(num) {\n"
                "if (num == 0) {\n"
                    "return \"0x0\"\n"
                "} else {\n"
                    "return \"0x\" + StringUtil.toLower(hex32(num)).trimStart(\"0\")\n"
                "}\n"
            "}\n"

            )) { return false; }

            // TODO: bin4
            // TODO: bin8
            // TODO: bin16
            // TODO: bin32

            if (!wrenCode(vm,

            "static toRoman(val, s) {\n"
                "if (val == 0) {\n"
                    "if (s == \"\") {\n"
                        "return \"nulla\"\n"
                    "} else {\n"
                        "return s\n"
                    "}\n"
                "}\n"

                "if (val >= 1000) {\n"
                    "return toRoman(val - 1000, s + \"M\")\n"
                "}\n"

                "if (val >= 900) {\n"
                    "return toRoman(val - 900, s + \"CM\")\n"
                "}\n"

                "if (val >= 500) {\n"
                    "return toRoman(val - 500, s + \"D\")\n"
                "}\n"

                "if (val >= 400) {\n"
                    "return toRoman(val - 400, s + \"CD\")\n"
                "}\n"

                "if (val >= 100) {\n"
                    "return toRoman(val - 100, s + \"C\")\n"
                "}\n"

                "if (val >= 90) {\n"
                    "return toRoman(val - 90, s + \"XC\")\n"
                "}\n"

                "if (val >= 50) {\n"
                    "return toRoman(val - 50, s + \"L\")\n"
                "}\n"

                "if (val >= 40) {\n"
                    "return toRoman(val - 40, s + \"XL\")\n"
                "}\n"

                "if (val >= 10) {\n"
                    "return toRoman(val - 10, s + \"X\")\n"
                "}\n"

                "if (val == 9) {\n"
                    "return toRoman(0, s + \"IX\")\n"
                "}\n"

                "if (val == 4) {\n"
                    "return toRoman(0, s + \"IV\")\n"
                "}\n"

                "if (val >= 5) {\n"
                    "return toRoman(val - 5, s + \"V\")\n"
                "}\n"

                "if (val > 0) {\n"
                    "return toRoman(val - 1, s + \"I\")\n"
                "}\n"

                "Fiber.abort(\"unreachable\")\n"
            "}\n"

            "static toRoman(value) {\n"
                "return toRoman(value, \"\")\n"
            "}\n"

            )) { return false; }

            if (!utilNumUtilWrenInitEx(vm))
            {
                return false;
            }
        }
        WREN_END_CLASS();

        WREN_BEGIN_CLASS_EX(util, StringUtil, NULL, NULL);
        {
            if (!wrenCode(vm,

            "static codePoint(c) {\n"
                #if WRENCH_DEBUG
                "if (!(c is String)) {\n"
                    "Fiber.abort(\"Argument must be a string.\")\n"
                "}\n"

                "if (c.count != 1) {\n"
                    "Fiber.abort(\"Argument must be a 1-character string.\")\n"
                "}\n"
                #endif

                "return c.codePoints[0]\n"
            "}\n"

            )) { return false; }

            if (0) // ASCII
            {
                WREN_METHOD(util, StringUtil, true, isUpper, "(string)", "(_)");
            }
            else // UTF-8
            {
                if (!wrenCode(vm,

                "static isUpper(string) {\n"
                    "for (c in string) {\n"
                        "var d = codePoint(c)\n"
                        /*
                         * isLower
                         */
                        "if ((d >= 97 && d <= 122) || (d == 181) || (d >= 223 && d <= 246) || (d >= 248 && d <= 255)) {\n"
                            "return false\n"
                        "}\n"
                    "}\n"

                    "return true\n"
                "}\n"

                )) { return false; }
            }

            if (0) // ASCII
            {
                WREN_METHOD(util, StringUtil, true, isLower, "(string)", "(_)");
            }
            else // UTF-8
            {
                if (!wrenCode(vm,

                "static isLower(string) {\n"
                    "for (c in string) {\n"
                        "var d = codePoint(c)\n"
                        /*
                         * isUpper
                         */
                        "if ((d >= 65 && d <= 90) || (d >= 192 && d <= 214) || (d >= 216 && d <= 222)) {\n"
                            "return false\n"
                        "}\n"
                    "}\n"

                    "return true\n"
                "}\n"

                )) { return false; }
            }

            if (0) // ASCII
            {
                WREN_METHOD(util, StringUtil, true, toUpper, "(string)", "(_)");
            }
            else // UTF-8
            {
                if (!wrenCode(vm,

                "static toUpper(string) {\n"
                    "var s = []\n"

                    "for (c in string) {\n"
                        "var d = codePoint(c)\n"
                        /*
                         * isLower
                         */
                        "if ((d >= 97 && d <= 122) || (d >= 224 && d <= 246) || (d >= 248 && d <= 254)) {\n"
                            "s.add(String.fromCodePoint(d - 32))\n"
                        "} else {\n"
                            "s.add(c[0])\n"
                        "}\n"
                    "}\n"

                    "return s.join()\n"
                "}\n"

                )) { return false; }
            }

            if (0) // ASCII
            {
                WREN_METHOD(util, StringUtil, true, toLower, "(string)", "(_)");
            }
            else // UTF-8
            {
                if (!wrenCode(vm,

                "static toLower(string) {\n"
                    "var s = []\n"

                    "for (c in string) {\n"
                        "var d = codePoint(c)\n"
                        /*
                         * isUpper
                         */
                        "if ((d >= 65 && d <= 90) || (d >= 192 && d <= 214) || (d >= 216 && d <= 222)) {\n"
                            "s.add(String.fromCodePoint(d + 32))\n"
                        "} else {\n"
                            "s.add(c[0])\n"
                        "}\n"
                    "}\n"

                    "return s.join()\n"
                "}\n"

                )) { return false; }
            }

            if (0) // ASCII
            {
                WREN_METHOD(util, StringUtil, true, caseCompare, "(a, b)", "(_,_)");
            }
            else // UTF-8
            {
                if (!wrenCode(vm,

                "static caseCompare(a, b) {\n"
                    "return compare(toLower(a), toLower(b))\n"
                "}\n"

                )) { return false; }
            }

            if (0) // ASCII
            {
                WREN_METHOD(util, StringUtil, true, compare, "(a, b)", "(_,_)");
            }
            else // UTF-8
            {
                if (!wrenCode(vm,

                "static compare(s1, s2) {\n"
                    "if (s1 == s2) {\n"
                        "return 0\n"
                    "}\n"

                    "var cp1 = s1.codePoints.toList\n"
                    "var cp2 = s2.codePoints.toList\n"
                    "var len = (cp1.count <= cp2.count) ? cp1.count : cp2.count\n"

                    "for (i in 0...len) {\n"
                        "if (cp1[i] < cp2[i]) return -1\n"
                        "if (cp1[i] > cp2[i]) return 1\n"
                    "}\n"

                    "return (cp1.count < cp2.count) ? -1 : 1\n"
                "}\n"

                )) { return false; }
            }

            if (0) // ASCII
            {
                WREN_METHOD(util, StringUtil, true, caseEquals, "(a, b)", "(_,_)");
            }
            else // UTF-8
            {
                if (!wrenCode(vm,

                "static caseEquals(a, b) {\n"
                    "return caseCompare(a, b) == 0\n"
                "}\n"

                )) { return false; }
            }

            if (0) // ASCII
            {
                WREN_METHOD(util, StringUtil, true, equals, "(a, b)", "(_,_)");
            }
            else // UTF-8
            {
                if (!wrenCode(vm,

                "static equals(a, b) {\n"
                    "return compare(a, b) == 0\n"
                "}\n"

                )) { return false; }
            }

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
