/* -----------------------------------------------------------------------------
--- Copyright (c) 2012-2026 Adam Schackart / "AJ Hackman", all rights reserved.
--- Distributed under the BSD license v2 (opensource.org/licenses/BSD-3-Clause)
----------------------------------------------------------------------------- */
#ifndef __WRENCH_ZIP_H__
#define __WRENCH_ZIP_H__

#include <wrench.h>

// Forward declare.
struct zip_t;

WRENCH_STRUCT_HEADER(zip, Archive)
    struct zip_t* zip;
    // TODO: bool collect
WRENCH_STRUCT_FOOTER(zip, Archive)

#endif /* __WRENCH_ZIP_H__ */
