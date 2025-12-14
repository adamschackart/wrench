/* -----------------------------------------------------------------------------
--- Copyright (c) 2012-2026 Adam Schackart / "AJ Hackman", all rights reserved.
--- Distributed under the BSD license v2 (opensource.org/licenses/BSD-3-Clause)
----------------------------------------------------------------------------- */
#ifndef __WRENCH_PROCESS_H__
#define __WRENCH_PROCESS_H__

#include <wrench.h>

#ifndef SHEREDOM_SUBPROCESS_H_INCLUDED
#include <subprocess.h/subprocess.h>
#endif

typedef struct process_Process
{
    WRENCH_MAGIC_TAG;
    struct subprocess_s process;
}
process_Process;

#endif /* __WRENCH_PROCESS_H__ */
