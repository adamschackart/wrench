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

static void util_NumUtil_atan2(WrenVM* vm)
{
    wrenSetSlotDouble(vm, 0, wrench_atan2(wrenGetSlotDouble(vm, 1), wrenGetSlotDouble(vm, 2)));
}

static void util_NumUtil_signedPow(WrenVM* vm)
{
    const double val = wrenGetSlotDouble(vm, 1);
    const double exp = wrenGetSlotDouble(vm, 2);

    double val_abs = wrench_fabs(val);

#if 1
    if (val_abs < 0.0000001)
#else
    if (val == 0)
#endif
    {
        wrenSetSlotDouble(vm, 0, 0.0);
    }
    else
    {
        double sign;

        if (val > 0.0)
        {
            sign = 1.0;
        }
        else if (val < 0.0)
        {
            sign = -1.0;
        }
        else
        {
            sign = 0.0;
        }

        wrenSetSlotDouble(vm, 0, sign * wrench_pow(val_abs, exp));
    }
}

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

static double wrench_internal_exponent(double value, int power)
{
    if (power == 0)
    {
        return 1.0;
    }

    if (power < 0)
    {
        value = 1.0 / value;
        power = -power;
    }

    double result = 1.0;
    double current_product = value;

    // Exponentiation by squaring.
    while (power > 0)
    {
        if (power % 2 == 1)
        {
            result *= current_product;
        }

        current_product *= current_product;
        power /= 2;
    }

    return result;
}

static void util_NumUtil_exponent(WrenVM* vm)
{
    const double value = wrenGetSlotDouble(vm, 1);
    const int power = wrenGetSlotInt(vm, 2);

    wrenSetSlotDouble(vm, 0, wrench_internal_exponent(value, power));
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
        p[i] = (char)wrench_toupper(s[i]);
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
        p[i] = (char)wrench_tolower(s[i]);
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

static void util_StringUtil_generateVisualStudioGUID(WrenVM* vm)
{
    const char* input = wrenGetSlotString(vm, 1);
    char output[64];

    // 32-bit FNV-1a string hash.
    uint32_t hash = 2166136261u;

    for (int i = 0; input[i] != '\0'; i++)
    {
        hash ^= (uint8_t)input[i];
        hash *= 16777619u;
    }

    // Derive components by multiplying by primes. Using uint32_t automatically wraps overflows,
    // acting exactly like the (& 0xFFFFFFFF) bitwise clamp in the default Wren implementation.
    const uint32_t part1 = hash;
    const uint32_t part2 = hash * 31u;
    const uint32_t part3 = hash * 73u;
    const uint32_t part4 = hash * 109u;

    // Format as XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX. We emulate the Wren string slicing
    // (e.g., hex2[0...4] and hex2[4...8]) by shifting the 32-bit integers down by 16 bits
    // for the first half, and masking the lower 16 bits for the second half.
    wrench_snprintf(output,
                    sizeof(output),
                    "%08x-%04x-%04x-%04x-%04x%08x",
                    part1,
                    (part2 >> 16) & 0xFFFF,
                    part2 & 0xFFFF,
                    (part3 >> 16) & 0xFFFF,
                    part3 & 0xFFFF,
                    part4);

    wrenSetSlotString(vm, 0, (const char*)output);
}

static void util_StringUtil_escapeAndQuote(WrenVM* vm)
{
    const char* input = wrenGetSlotString(vm, 1);

    /* Max expansion is 6 bytes per character (\uXXXX) + 2 quotes + 1 NUL.
     */
    const size_t input_length = wrench_strlen(input);
    const size_t output_size = input_length * 6 + 3;

    char* output = (char*)wrenStackMalloc(vm, output_size);
    char* s = output;

    if (output == NULL)
    {
        wrenSetSlotString(vm, 0, "Out of memory! Wrench temp string alloc failed.");
        wrenAbortFiber(vm, 0);

        return;
    }

    wrench_assert(s + 1 <= output + (output_size - 1), "");
    *s++ = '"';

    while (*input)
    {
        // Cast prevents sign extension + UTF-8 capture.
        const unsigned char c = (unsigned char)*input++;

        char escape = 0;
        switch (c)
        {
            case '\"': escape = '"';  break;
            case '\\': escape = '\\'; break;
            case '\b': escape = 'b';  break;
            case '\f': escape = 'f';  break;
            case '\n': escape = 'n';  break;
            case '\r': escape = 'r';  break;
            case '\t': escape = 't';  break;
        }

        if (escape)
        {
            wrench_assert(s + 1 <= output + (output_size - 1), "");
            *s++ = '\\';

            wrench_assert(s + 1 <= output + (output_size - 1), "");
            *s++ = escape;
        }
        else if (c <= 0x1F)
        {
            char buffer[8];
            wrench_snprintf(buffer, sizeof(buffer), "\\u%04X", c);

            wrench_assert(s + 6 <= output + (output_size - 1), "");
            wrench_memcpy(s, buffer, 6);

            s += 6;
        }
        else if (c >= 0x80)
        {
            // Determine the expected unicode sequence length.
            int bytes;

            if ((c & 0xE0) == 0xC0)
            {
                bytes = 1;
            }
            else if ((c & 0xF0) == 0xE0)
            {
                bytes = 2;
            }
            else if ((c & 0xF8) == 0xF0)
            {
                bytes = 3;
            }
            else
            {
                // Drop invalid leading bytes (e.g. orphan continuation bytes).
                continue;
            }

            // Validate upcoming continuation bytes without consuming them yet.
            int valid = 1;
            const char* temp = input;

            for (int i = 0; i < bytes; i++)
            {
                if (!*temp || (*temp & 0xC0) != 0x80)
                {
                    valid = 0;
                    break;
                }

                temp++;
            }

            if (valid)
            {
                // Pass the leading byte + continuation bytes through natively.
                *s++ = (char)c;

                for (int i = 0; i < bytes; i++)
                {
                    *s++ = *input++;
                }
            }
            else
            {
                // Drop the malformed sequence.
                continue;
            }
        }
        else
        {
            wrench_assert(s + 1 <= output + (output_size - 1), "");
            *s++ = (char)c;
        }
    }

    wrench_assert(s + 1 <= output + (output_size - 1), "");
    *s++ = '"';

    wrench_assert(s + 1 <= output + output_size, "");
    *s++ = '\0';

    wrenSetSlotString(vm, 0, output);
    wrenStackFree(vm, output, output_size);
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
        /* TODO: Should probably be `SequenceUtil` instead.
         */
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
                WREN_METHOD(util, NumUtil, true, atan2, "(y, x)", "(_,_)");
            }
            else
            {
                WREN_CODE("static atan2(y, x) { y.atan(x) }");
            }

            if (1)
            {
                WREN_METHOD(util, NumUtil, true, signedPow, "(val, exp)", "(_,_)");
            }
            else
            {
                if (!wrenCode(vm,

                "static signedPow(val, exp) {\n"
                    "if (val == 0) {\n"
                        "return 0\n"
                    "} else {\n"
                        "return val.sign * val.abs.pow(exp)\n"
                    "}\n"
                "}\n"

                )) { return false; }
            }

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

            /* TODO: Implement this method in C.
             */
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

            if (1)
            {
                WREN_METHOD(util, NumUtil, true, exponent, "(value, power)", "(_,_)");
            }
            else
            {
                if (!wrenCode(vm,

                "static exponent(value, power) {\n"
                    "if (power == 0) {\n"
                        "return 1\n"
                    "}\n"

                    "var result = value\n"

                    "for (i in 1...power) {\n"
                        "result = result * value\n"
                    "}\n"

                    "return result\n"
                "}\n"

                )) { return false; }
            }

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
            WREN_CODE("static reverse(str) { str[-1..0] }");

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

            /* TODO: C versions of these methods.
             */
            if (!wrenCode(vm,

            "static leftPad(s, count, with) {\n"
                "if (s.count >= count) {\n"
                    "return s\n"
                "}\n"

                "var paddingNeeded = count - s.count\n"

                "var pad = with * (paddingNeeded / with.count).ceil\n"
                "pad = pad[0...paddingNeeded]\n"

                "return pad + s\n"
            "}\n"

            "static rightPad(s, count, with) {\n"
                "if (s.count >= count) {\n"
                    "return s\n"
                "}\n"

                "var paddingNeeded = count - s.count\n"

                "var pad = with * paddingNeeded\n"
                "pad = pad[0...paddingNeeded]\n"

                "return s + pad\n"
            "}\n"

            )) { return false; }

            /* Visual Studio strictly expects GUIDs in the 8-4-4-4-12 hex format. This enables us to
             * generate a deterministic psuedo-hash to ensure the solution doesn't constantly reload.
             */
            if (1)
            {
                WREN_METHOD(util, StringUtil, true, generateVisualStudioGUID, "(str)", "(_)");
            }
            else
            {
                if (!wrenCode(vm,

                "static generateVisualStudioGUID(str) {\n"
                    "var hash = 2166136261\n"

                    "for (i in 0...str.byteCount_) {\n"
                        "hash = (hash ^ str.bytes[i]) * 16777619\n"
                    "}\n"

                    /* Helper to guarantee 8-character strings by padding leading zeros.
                     * TODO: Should add this to StringUtil & make padding configurable.
                     */
                    "var pad8 = Fn.new { |num|\n"
                        "var s = \"00000000\" + num.toString\n"
                        "return s[-8..-1]\n"
                    "}\n"

                    "var hex1 = pad8.call(NumUtil.hex32(hash & 0xFFFFFFFF))\n"
                    "var hex2 = pad8.call(NumUtil.hex32((hash * 31) & 0xFFFFFFFF))\n"
                    "var hex3 = pad8.call(NumUtil.hex32((hash * 73) & 0xFFFFFFFF))\n"
                    "var hex4 = pad8.call(NumUtil.hex32((hash * 109) & 0xFFFFFFFF))\n"

                    // Maps to XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX.
                    "return hex1 + \"-\" + hex2[0...4] + \"-\" + hex2[4...8] + \"-\" + hex3[0...4] + \"-\" + hex3[4...8] + hex4\n"
                "}\n"

                )) { return false; }
            }

            /* For JSON serialization.
             */
            if (1)
            {
                WREN_METHOD(util, StringUtil, true, escapeAndQuote, "(obj)", "(_)");
            }
            else
            {
                if (!wrenCode(vm,

                "static escapeAndQuote(obj) {\n"
                    "var substrings = []\n"

                    // Escape special characters.
                    "for (char in obj) {\n"
                        "if (char == \"\\\"\") {\n"
                            "substrings.add(\"\\\\\\\"\")\n"
                        "} else if (char == \"\\\\\") {\n"
                            "substrings.add(\"\\\\\\\\\")\n"
                        "} else if (char == \"\\b\") {\n"
                            "substrings.add(\"\\\\b\")\n"
                        "} else if (char == \"\\f\") {\n"
                            "substrings.add(\"\\\\f\")\n"
                        "} else if (char == \"\\n\") {\n"
                            "substrings.add(\"\\\\n\")\n"
                        "} else if (char == \"\\r\") {\n"
                            "substrings.add(\"\\\\r\")\n"
                        "} else if (char == \"\\t\") {\n"
                            "substrings.add(\"\\\\t\")\n"
                        /*
                        "} else if (char.codePoints[0] <= 0x1f) {\n"
                            // Control characters.
                            "var pt = char.codePoints[0]\n"
                            "var hex = StringUtil.leftPad(StringUtil.toHex(pt), 4, \"0\")\n"

                            "substrings.add(\"\\\\u\" + hex)\n"
                        */
                        "} else if (char.bytes[0] <= 0x1f) {\n"
                            // Control characters.
                            "var byte = char.bytes[0]\n"
                            "var hex = StringUtil.leftPad(StringUtil.toHex(byte), 4, \"0\")\n"

                            "substrings.add(\"\\\\u\" + hex)\n"
                        "} else {\n"
                            "substrings.add(char)\n"
                        "}\n"
                    "}\n"

                    "return \"\\\"\" + substrings.join(\"\") + \"\\\"\"\n"
                "}\n"

                )) { return false; }
            }

            /* TODO: This is a duplicate of NumUtil.hex - should remove it after testing.
             */
            if (0)
            {
                // TODO: C version.
            }
            else
            {
                if (!wrenCode(vm,

                "static toHex(byte) {\n"
                    "if (byte == 0) {\n"
                        "return \"0\"\n"
                    "}\n"

                    "var hex_chars = [\"0\", \"1\", \"2\", \"3\", \"4\", \"5\", \"6\", \"7\", \"8\", \"9\", \"A\", \"B\", \"C\", \"D\", \"E\", \"F\"]\n"
                    "var hex = \"\"\n"

                    "while (byte > 0) {\n"
                        "var c = byte % 16\n"
                        "hex = hex_chars[c] + hex\n"
                        "byte = byte >> 4\n"
                    "}\n"

                    "return hex\n"
                "}\n"

                )) { return false; }
            }

            // TODO: Fuzzy string matching.

            if (!utilStringUtilWrenInitEx(vm))
            {
                return false;
            }
        }
        WREN_END_CLASS();

        WREN_BEGIN_CLASS_EX(util, ObjectUtil, NULL, NULL);
        {
            if (!wrenCode(vm,

            /* Attempt to reduce an object down to basic Wren types, for JSON serialization etc.
             */
            "static toPrimitive(object) {\n"
                "if (object is Num || object is Bool || object is Null || object is String) {\n"
                    "return object\n"
                "} else if (object is List) {\n"
                    "return object.map { |element| toPrimitive(element) }.toList\n"
                "} else if (object is Map) {\n"
                    "var r = {}\n"

                    "object.each { |pair| r[toPrimitive(pair.key)] = toPrimitive(pair.value) }\n"
                    "return r\n"
                "} else {\n"
                    "return object.toPrimitive\n"
                "}\n"
            "}\n"

            )) { return false; }
        }
        WREN_END_CLASS();

        if (!wrenCode(vm,

        "class Set is Sequence {\n"
            "construct new() {\n"
                "_map = {}\n"
            "}\n"

            "iterate(iterator) {\n"
                "return _map.keys.iterate(iterator)\n"
            "}\n"

            "iteratorValue(iterator) {\n"
                "return _map.keys.iteratorValue(iterator)\n"
            "}\n"

            "clear() {\n"
                "_map.clear()\n"
            "}\n"

            "contains(value) {\n"
                "return _map[value] != null\n"
            "}\n"

            "has(value) {\n"
                "return _map[value] != null\n"
            "}\n"

            "count {\n"
                "return _map.count\n"
            "}\n"

            "toList {\n"
                "return _map.keys.toList\n"
            "}\n"

            "add(value) {\n"
                "_map[value] = true\n"
                "return value\n"
            "}\n"

            "remove(value) {\n"
                "return _map.remove(value) != null ? value : null\n"
            "}\n"

            "discard(value) {\n"
                "return _map.remove(value) != null ? value : null\n"
            "}\n"

            "pop() {\n"
                "if (_map.count > 0) {\n"
                    "var iterator = _map.keys.iterate(null)\n"
                    "return _map.remove(_map.keys.iteratorValue(iterator))\n"
                "} else {\n"
                    "return null\n"
                "}\n"
            "}\n"

            // TODO: isDisjoint(other)
            // TODO: isSubSet(other)
            // TODO: isSuperSet(other)
            // TODO: union(other)
            // TODO: intersection(other)
            // TODO: difference(other)
            // TODO: symmetricDifference(other)
            // TODO: copy
            // TODO: update(others)
            // TODO: intersectionUpdate(others)
            // TODO: differenceUpdate(others)
            // TODO: symmetricDifferenceUpdate(others)
        "}\n"

        )) { return false; }

        if (!wrenCode(vm,

        "class Generator {\n"
            "construct new(fn) {\n"
                "if (fn is Fiber) {\n"
                    "_fiber = fn\n"
                "} else if (fn is Fn) {\n"
                    "_fiber = Fiber.new(fn)\n"
                "} else {\n"
                    "if (true) {\n"
                        /*
                         * Capture in upvalue and try to call.
                         */
                        "_fiber = Fiber.new { fn.call() }\n"
                    "} else {\n"
                        "Fiber.abort(\"%(fn)\")\n"
                    "}\n"
                "}\n"

                "_data = null\n"
            "}\n"

            "static newData(data, fn) {\n"
                "var generator = new(fn)\n"

                "generator.data = data\n"
                "return generator\n"
            "}\n"

            "data { _data }\n"
            "data=(value) { _data = value }\n"

            "iterate(iterator) {\n"
                "if (_fiber.isDone) {\n"
                    "return false\n"
                "}\n"

                "_current = _fiber.call(data)\n"
                "return !_fiber.isDone\n"
            "}\n"

            "iteratorValue(iterator) { _current }\n"
        "}\n"

        )) { return false; }
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
