/* -----------------------------------------------------------------------------
--- Copyright (c) 2012-2026 Adam Schackart / "AJ Hackman", all rights reserved.
--- Distributed under the BSD license v2 (opensource.org/licenses/BSD-3-Clause)
----------------------------------------------------------------------------- */
#ifndef __WRENCH_TCC_H__
#define __WRENCH_TCC_H__

#include <libtcc.h>
#include <wrench.h>

typedef struct tcc_State
{
    WRENCH_MAGIC_TAG;
    TCCState* state;
}
tcc_State;

#endif /* __WRENCH_TCC_H__ */
