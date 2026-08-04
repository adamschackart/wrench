/* -----------------------------------------------------------------------------
--- Copyright (c) 2012-2026 Adam Schackart / "AJ Hackman", all rights reserved.
--- Distributed under the BSD license v2 (opensource.org/licenses/BSD-3-Clause)
----------------------------------------------------------------------------- */
#ifndef __WRENCH_VM_H__
#define __WRENCH_VM_H__

#include <wrench.h>

WRENCH_STRUCT_HEADER(vm, WrenConfiguration)
    /*
     * Avoid an extra allocation.
     */
    WrenConfiguration* config;
    WrenConfiguration _config;
WRENCH_STRUCT_FOOTER(vm, WrenConfiguration)

WRENCH_STRUCT_HEADER(vm, WrenVM)
    WrenVM* vm;

    bool extended;
    bool collect;
WRENCH_STRUCT_FOOTER(vm, WrenVM)

#if 1
    struct wrench_preprocessor_t; // Opaque preprocessor struct.
    typedef struct wrench_preprocessor_t* wrench_preprocessor_p;
#else
    #include <wrench_preprocessor.h>
#endif

WRENCH_STRUCT_HEADER(vm, Preprocessor)
    wrench_preprocessor_p preprocessor;
    bool collect;
WRENCH_STRUCT_FOOTER(vm, Preprocessor)

#if 1
    struct gasket_context_t; // Opaque context struct.
    typedef struct gasket_context_t* gasket_context_p;
#else
    #include <wrench_gasket.h>
#endif

WRENCH_STRUCT_HEADER(vm, Gasket)
    gasket_context_p context;
    bool collect;
WRENCH_STRUCT_FOOTER(vm, Gasket)

#endif /* __WRENCH_VM_H__ */
