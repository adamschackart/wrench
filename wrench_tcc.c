/* -----------------------------------------------------------------------------
--- Copyright (c) 2012-2026 Adam Schackart / "AJ Hackman", all rights reserved.
--- Distributed under the BSD license v2 (opensource.org/licenses/BSD-3-Clause)
----------------------------------------------------------------------------- */

#ifndef WRENCH_IMPLEMENTATION
#define WRENCH_IMPLEMENTATION 1
#endif
#include <wrench_tcc.h>

/*
================================================================================
 * ~~ [ state ] ~~ *
--------------------------------------------------------------------------------
*/

static void tcc_State_error(void *opaque, const char *msg)
{
    wrenSetErrorString((WrenVM*)opaque, msg);
}

static void* tcc_State_realloc(void *ptr, unsigned long size)
{
    return wrench_realloc(ptr, size);
}

static void tcc_State_ctor(WrenVM* vm)
{
    /* XXX: We're basically treating this as "security hardened" mode. Disable compiler.
     */
    if (!wrenGetForeignLibraryLoadEnabled(vm))
    {
        wrenSetSlotString(vm, 0, "Foreign code loading is disabled - cannot create TCC state.");
        wrenAbortFiber(vm, 0);

        return;
    }

    TCCState* state = tcc_new();

    if (state != NULL)
    {
        tcc_State* self = (tcc_State*)wrenSetSlotNewForeign(vm, 0, 0, sizeof(tcc_State));
        WRENCH_SET_MAGIC_TAG(self, tcc, State);

        tcc_set_error_func(state, vm, tcc_State_error);

        // FIXME: Compiling against an older version of libtcc that doesn't declare this.
        // tcc_set_realloc(tcc_State_realloc);

        self->state = state;
    }
    else
    {
        wrenSetSlotString(vm, 0, "Out of memory - failed allocate TCC state.");
        wrenAbortFiber(vm, 0);
    }
}

static void tcc_State_dtor(void* data)
{
    tcc_delete(((tcc_State*)data)->state);
}

static void tcc_State_setLibPath(WrenVM* vm)
{
    tcc_State* self = (tcc_State*)wrenGetSlotForeign(vm, 0);
    WRENCH_CHECK_MAGIC_TAG(self, tcc, State);

    tcc_set_lib_path(self->state, wrenGetSlotString(vm, 1));
}

static void tcc_State_setOptions(WrenVM* vm)
{
    tcc_State* self = (tcc_State*)wrenGetSlotForeign(vm, 0);
    WRENCH_CHECK_MAGIC_TAG(self, tcc, State);

    /* XXX: Older versions of TCC don't return error values.
     */
    #if 0
    {
        if (tcc_set_options(self->state, wrenGetSlotString(vm, 1)) < 0)
        {
            wrenSetSlotString(vm, 0, wrenGetErrorString(vm));
            wrenAbortFiber(vm, 0);
        }
    }
    #else
    {
        tcc_set_options(self->state, wrenGetSlotString(vm, 1));
    }
    #endif
}

static void tcc_State_addIncludePath(WrenVM* vm)
{
    tcc_State* self = (tcc_State*)wrenGetSlotForeign(vm, 0);
    WRENCH_CHECK_MAGIC_TAG(self, tcc, State);

    if (tcc_add_include_path(self->state, wrenGetSlotString(vm, 1)) < 0)
    {
        wrenSetSlotString(vm, 0, wrenGetErrorString(vm));
        wrenAbortFiber(vm, 0);
    }
}

static void tcc_State_addSysIncludePath(WrenVM* vm)
{
    tcc_State* self = (tcc_State*)wrenGetSlotForeign(vm, 0);
    WRENCH_CHECK_MAGIC_TAG(self, tcc, State);

    if (tcc_add_sysinclude_path(self->state, wrenGetSlotString(vm, 1)) < 0)
    {
        wrenSetSlotString(vm, 0, wrenGetErrorString(vm));
        wrenAbortFiber(vm, 0);
    }
}

static void tcc_State_defineSymbol(WrenVM* vm)
{
    tcc_State* self = (tcc_State*)wrenGetSlotForeign(vm, 0);
    WRENCH_CHECK_MAGIC_TAG(self, tcc, State);

    tcc_define_symbol(self->state, wrenGetSlotString(vm, 1), wrenGetSlotString(vm, 2));
}

static void tcc_State_undefineSymbol(WrenVM* vm)
{
    tcc_State* self = (tcc_State*)wrenGetSlotForeign(vm, 0);
    WRENCH_CHECK_MAGIC_TAG(self, tcc, State);

    tcc_undefine_symbol(self->state, wrenGetSlotString(vm, 1));
}

static void tcc_State_addFile(WrenVM* vm)
{
    tcc_State* self = (tcc_State*)wrenGetSlotForeign(vm, 0);
    WRENCH_CHECK_MAGIC_TAG(self, tcc, State);

    if (tcc_add_file(self->state, wrenGetSlotString(vm, 1)) < 0)
    {
        wrenSetSlotString(vm, 0, wrenGetErrorString(vm));
        wrenAbortFiber(vm, 0);
    }
}

static void tcc_State_compileString(WrenVM* vm)
{
    tcc_State* self = (tcc_State*)wrenGetSlotForeign(vm, 0);
    WRENCH_CHECK_MAGIC_TAG(self, tcc, State);

    if (tcc_compile_string(self->state, wrenGetSlotString(vm, 1)) < 0)
    {
        wrenSetSlotString(vm, 0, wrenGetErrorString(vm));
        wrenAbortFiber(vm, 0);
    }
}

static void tcc_State_setOutputType(WrenVM* vm)
{
    tcc_State* self = (tcc_State*)wrenGetSlotForeign(vm, 0);
    WRENCH_CHECK_MAGIC_TAG(self, tcc, State);

    const char* s = wrenGetSlotString(vm, 1);
    int type;

    if (wrench_strcasecmp(s, "memory"))
    {
        type = TCC_OUTPUT_MEMORY;
    }
    else if (wrench_strcasecmp(s, "exe"))
    {
        type = TCC_OUTPUT_EXE;
    }
    else if (wrench_strcasecmp(s, "dll"))
    {
        type = TCC_OUTPUT_DLL;
    }
    else if (wrench_strcasecmp(s, "obj"))
    {
        type = TCC_OUTPUT_OBJ;
    }
    else if (wrench_strcasecmp(s, "preprocess"))
    {
        type = TCC_OUTPUT_PREPROCESS;
    }
    else
    {
        wrench_assert(0, "%s", s);
        type = 0;
    }

    if (tcc_set_output_type(self->state, type) < 0)
    {
        wrenSetSlotString(vm, 0, wrenGetErrorString(vm));
        wrenAbortFiber(vm, 0);
    }
}

static void tcc_State_addLibraryPath(WrenVM* vm)
{
    tcc_State* self = (tcc_State*)wrenGetSlotForeign(vm, 0);
    WRENCH_CHECK_MAGIC_TAG(self, tcc, State);

    if (tcc_add_library_path(self->state, wrenGetSlotString(vm, 1)) < 0)
    {
        wrenSetSlotString(vm, 0, wrenGetErrorString(vm));
        wrenAbortFiber(vm, 0);
    }
}

static void tcc_State_addLibrary(WrenVM* vm)
{
    tcc_State* self = (tcc_State*)wrenGetSlotForeign(vm, 0);
    WRENCH_CHECK_MAGIC_TAG(self, tcc, State);

    if (tcc_add_library(self->state, wrenGetSlotString(vm, 1)) < 0)
    {
        wrenSetSlotString(vm, 0, wrenGetErrorString(vm));
        wrenAbortFiber(vm, 0);
    }
}

static void tcc_State_outputFile(WrenVM* vm)
{
    tcc_State* self = (tcc_State*)wrenGetSlotForeign(vm, 0);
    WRENCH_CHECK_MAGIC_TAG(self, tcc, State);

    if (tcc_output_file(self->state, wrenGetSlotString(vm, 1)) < 0)
    {
        wrenSetSlotString(vm, 0, wrenGetErrorString(vm));
        wrenAbortFiber(vm, 0);
    }
}

static void tcc_State_run(WrenVM* vm)
{
    tcc_State* self = (tcc_State*)wrenGetSlotForeign(vm, 0);
    WRENCH_CHECK_MAGIC_TAG(self, tcc, State);

    wrench_assert(wrenGetListCount(vm, 1) == 0, "TODO");
    wrenSetSlotInt(vm, 0, tcc_run(self->state, 0, NULL));
}

static void tcc_State_relocate(WrenVM* vm)
{
    tcc_State* self = (tcc_State*)wrenGetSlotForeign(vm, 0);
    WRENCH_CHECK_MAGIC_TAG(self, tcc, State);

    /* XXX: Older versions of TCC take this mysterious pointer.
     */
    if (tcc_relocate(self->state, TCC_RELOCATE_AUTO) < 0)
    {
        wrenSetSlotString(vm, 0, wrenGetErrorString(vm));
        wrenAbortFiber(vm, 0);
    }
}

/*
================================================================================
 * ~~ [ (un)hook ] ~~ *
--------------------------------------------------------------------------------
*/

#if WRENCH_TCC_EXTENDED
    /*
     * Enable user extension of stdlib modules.
     */
    #ifndef __TCC_EX_INL__
    #include <tcc_ex.inl>
    #endif
#else
    static bool tccWrenInitEx(WrenVM* vm)
    {
        return true;
    }

    static void tccWrenQuitEx(void)
    {
        //
    }

    static bool tccStateWrenInitEx(WrenVM* vm)
    {
        return true;
    }
#endif /* WRENCH_TCC_EXTENDED */

WRENCH_EXPORT bool tccWrenInit(WrenVM* vm)
{
    if (!wrenBeginModule(vm, "tcc")) { return false; } else
    {
        WREN_BEGIN_CLASS(tcc, State);
        {
            WREN_CODE("construct new() {}");

            WREN_METHOD(tcc, State, false, setLibPath, "(path_string)", "(_)");
            WREN_METHOD(tcc, State, false, setOptions, "(options_string)", "(_)");
            WREN_METHOD(tcc, State, false, addIncludePath, "(path_string)", "(_)");
            WREN_METHOD(tcc, State, false, addSysIncludePath, "(path_string)", "(_)");
            WREN_METHOD(tcc, State, false, defineSymbol, "(symbol, value)", "(_,_)");
            WREN_METHOD(tcc, State, false, undefineSymbol, "(symbol)", "(_)");
            WREN_METHOD(tcc, State, false, addFile, "(filename)", "(_)");
            WREN_METHOD(tcc, State, false, compileString, "(string)", "(_)");
            WREN_METHOD(tcc, State, false, setOutputType, "(type)", "(_)");
            WREN_METHOD(tcc, State, false, addLibraryPath, "(library_path)", "(_)");
            WREN_METHOD(tcc, State, false, addLibrary, "(library_name)", "(_)");
            // TODO: addSymbol
            WREN_METHOD(tcc, State, false, outputFile, "(filename)", "(_)");
            WREN_METHOD(tcc, State, false, run, "(args)", "(_)");
            WREN_METHOD(tcc, State, false, relocate, "()", "()");
            // TODO: getSymbol
            // TODO: listSymbols
            // TODO: setjmp
            // TODO: setBackTraceFunc

            if (!tccStateWrenInitEx(vm))
            {
                return false;
            }
        }
        WREN_END_CLASS();
    }

    if (!tccWrenInitEx(vm))
    {
        return false;
    }

    return wrenEndModule(vm);
}

WRENCH_EXPORT void tccWrenQuit(void)
{
    tccWrenQuitEx();
}
