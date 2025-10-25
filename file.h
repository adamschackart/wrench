/* -----------------------------------------------------------------------------
--- Copyright (c) 2012-2026 Adam Schackart / "AJ Hackman", all rights reserved.
--- Distributed under the BSD license v2 (opensource.org/licenses/BSD-3-Clause)
----------------------------------------------------------------------------- */
#ifndef __WRENCH_FILE_H__
#define __WRENCH_FILE_H__

#include <wrench.h>

typedef struct file_File
{
    WRENCH_MAGIC_TAG;
    FILE* file;
}
file_File;

#endif /* __WRENCH_FILE_H__ */
