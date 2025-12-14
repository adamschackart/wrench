/* -----------------------------------------------------------------------------
--- Copyright (c) 2012-2026 Adam Schackart / "AJ Hackman", all rights reserved.
--- Distributed under the BSD license v2 (opensource.org/licenses/BSD-3-Clause)
----------------------------------------------------------------------------- */
#ifndef __WRENCH_IMAGE_H__
#define __WRENCH_IMAGE_H__

#include <wrench.h>

typedef struct image_Image
{
    WRENCH_MAGIC_TAG;

    void* pixels;
    int width;
    int height;
    int color_channels;
    int bytes_per_channel;
}
image_Image;

#endif /* __WRENCH_IMAGE_H__ */
