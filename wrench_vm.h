/* -----------------------------------------------------------------------------
--- Copyright (c) 2012-2026 Adam Schackart / "AJ Hackman", all rights reserved.
--- Distributed under the BSD license v2 (opensource.org/licenses/BSD-3-Clause)
----------------------------------------------------------------------------- */
#ifndef __WRENCH_VM_H__
#define __WRENCH_VM_H__

#include <wrench.h>

typedef struct vm_WrenConfiguration
{
    WRENCH_MAGIC_TAG;

    /* Avoid an extra allocation.
     */
    WrenConfiguration* config;
    WrenConfiguration _config;
}
vm_WrenConfiguration;

typedef struct vm_WrenVM
{
    WRENCH_MAGIC_TAG;
    WrenVM* vm;

    bool extended;
    bool collect;
}
vm_WrenVM;

#endif /* __WRENCH_VM_H__ */
