/* -----------------------------------------------------------------------------
--- Copyright (c) 2012-2026 Adam Schackart / "AJ Hackman", all rights reserved.
--- Distributed under the BSD license v2 (opensource.org/licenses/BSD-3-Clause)
----------------------------------------------------------------------------- */
#ifndef __WRENCH_RECT_H__
#define __WRENCH_RECT_H__

#include <wrench.h>

WRENCH_STRUCT_HEADER(rect, IntRect)
    int xywh[4];
WRENCH_STRUCT_FOOTER(rect, IntRect)

WRENCH_STRUCT_HEADER(rect, FltRect)
    float xywh[4];
WRENCH_STRUCT_FOOTER(rect, FltRect)

WRENCH_STRUCT_HEADER(rect, DblRect)
    double xywh[4];
WRENCH_STRUCT_FOOTER(rect, DblRect)

#endif /* __WRENCH_RECT_H__ */
