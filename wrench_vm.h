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

#endif /* __WRENCH_VM_H__ */
