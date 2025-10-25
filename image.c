/* -----------------------------------------------------------------------------
--- Copyright (c) 2012-2026 Adam Schackart / "AJ Hackman", all rights reserved.
--- Distributed under the BSD license v2 (opensource.org/licenses/BSD-3-Clause)
----------------------------------------------------------------------------- */

#define WRENCH_IMPLEMENTATION
#include <image.h>

#define STBI_MALLOC wrench_malloc
#define STBI_REALLOC wrench_realloc
#define STBI_FREE wrench_free

// More verbose error messages.
#define STBI_FAILURE_USERMSG 1

#define STB_IMAGE_IMPLEMENTATION 1
#include <stb/stb_image.h>

#define STBIW_MALLOC wrench_malloc
#define STBIW_REALLOC wrench_realloc
#define STBIW_FREE wrench_free
#define STBIW_MEMMOVE wrench_memmove

#define STB_IMAGE_WRITE_IMPLEMENTATION 1
#include <stb/stb_image_write.h>

//#define STB_IMAGE_RESIZE2_IMPLEMENTATION 1
//#include <stb/stb_image_resize2.h>

/*
================================================================================
 * ~~ [ image ] ~~ *
--------------------------------------------------------------------------------
*/

static void image_Image_ctor(WrenVM* vm)
{
    WRENCH_STUB();
}

static void image_Image_dtor(void* data)
{
    if (((image_Image*)data)->pixels != NULL)
    {
        wrench_free(((image_Image*)data)->pixels);
    }
}

static void image_Image_load(WrenVM* vm)
{
    const char* filename = wrenGetSlotString(vm, 1);
    const int desired_color_channels = wrenGetSlotInt(vm, 2);
    const int desired_bytes_per_channel = wrenGetSlotInt(vm, 3);

    image_Image result = {};
    char error[1024 * 4];

    switch (desired_bytes_per_channel)
    {
        case 0:
        case sizeof(uint8_t):
        {
            result.pixels = (void*)stbi_load(filename, &result.width, &result.height, &result.color_channels, desired_color_channels);
            result.bytes_per_channel = 1;
        }
        break;

        case sizeof(uint16_t):
        {
            result.pixels = (void*)stbi_load_16(filename, &result.width, &result.height, &result.color_channels, desired_color_channels);
            result.bytes_per_channel = 2;
        }
        break;

        case sizeof(float):
        {
            result.pixels = (void*)stbi_loadf(filename, &result.width, &result.height, &result.color_channels, desired_color_channels);
            result.bytes_per_channel = 4;
        }
        break;

        default:
        {
            if (1)
            {
                wrench_snprintf(error, sizeof(error), "Invalid bytes per channel hint for image file \"%s\": %i",
                                                                            filename, desired_bytes_per_channel);
                wrenSetSlotString(vm, 0, (const char*)error);
                wrenAbortFiber(vm, 0);

                return;
            }
            else
            {
                stbi__g_failure_reason = "Invalid bytes per channel hint";
            }
        }
        break;
    }

    if (result.pixels != NULL)
    {
        image_Image* data = (image_Image*)wrenSetSlotNewForeign(vm, 0, 0, sizeof(image_Image));
        *data = result;

        WRENCH_SET_MAGIC_TAG(data, image, Image);
    }
    else
    {
        wrench_snprintf(error, sizeof(error), "Failed to load image file \"%s\": %s.",
                                                    filename, stbi_failure_reason());

        wrenSetSlotString(vm, 0, (const char*)error);
        wrenAbortFiber(vm, 0);
    }
}

static void image_Image_save(WrenVM* vm)
{
    image_Image* self = (image_Image*)wrenGetSlotForeign(vm, 0);
    WRENCH_CHECK_MAGIC_TAG(self, image, Image);

    const char* filename = wrenGetSlotString(vm, 1);
    char error[1024 * 4];

    if (self->pixels == NULL)
    {
        wrench_snprintf(error, sizeof(error), "Failed to save invalid image to \"%s\".", filename);

        error:
        {
            if (error[0] == '\0')
            {
                wrench_snprintf(error, sizeof(error), "Failed to save image to \"%s\".", filename);
            }

            wrenSetSlotString(vm, 0, (const char*)error);
            wrenAbortFiber(vm, 0);

            return;
        }
    }
    else
    {
        error[0] = '\0';
    }

    if (wrench_strstr(filename, ".png") != NULL)
    {
        if (!stbi_write_png(filename, self->width, self->height, self->color_channels, self->pixels, self->width * self->color_channels * self->bytes_per_channel))
        {
            goto error;
        }
    }
    else if (wrench_strstr(filename, ".bmp") != NULL)
    {
        if (!stbi_write_bmp(filename, self->width, self->height, self->color_channels, self->pixels))
        {
            goto error;
        }
    }
    else if (wrench_strstr(filename, ".tga") != NULL)
    {
        if (!stbi_write_tga(filename, self->width, self->height, self->color_channels, self->pixels))
        {
            goto error;
        }
    }
    else if (wrench_strstr(filename, ".hdr") != NULL)
    {
        if (!stbi_write_hdr(filename, self->width, self->height, self->color_channels, self->pixels))
        {
            goto error;
        }
    }
    else if (wrench_strstr(filename, ".jpg") != NULL)
    {
        if (!stbi_write_jpg(filename, self->width, self->height, self->color_channels, self->pixels, 0))
        {
            goto error;
        }
    }
    else
    {
        wrench_snprintf(error, sizeof(error), "No encoder for image file \"%s\".", filename);
        goto error;
    }
}

/*
================================================================================
 * ~~ [ (un)hook ] ~~ *
--------------------------------------------------------------------------------
*/

WRENCH_EXPORT bool imageWrenInit(WrenVM* vm)
{
    if (!wrenBeginModule(vm, "image")) { return false; } else
    {
        WREN_BEGIN_CLASS(image, Image);
        {
            WREN_METHOD(image, Image, true, load, "(filename, desiredColorChannels, desiredBytesPerChannel)", "(_,_,_)");
            WREN_CODE("static load(filename) { load(filename, 0, 0) }");

            // TODO: loadFromBytes

            // TODO: info
            // TODO: infoFromBytes

            WREN_METHOD(image, Image, false, save, "(path)", "(_)");

            // TODO: saveToBytes

            // TODO: name
            // TODO: path

            // TODO: toString

            // TODO: [x, y]
            // TODO: [x, y]=

            WREN_CODE("static MONO { 1 }");
            WREN_CODE("static RGB { 3 }");
            WREN_CODE("static RGBA { 4 }");

            WREN_CODE("static BYTE { 1 }");
            WREN_CODE("static SHORT { 2 }");
            WREN_CODE("static FLOAT { 4 }");

            // TODO: data
            // TODO: colorChannels
            // TODO: colorChannels=
            // TODO: isMono
            // TODO: isMono=
            // TODO: isRGB
            // TODO: isRGB=
            // TODO: isRGBA
            // TODO: isRGBA=
            // TODO: bytesPerChannel
            // TODO: bytesPerChannel=
            // TODO: bytesPerPixel
            // TODO: bytesPerPixel=
            // TODO: width
            // TODO: height
            // TODO: pitch

            // TODO: resize
            // TODO: convert
        }
        WREN_END_CLASS();
    }

    return wrenEndModule(vm);
}

WRENCH_EXPORT void imageWrenQuit(void)
{
    //
}
