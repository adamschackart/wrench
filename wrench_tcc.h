/* -----------------------------------------------------------------------------
--- Copyright (c) 2012-2026 Adam Schackart / "AJ Hackman", all rights reserved.
--- Distributed under the BSD license v2 (opensource.org/licenses/BSD-3-Clause)
----------------------------------------------------------------------------- */
#ifndef __WRENCH_TCC_H__
#define __WRENCH_TCC_H__

#include <libtcc.h>
#include <wrench.h>

WRENCH_STRUCT_HEADER(tcc, State)
    TCCState* state;
WRENCH_STRUCT_FOOTER(tcc, State)

#endif /* __WRENCH_TCC_H__ */
