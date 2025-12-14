/* -----------------------------------------------------------------------------
--- Copyright (c) 2012-2026 Adam Schackart / "AJ Hackman", all rights reserved.
--- Distributed under the BSD license v2 (opensource.org/licenses/BSD-3-Clause)
----------------------------------------------------------------------------- */
#ifndef __WRENCH_RECT_H__
#define __WRENCH_RECT_H__

#include <wrench.h>

typedef struct rect_IntRect
{
    WRENCH_MAGIC_TAG;
    int xywh[4];
}
rect_IntRect;

typedef struct rect_FltRect
{
    WRENCH_MAGIC_TAG;
    float xywh[4];
}
rect_FltRect;

typedef struct rect_DblRect
{
    WRENCH_MAGIC_TAG;
    double xywh[4];
}
rect_DblRect;

#endif /* __WRENCH_RECT_H__ */
