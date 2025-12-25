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
 * ~~ [ platform ] ~~ *
--------------------------------------------------------------------------------
*/

static void platform_Platform_is32bit(WrenVM* vm)
{
    wrenSetSlotBool(vm, 0, sizeof(void*) == 4);
}

static void platform_Platform_is64bit(WrenVM* vm)
{
    wrenSetSlotBool(vm, 0, sizeof(void*) == 8);
}

/*
================================================================================
 * ~~ [ (un)hook ] ~~ *
--------------------------------------------------------------------------------
*/

#if WRENCH_PLATFORM_EXTENDED
    /*
     * Enable user extension of stdlib modules.
     */
    #ifndef __PLATFORM_EX_INL__
    #include <platform_ex.inl>
    #endif
#else
    static bool platformWrenInitEx(WrenVM* vm)
    {
        return true;
    }

    static void platformWrenQuitEx(void)
    {
        //
    }

    static bool platformPlatformWrenInitEx(WrenVM* vm)
    {
        return true;
    }
#endif /* WRENCH_PLATFORM_EXTENDED */

WRENCH_EXPORT bool platformWrenInit(WrenVM* vm)
{
    if (!wrenBeginModule(vm, "platform")) { return false; } else
    {
        WREN_BEGIN_CLASS_EX(platform, Platform, NULL, NULL);
        {
            WREN_METHOD(platform, Platform, true, is32bit, "", "");
            WREN_METHOD(platform, Platform, true, is64bit, "", "");

            // ===== [ OS ] ====================================================

            #if defined(__ANDROID__)
            {
                WREN_CODE("static isAndroid { true }");
            }
            #else
            {
                WREN_CODE("static isAndroid { false }");
            }
            #endif

            #if defined(__BEOS__)
            {
                WREN_CODE("static isBeOS { true }");
            }
            #else
            {
                WREN_CODE("static isBeOS { false }");
            }
            #endif

            #if defined(__FreeBSD__) || \
                defined(__NetBSD__) ||  \
                defined(__OpenBSD__) || \
                defined(__bsdi__) ||    \
                defined(__DragonFly__) || \
                defined(_SYSTYPE_BSD)
            {
                WREN_CODE("static isBSD { true }");
            }
            #else
            {
                WREN_CODE("static isBSD { false }");
            }
            #endif

            #if defined(__CYGWIN__)
            {
                WREN_CODE("static isCygwin { true }");
            }
            #else
            {
                WREN_CODE("static isCygwin { false }");
            }
            #endif

            #if defined(__linux__) || \
                defined(linux) || \
                defined(__linux)
            {
                WREN_CODE("static isLinux { true }");
            }
            #else
            {
                WREN_CODE("static isLinux { false }");
            }
            #endif

            #if defined(macintosh) || \
                defined(Macintosh)
            {
                WREN_CODE("static isMacOS9 { true }");
            }
            #else
            {
                WREN_CODE("static isMacOS9 { false }");
            }
            #endif

            #if defined(__APPLE__) && defined(__MACH__)
            {
                WREN_CODE("static isMacOSX { true }");
            }
            #else
            {
                WREN_CODE("static isMacOSX { false }");
            }
            #endif

            #if defined(MSDOS) || \
                defined(__MSDOS__) || \
                defined(_MSDOS) || \
                defined(__DOS__)
            {
                WREN_CODE("static isMSDOS { true }");
            }
            #else
            {
                WREN_CODE("static isMSDOS { false }");
            }
            #endif

            #if defined(OS2) || \
                defined(_OS2) || \
                defined(__OS2__) || \
                defined(__TOS_OS2__)
            {
                WREN_CODE("static isOS2 { true }");
            }
            #else
            {
                WREN_CODE("static isOS2 { false }");
            }
            #endif

            #if defined(unix) || \
                defined(__unix__) || \
                defined(__unix)
            {
                WREN_CODE("static isUnix { true }");
            }
            #else
            {
                WREN_CODE("static isUnix { false }");
            }
            #endif

            #if defined(_WIN16) || \
                defined(_WIN32) || \
                defined(_WIN64) || \
                defined(__WIN32__) || \
                defined(__TOS_WIN__) || \
                defined(__WINDOWS__)
            {
                WREN_CODE("static isWindows { true }");
            }
            #else
            {
                WREN_CODE("static isWindows { false }");
            }
            #endif

            #if defined(_WIN32_WCE)
            {
                WREN_CODE("static isWindowsCE { true }");
            }
            #else
            {
                WREN_CODE("static isWindowsCE { false }");
            }
            #endif

            // ===== [ CPU ] ===================================================

            #if defined(__alpha__) || \
                defined(__alpha) || \
                defined(_M_ALPHA)
            {
                WREN_CODE("static isAlpha { true }");
            }
            #else
            {
                WREN_CODE("static isAlpha { false }");
            }
            #endif

            #if defined(__amd64__) || \
                defined(__amd64) || \
                defined(__x86_64__) || \
                defined(__x86_64) || \
                defined(_M_X64) || \
                defined(_M_AMD64)
            {
                WREN_CODE("static isX86_64 { true }");
            }
            #else
            {
                WREN_CODE("static isX86_64 { false }");
            }
            #endif

            #if defined(__arm__) || \
                defined(__TARGET_ARCH_ARM) || \
                defined(_ARM) || \
                defined(_M_ARM) || \
                defined(__arm)
            {
                WREN_CODE("static isARM32 { true }");
            }
            #else
            {
                WREN_CODE("static isARM32 { false }");
            }
            #endif

            #if defined(__thumb__) || \
                defined(__TARGET_ARCH_THUMB) || \
                defined(_M_ARMT)
            {
                WREN_CODE("static isARM_Thumb { true }");
            }
            #else
            {
                WREN_CODE("static isARM_Thumb { false }");
            }
            #endif

            #if defined(__aarch64__)
            {
                WREN_CODE("static isARM64 { true }");
            }
            #else
            {
                WREN_CODE("static isARM64 { false }");
            }
            #endif

            #if defined(__ARM_NEON__) || \
                defined(__ARM_NEON)
            {
                WREN_CODE("static isARM_NEON { true }");
            }
            #else
            {
                WREN_CODE("static isARM_NEON { false }");
            }
            #endif

            #if defined(__bfin) || \
                defined(__BFIN__)
            {
                WREN_CODE("static isBlackfin { true }");
            }
            #else
            {
                WREN_CODE("static isBlackfin { false }");
            }
            #endif

            #if defined(__convex__)
            {
                WREN_CODE("static isConvex { true }");
            }
            #else
            {
                WREN_CODE("static isConvex { false }");
            }
            #endif

            #if defined(__epiphany__)
            {
                WREN_CODE("static isEpiphany { true }");
            }
            #else
            {
                WREN_CODE("static isEpiphany { false }");
            }
            #endif

            #if defined(__hppa__) || \
                defined(__HPPA__) || \
                defined(__hppa)
            {
                WREN_CODE("static isHPPA { true }");
            }
            #else
            {
                WREN_CODE("static isHPPA { false }");
            }
            #endif

            #if defined(i386) || \
                defined(__i386) || \
                defined(__i386__) || \
                defined(__IA32__) || \
                defined(_M_I86) || \
                defined(_M_IX86) || \
                defined(__X86__) || \
                defined(_X86_) || \
                defined(__THW_INTEL__) || \
                defined(__I86__) || \
                defined(__INTEL__) || \
                defined(__386)
            {
                WREN_CODE("static isX86_32 { true }");
            }
            #else
            {
                WREN_CODE("static isX86_32 { false }");
            }
            #endif

            #if defined(__ia64__) || \
                defined(_IA64) || \
                defined(__IA64__) || \
                defined(__ia64) || \
                defined(_M_IA64) || \
                defined(__itanium__)
            {
                WREN_CODE("static isItanium { true }");
            }
            #else
            {
                WREN_CODE("static isItanium { false }");
            }
            #endif

            #if defined(__m68k__) || \
                defined(M68000) || \
                defined(__MC68K__)
            {
                WREN_CODE("static is68k { true }");
            }
            #else
            {
                WREN_CODE("static is68k { false }");
            }
            #endif

            #if defined(__mips__) || \
                defined(mips) || \
                defined(__mips) || \
                defined(__MIPS__)
            {
                WREN_CODE("static isMIPS { true }");
            }
            #else
            {
                WREN_CODE("static isMIPS { false }");
            }
            #endif

            #if defined(__powerpc) || \
                defined(__powerpc__) || \
                defined(__powerpc64__) || \
                defined(__POWERPC__) || \
                defined(__ppc__) || \
                defined(__ppc64__) || \
                defined(__PPC__) || \
                defined(__PPC64__) || \
                defined(_ARCH_PPC) || \
                defined(_ARCH_PPC64) || \
                defined(_M_PPC) || \
                defined(__PPCGECKO__) || \
                defined(__PPCBROADWAY__) || \
                defined(_XENON) || \
                defined(__ppc)
            {
                WREN_CODE("static isPowerPC { true }");
            }
            #else
            {
                WREN_CODE("static isPowerPC { false }");
            }
            #endif

            #if defined(pyr)
            {
                WREN_CODE("static isPyramid { true }");
            }
            #else
            {
                WREN_CODE("static isPyramid { false }");
            }
            #endif

            #if defined(__riscv)
            {
                WREN_CODE("static isRiscV { true }");
            }
            #else
            {
                WREN_CODE("static isRiscV { false }");
            }
            #endif

            // TODO: RiscV32 (__riscv_xlen == 32)

            // TODO: RiscV64 (__riscv_xlen == 64)

            // TODO: RiscV_Compressed (__riscv_compressed)

            // TODO: RiscV_Vector (__riscv_vector)

            #if defined(__THW_RS6000) || \
                defined(_IBMR2) || \
                defined(_POWER) || \
                defined(_ARCH_PWR) || \
                defined(_ARCH_PWR2) || \
                defined(_ARCH_PWR3) || \
                defined(_ARCH_PWR4)
            {
                WREN_CODE("static isRS6000 { true }");
            }
            #else
            {
                WREN_CODE("static isRS6000 { false }");
            }
            #endif

            #if defined(__sparc__) || \
                defined(__sparc)
            {
                WREN_CODE("static isSparc { true }");
            }
            #else
            {
                WREN_CODE("static isSparc { false }");
            }
            #endif

            #if defined(__sh__)
            {
                WREN_CODE("static isSuperH { true }");
            }
            #else
            {
                WREN_CODE("static isSuperH { false }");
            }
            #endif

            #if defined(__370__) || \
                defined(__THW_370__) || \
                defined(__s390__) || \
                defined(__s390x__) || \
                defined(__zarch__) || \
                defined(__SYSC_ZARCH__)
            {
                WREN_CODE("static isSystemZ { true }");
            }
            #else
            {
                WREN_CODE("static isSystemZ { false }");
            }
            #endif

            #if defined(_TMS320C2XX) || \
                defined(__TMS320C2000__) || \
                defined(_TMS320C5X) || \
                defined(__TMS320C55X__) || \
                defined(_TMS320C6X) || \
                defined(__TMS320C6X__)
            {
                WREN_CODE("static isTMS320 { true }");
            }
            #else
            {
                WREN_CODE("static isTMS320 { false }");
            }
            #endif

            #if defined(__TMS470__)
            {
                WREN_CODE("static isTMS470 { true }");
            }
            #else
            {
                WREN_CODE("static isTMS470 { false }");
            }
            #endif

            // ===== [ emulation ] =============================================

            // TODO: isWine

            if (!platformPlatformWrenInitEx(vm))
            {
                return false;
            }
        }
        WREN_END_CLASS();
    }

    if (!platformWrenInitEx(vm))
    {
        return false;
    }

    return wrenEndModule(vm);
}

WRENCH_EXPORT void platformWrenQuit(void)
{
    platformWrenQuitEx();
}
