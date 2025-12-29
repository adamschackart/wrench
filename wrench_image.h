/* -----------------------------------------------------------------------------
--- Copyright (c) 2012-2026 Adam Schackart / "AJ Hackman", all rights reserved.
--- Distributed under the BSD license v2 (opensource.org/licenses/BSD-3-Clause)
----------------------------------------------------------------------------- */
#ifndef __WRENCH_IMAGE_H__
#define __WRENCH_IMAGE_H__

#include <wrench.h>

WRENCH_STRUCT_HEADER(image, Image)
    void* pixels;
    int width;
    int height;
    int color_channels;
    int bytes_per_channel;
WRENCH_STRUCT_FOOTER(image, Image)

#endif /* __WRENCH_IMAGE_H__ */
