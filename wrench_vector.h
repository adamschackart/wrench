/* -----------------------------------------------------------------------------
--- Copyright (c) 2012-2026 Adam Schackart / "AJ Hackman", all rights reserved.
--- Distributed under the BSD license v2 (opensource.org/licenses/BSD-3-Clause)
----------------------------------------------------------------------------- */
#ifndef __WRENCH_VECTOR_H__
#define __WRENCH_VECTOR_H__

#include <wrench.h>

WRENCH_STRUCT_HEADER(vector, IntVector)
    size_t dimensions;
    int* elements;
    bool collect;
WRENCH_STRUCT_FOOTER(vector, IntVector)

WRENCH_STRUCT_HEADER(vector, FltVector)
    size_t dimensions;
    float* elements;
    bool collect;
WRENCH_STRUCT_FOOTER(vector, FltVector)

WRENCH_STRUCT_HEADER(vector, DblVector)
    size_t dimensions;
    double* elements;
    bool collect;
WRENCH_STRUCT_FOOTER(vector, DblVector)

#endif /* __WRENCH_VECTOR_H__ */
