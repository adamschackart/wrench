/* -----------------------------------------------------------------------------
--- Copyright (c) 2012-2026 Adam Schackart / "AJ Hackman", all rights reserved.
--- Distributed under the BSD license v2 (opensource.org/licenses/BSD-3-Clause)
----------------------------------------------------------------------------- */
#ifndef __WRENCH_VECTOR_H__
#define __WRENCH_VECTOR_H__

#include <wrench.h>

typedef struct vector_IntVector
{
    WRENCH_MAGIC_TAG;

    size_t dimensions;
    int* elements;
    bool collect;
}
vector_IntVector;

typedef struct vector_FltVector
{
    WRENCH_MAGIC_TAG;

    size_t dimensions;
    float* elements;
    bool collect;
}
vector_FltVector;

typedef struct vector_DblVector
{
    WRENCH_MAGIC_TAG;

    size_t dimensions;
    double* elements;
    bool collect;
}
vector_DblVector;

#endif /* __WRENCH_VECTOR_H__ */
