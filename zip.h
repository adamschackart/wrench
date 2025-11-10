/* -----------------------------------------------------------------------------
--- Copyright (c) 2012-2026 Adam Schackart / "AJ Hackman", all rights reserved.
--- Distributed under the BSD license v2 (opensource.org/licenses/BSD-3-Clause)
----------------------------------------------------------------------------- */
#ifndef __WRENCH_ZIP_H__
#define __WRENCH_ZIP_H__

#include <wrench.h>

// Forward declare.
struct zip_t;

typedef struct zip_Archive
{
    WRENCH_MAGIC_TAG;
    struct zip_t* zip;
}
zip_Archive;

#endif /* __WRENCH_ZIP_H__ */
