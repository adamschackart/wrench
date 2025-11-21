/* -----------------------------------------------------------------------------
--- Copyright (c) 2012-2026 Adam Schackart / "AJ Hackman", all rights reserved.
--- Distributed under the BSD license v2 (opensource.org/licenses/BSD-3-Clause)
----------------------------------------------------------------------------- */

#ifndef WRENCH_IMPLEMENTATION
#define WRENCH_IMPLEMENTATION 1
#endif
#include <image.h>
#include <rect.h>
#include <vector.h>

/* Image loading.
 */
#if !defined(STB_IMAGE_IMPLEMENTATION)
    #define STBI_MALLOC wrench_malloc
    #define STBI_REALLOC wrench_realloc
    #define STBI_FREE wrench_free

    // More verbose error messages.
    #define STBI_FAILURE_USERMSG 1

    #define STB_IMAGE_IMPLEMENTATION 1
    #include <stb/stb_image.h>
#endif

/* Image saving.
 */
#if !defined(STB_IMAGE_WRITE_IMPLEMENTATION)
    #define STBIW_MALLOC wrench_malloc
    #define STBIW_REALLOC wrench_realloc
    #define STBIW_FREE wrench_free
    #define STBIW_MEMMOVE wrench_memmove

    #define STB_IMAGE_WRITE_IMPLEMENTATION 1
    #include <stb/stb_image_write.h>
#endif

/* Image resizing.
 */
#if !defined(STB_IMAGE_RESIZE2_IMPLEMENTATION) && 0
    #define STBIR_MALLOC wrench_malloc
    #define STBIR_FREE wrench_free

    #define STB_IMAGE_RESIZE2_IMPLEMENTATION 1
    #include <stb/stb_image_resize2.h>
#endif

/*
================================================================================
 * ~~ [ image ] ~~ *
--------------------------------------------------------------------------------
*/

static void image_Image_ctor(WrenVM* vm)
{
    /* TODO: Lower-level C image reallocation function that we can re-use elsewhere.
     */
    const int width = wrenGetSlotInt(vm, 1);
    const int height = wrenGetSlotInt(vm, 2);
    const int color_channels = wrenGetSlotInt(vm, 3);
    const int bytes_per_channel = wrenGetSlotInt(vm, 4);

    char error[1024 * 4];
    void* pixels = wrench_calloc(1, width * height * color_channels * bytes_per_channel);

    if (pixels != NULL)
    {
        image_Image* self = (image_Image*)wrenSetSlotNewForeign(vm, 0, 0, sizeof(image_Image));
        WRENCH_SET_MAGIC_TAG(self, image, Image);

        self->pixels = pixels;
        self->width = width;
        self->height = height;
        self->color_channels = color_channels;
        self->bytes_per_channel = bytes_per_channel;
    }
    else
    {
        wrench_snprintf(error, sizeof(error), "Failed to allocate %ix%ix%ix%i image.",
                                    width, height, color_channels, bytes_per_channel);

        wrenSetSlotString(vm, 0, (const char*)error);
        wrenAbortFiber(vm, 0);
    }
}

static void image_Image_dtor(void* data)
{
    WRENCH_CHECK_MAGIC_TAG(data, image, Image);
    wrench_free(((image_Image*)data)->pixels);
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

static void image_Image_loadFromBytes(WrenVM* vm)
{
    int size;
    const char* data = wrenGetSlotBytes(vm, 1, &size);

    const int desired_color_channels = wrenGetSlotInt(vm, 2);
    const int desired_bytes_per_channel = wrenGetSlotInt(vm, 3);

    image_Image result = {};
    char error[1024 * 4];

    // HACK
    const char* filename = "memory";

    switch (desired_bytes_per_channel)
    {
        case 0:
        case sizeof(uint8_t):
        {
            result.pixels = (void*)stbi_load_from_memory((stbi_uc const*)data, size, &result.width, &result.height, &result.color_channels, desired_color_channels);
            result.bytes_per_channel = 1;
        }
        break;

        case sizeof(uint16_t):
        {
            result.pixels = (void*)stbi_load_16_from_memory((stbi_uc const*)data, size, &result.width, &result.height, &result.color_channels, desired_color_channels);
            result.bytes_per_channel = 2;
        }
        break;

        case sizeof(float):
        {
            result.pixels = (void*)stbi_loadf_from_memory((stbi_uc const*)data, size, &result.width, &result.height, &result.color_channels, desired_color_channels);
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
        if (!stbi_write_hdr(filename, self->width, self->height, self->color_channels, (const float*)self->pixels))
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

static void image_Image_width_get(WrenVM* vm)
{
    image_Image* self = (image_Image*)wrenGetSlotForeign(vm, 0);
    WRENCH_CHECK_MAGIC_TAG(self, image, Image);

    wrenSetSlotInt(vm, 0, self->width);
}

static void image_Image_height_get(WrenVM* vm)
{
    image_Image* self = (image_Image*)wrenGetSlotForeign(vm, 0);
    WRENCH_CHECK_MAGIC_TAG(self, image, Image);

    wrenSetSlotInt(vm, 0, self->height);
}

static void image_Image_colorChannels_get(WrenVM* vm)
{
    image_Image* self = (image_Image*)wrenGetSlotForeign(vm, 0);
    WRENCH_CHECK_MAGIC_TAG(self, image, Image);

    wrenSetSlotInt(vm, 0, self->color_channels);
}

static void image_Image_bytesPerChannel_get(WrenVM* vm)
{
    image_Image* self = (image_Image*)wrenGetSlotForeign(vm, 0);
    WRENCH_CHECK_MAGIC_TAG(self, image, Image);

    wrenSetSlotInt(vm, 0, self->bytes_per_channel);
}

static bool image_Image_yUp = false;

static void image_Image_yUp_get(WrenVM* vm)
{
    wrenSetSlotBool(vm, 0, image_Image_yUp);
}

static void image_Image_yUp_set(WrenVM* vm)
{
    image_Image_yUp = wrenGetSlotBool(vm, 1);
}

static void image_Image_index2_get(WrenVM* vm)
{
    char error[1024 * 4];

    image_Image* self = (image_Image*)wrenGetSlotForeign(vm, 0);
    WRENCH_CHECK_MAGIC_TAG(self, image, Image);

    wrench_assert(self->pixels != NULL, "invalid image");

    int x = wrenGetSlotInt(vm, 1);
    int y = wrenGetSlotInt(vm, 2);

    if (x < 0 || y < 0 || x >= self->width || y >= self->height)
    {
        wrench_snprintf(error, sizeof(error), "Image%ix%i[%i, %i]", self->width, self->height, x, y);

        wrenSetSlotString(vm, 0, (const char*)error);
        wrenAbortFiber(vm, 0);

        return;
    }

    if (image_Image_yUp) y = self->height - y - 1;

    switch (self->bytes_per_channel)
    {
        case sizeof(uint8_t):
        {
            switch (self->color_channels)
            {
                case 1:
                {
                    wrench_assert(0, "TODO");
                }
                break;

                case 3:
                {
                    wrench_assert(0, "TODO");
                }
                break;

                case 4:
                {
                    wrenSetSlotInt(vm, 0, ((int32_t*)self->pixels)[y * self->width + x]);
                }
                break;

                default:
                {
                    wrench_assert(0, "%i", self->color_channels);
                }
                break;
            }
        }
        break;

        case sizeof(uint16_t):
        {
            switch (self->color_channels)
            {
                case 1:
                {
                    wrench_assert(0, "TODO");
                }
                break;

                case 3:
                {
                    wrench_assert(0, "TODO");
                }
                break;

                case 4:
                {
                    wrench_assert(0, "TODO");
                }
                break;

                default:
                {
                    wrench_assert(0, "%i", self->color_channels);
                }
                break;
            }
        }
        break;

        case sizeof(float):
        {
            WrenchContext* context = (WrenchContext*)wrenGetUserData(vm);
            wrench_assert(context != NULL, "");

            if (context->FltVector_handle != NULL)
            {
                wrenSetSlotHandle(vm, 0, context->FltVector_handle);
            }
            else
            {
                wrenGetVariable(vm, "vector", "FltVector", 0);
                context->FltVector_handle = wrenGetSlotHandle(vm, 0);
            }

            vector_FltVector* rgba = (vector_FltVector*)wrenSetSlotNewForeign(vm, 0, 0, sizeof(vector_FltVector));
            WRENCH_SET_MAGIC_TAG(rgba, vector, FltVector);

            if (0)
            {
                // TODO `vector_FltVector_alloc` so we could use context small block allocator.
                float* elements = (float*)wrench_malloc(sizeof(float) * self->color_channels);

                if (elements == NULL)
                {
                    char error[1024 * 4];
                    wrench_snprintf(error, sizeof(error), "Out of memory! Failed to allocate FltVector of size %i.", self->color_channels);

                    wrenSetSlotString(vm, 0, (const char*)error);
                    wrenAbortFiber(vm, 0);

                    return;
                }

                for (int i = 0; i < self->color_channels; i++)
                {
                    elements[i] = ((float*)self->pixels)[y * self->width * self->color_channels + x * self->color_channels + i];
                }

                // TODO: Use `dimensions` MSB as flag.
                rgba->collect = true;

                rgba->elements = elements;
            }
            else
            {
                rgba->elements = (float*)self->pixels + y * self->width * self->color_channels + x * self->color_channels;
            }

            rgba->dimensions = self->color_channels;
        }
        break;

        default:
        {
            wrench_assert(0, "%i", self->bytes_per_channel);
        }
        break;
    }
}

static void image_Image_index2_set(WrenVM* vm)
{
    char error[1024 * 4];

    image_Image* self = (image_Image*)wrenGetSlotForeign(vm, 0);
    WRENCH_CHECK_MAGIC_TAG(self, image, Image);

    wrench_assert(self->pixels != NULL, "invalid image");

    int x = wrenGetSlotInt(vm, 1);
    int y = wrenGetSlotInt(vm, 2);

    if (x < 0 || y < 0 || x >= self->width || y >= self->height)
    {
        wrench_snprintf(error, sizeof(error), "Image%ix%i[%i, %i]", self->width, self->height, x, y);

        wrenSetSlotString(vm, 0, (const char*)error);
        wrenAbortFiber(vm, 0);

        return;
    }

    if (image_Image_yUp) y = self->height - y - 1;

    switch (self->bytes_per_channel)
    {
        case sizeof(uint8_t):
        {
            switch (self->color_channels)
            {
                case 1:
                {
                    wrench_assert(0, "TODO");
                }
                break;

                case 3:
                {
                    wrench_assert(0, "TODO");
                }
                break;

                case 4:
                {
                    if (0) // XXX TODO FIXME: Broken.
                    {
                        ((int32_t*)self->pixels)[y * self->width + x] = wrenGetSlotInt(vm, 3);
                    }
                    else
                    {
                        wrench_assert(0, "TODO");
                    }
                }
                break;

                default:
                {
                    wrench_assert(0, "%i", self->color_channels);
                }
                break;
            }
        }
        break;

        case sizeof(uint16_t):
        {
            switch (self->color_channels)
            {
                case 1:
                {
                    wrench_assert(0, "TODO");
                }
                break;

                case 3:
                {
                    wrench_assert(0, "TODO");
                }
                break;

                case 4:
                {
                    wrench_assert(0, "TODO");
                }
                break;

                default:
                {
                    wrench_assert(0, "%i", self->color_channels);
                }
                break;
            }
        }
        break;

        case sizeof(float):
        {
            if (self->color_channels == 1 && wrenGetSlotType(vm, 3) == WREN_TYPE_NUM)
            {
                ((float*)self->pixels)[y * self->width + x] = wrenGetSlotFloat(vm, 3);
            }
            else
            {
                vector_FltVector* rgba = (vector_FltVector*)wrenGetSlotForeign(vm, 3);
                WRENCH_CHECK_MAGIC_TAG(rgba, vector, FltVector);

                wrench_assert((int)rgba->dimensions >= self->color_channels, "FltVector%i < %i", (int)rgba->dimensions, self->color_channels);
                wrench_assert(rgba->elements != NULL, "");

                for (int i = 0; i < self->color_channels; i++)
                {
                    ((float*)self->pixels)[y * self->width * self->color_channels + x * self->color_channels + i] = rgba->elements[i];
                }
            }
        }
        break;

        default:
        {
            wrench_assert(0, "%i", self->bytes_per_channel);
        }
        break;
    }
}

static void image_Image_resize(WrenVM* vm)
{
    WRENCH_STUB();
}

static void image_Image_convert(WrenVM* vm)
{
    char error[1024 * 4];

    image_Image* self = (image_Image*)wrenGetSlotForeign(vm, 0);
    WRENCH_CHECK_MAGIC_TAG(self, image, Image);

    const int old_color_channels = self->color_channels;
    const int old_bytes_per_channel = self->bytes_per_channel;

    const int new_color_channels = wrenGetSlotInt(vm, 1);
    const int new_bytes_per_channel = wrenGetSlotInt(vm, 2);

    void* old_pixels = self->pixels;
    void* new_pixels = wrench_malloc(self->width * self->height * new_color_channels * new_bytes_per_channel);

    if (new_pixels != NULL)
    {
        // TODO: Slow!!! Should use a WrenHandle.
        wrenGetVariable(vm, "image", "Image", 0);

        image_Image* copy = (image_Image*)wrenSetSlotNewForeign(vm, 0, 0, sizeof(image_Image));
        WRENCH_SET_MAGIC_TAG(copy, image, Image);

        copy->pixels = new_pixels;
        copy->width = self->width;
        copy->height = self->height;
        copy->color_channels = new_color_channels;
        copy->bytes_per_channel = new_bytes_per_channel;
    }
    else
    {
        wrench_snprintf(error, sizeof(error), "Failed to allocate %ix%ix%ix%i image.",
                self->width, self->height, new_color_channels, new_bytes_per_channel);

        wrenSetSlotString(vm, 0, (const char*)error);
        wrenAbortFiber(vm, 0);

        return;
    }

    if (old_color_channels == new_color_channels && old_bytes_per_channel == new_bytes_per_channel)
    {
        wrench_memcpy(new_pixels, old_pixels, self->width * self->height * new_color_channels * new_bytes_per_channel);
        return;
    }

    #define C(SRC_TYPE, SRC_FORMAT, DST_TYPE, DST_FORMAT) (uint32_t)(   \
                                                                        \
        (((uint32_t)(SRC_TYPE)   & 0xFF) << 24) |                       \
        (((uint32_t)(SRC_FORMAT) & 0xFF) << 16) |                       \
        (((uint32_t)(DST_TYPE)   & 0xFF) <<  8) |                       \
        (((uint32_t)(DST_FORMAT) & 0xFF) <<  0) )                       \

    const uint32_t mode = C(old_bytes_per_channel, old_color_channels, new_bytes_per_channel, new_color_channels);

    switch (mode)
    {
        // ===== [ type conversion ] ===========================================

        // float RGBA -> byte RGBA (HDR -> LDR)
        case C(4, 4, 1, 4):
        {
            uint8_t* dst = (uint8_t*)new_pixels;

            float* end = (float*)old_pixels + self->width * self->height * 4;
            float* src = (float*)old_pixels;

            for (; src < end; src += 4, dst += 4)
            {
                float z;

                for (size_t i = 0; i < 3; i++)
                {
                    z = (float)wrench_pow(src[i] * stbi__h2l_scale_i, stbi__h2l_gamma_i) * 255 + 0.5f;
                    if (z < 0.0f) z = 0.0f;
                    if (z > 255.0f) z = 255.0f;
                    dst[i] = (uint8_t)stbi__float2int(z);
                }

                z = src[3] * 255 + 0.5f;
                if (z < 0.0f) z = 0.0f;
                if (z > 255.0f) z = 255.0f;
                dst[3] = (uint8_t)stbi__float2int(z);
            }
        }
        break;

        // byte RGBA -> float RGBA (LDR -> HDR)
        case C(1, 4, 4, 4):
        {
            float* dst = (float*)new_pixels;

            uint8_t* end = (uint8_t*)old_pixels + self->width * self->height * 4;
            uint8_t* src = (uint8_t*)old_pixels;

            for (; src < end; src += 4, dst += 4)
            {
                dst[0] = (float)(wrench_pow(src[0] / 255.0f, stbi__l2h_gamma) * stbi__l2h_scale);
                dst[1] = (float)(wrench_pow(src[1] / 255.0f, stbi__l2h_gamma) * stbi__l2h_scale);
                dst[2] = (float)(wrench_pow(src[2] / 255.0f, stbi__l2h_gamma) * stbi__l2h_scale);

                dst[3] = src[3] / 255.0f;
            }
        }
        break;

        // float RGB -> byte RGB (HDR -> LDR)
        case C(4, 3, 1, 3):
        {
            uint8_t* dst = (uint8_t*)new_pixels;

            float* end = (float*)old_pixels + self->width * self->height * 3;
            float* src = (float*)old_pixels;

            for (; src < end; src += 3, dst += 3)
            {
                for (size_t i = 0; i < 3; i++)
                {
                    float z = (float)wrench_pow(src[i] * stbi__h2l_scale_i, stbi__h2l_gamma_i) * 255 + 0.5f;
                    if (z < 0.0f) z = 0.0f;
                    if (z > 255.0f) z = 255.0f;
                    dst[i] = (uint8_t)stbi__float2int(z);
                }
            }
        }
        break;

        // byte RGB -> float RGB (LDR -> HDR)
        case C(1, 3, 4, 3):
        {
            float* dst = (float*)new_pixels;

            uint8_t* end = (uint8_t*)old_pixels + self->width * self->height * 3;
            uint8_t* src = (uint8_t*)old_pixels;

            for (; src < end; src += 3, dst += 3)
            {
                dst[0] = (float)(wrench_pow(src[0] / 255.0f, stbi__l2h_gamma) * stbi__l2h_scale);
                dst[1] = (float)(wrench_pow(src[1] / 255.0f, stbi__l2h_gamma) * stbi__l2h_scale);
                dst[2] = (float)(wrench_pow(src[2] / 255.0f, stbi__l2h_gamma) * stbi__l2h_scale);
            }
        }
        break;

        // ===== [ format conversion ] =========================================

        // byte RGB -> byte RGBA
        case C(1, 3, 1, 4):
        {
            uint8_t* dst = (uint8_t*)new_pixels;

            uint8_t* end = (uint8_t*)old_pixels + self->width * self->height * 3;
            uint8_t* src = (uint8_t*)old_pixels;

            for (; src < end; src += 3, dst += 4)
            {
                wrench_memcpy(dst, src, sizeof(uint8_t[3]));
                dst[3] = 0xFF;
            }
        }
        break;

        // byte RGBA -> byte RGB
        case C(1, 4, 1, 3):
        {
            uint8_t* dst = (uint8_t*)new_pixels;

            uint8_t* end = (uint8_t*)old_pixels + self->width * self->height * 4;
            uint8_t* src = (uint8_t*)old_pixels;

            for (; src < end; src += 4, dst += 3)
            {
                wrench_memcpy(dst, src, sizeof(uint8_t[3]));
            }
        }
        break;

        // float RGB -> float RGBA
        case C(4, 3, 4, 4):
        {
            float* dst = (float*)new_pixels;

            float* end = (float*)old_pixels + self->width * self->height * 3;
            float* src = (float*)old_pixels;

            for (; src < end; src += 3, dst += 4)
            {
                wrench_memcpy(dst, src, sizeof(float[3]));
                dst[3] = 1.0f;
            }
        }
        break;

        // float RGBA -> float RGB
        case C(4, 4, 4, 3):
        {
            float* dst = (float*)new_pixels;

            float* end = (float*)old_pixels + self->width * self->height * 4;
            float* src = (float*)old_pixels;

            for (; src < end; src += 4, dst += 3)
            {
                wrench_memcpy(dst, src, sizeof(float[3]));
            }
        }
        break;

        // ===== [ dual conversion ] ===========================================

        // byte RGBA -> float RGB
        case C(1, 4, 4, 3):
        {
            float* dst = (float*)new_pixels;

            uint8_t* end = (uint8_t*)old_pixels + self->width * self->height * 4;
            uint8_t* src = (uint8_t*)old_pixels;

            for (; src < end; src += 4, dst += 3)
            {
                dst[0] = (float)(wrench_pow(src[0] / 255.0f, stbi__l2h_gamma) * stbi__l2h_scale);
                dst[1] = (float)(wrench_pow(src[1] / 255.0f, stbi__l2h_gamma) * stbi__l2h_scale);
                dst[2] = (float)(wrench_pow(src[2] / 255.0f, stbi__l2h_gamma) * stbi__l2h_scale);
            }
        }
        break;

        default:
        {
            wrench_assert(0, "TODO: %i %i -> %i %i", old_color_channels, old_bytes_per_channel,
                                                    new_color_channels, new_bytes_per_channel);
        }
        break;
    }

    #undef C
}

static void image_Image_clipIntRect(int* src, int* dst, int w, int h)
{
    if (src == NULL)
    {
        dst[0] = 0;
        dst[1] = 0;
        dst[2] = w;
        dst[3] = h;
    }
    else
    {
        /* Convert to axis-aligned bounding box and clamp, then back.
         */
        const float x_min = wrench_int_clamp(src[0], 0, w);
        const float y_min = wrench_int_clamp(src[1], 0, h);

        const float x_max = wrench_int_clamp(src[0] + src[2], 0, w);
        const float y_max = wrench_int_clamp(src[1] + src[3], 0, h);

        dst[0] = x_min;
        dst[1] = y_min;

        dst[2] = x_max - x_min;
        dst[3] = y_max - y_min;
    }
}

static void image_Image_region(WrenVM* vm)
{
    image_Image* self = (image_Image*)wrenGetSlotForeign(vm, 0);
    WRENCH_CHECK_MAGIC_TAG(self, image, Image);

    const WrenType type = wrenGetSlotType(vm, 1);
    int rect[4];

    switch (type)
    {
        case WREN_TYPE_FOREIGN:
        {
            rect_IntRect* rect_object = (rect_IntRect*)wrenGetSlotForeign(vm, 1);
            WRENCH_CHECK_MAGIC_TAG(rect_object, rect, IntRect);

            if (image_Image_yUp)
            {
                rect_object->xywh[1] = (self->height - rect_object->xywh[3]) - rect_object->xywh[1];
            }

            image_Image_clipIntRect(rect_object->xywh, rect, self->width, self->height);
        }
        break;

        case WREN_TYPE_NULL:
        {
            image_Image_clipIntRect(NULL, rect, self->width, self->height);
        }
        break;

        default:
        {
            wrench_assert(0, "%i", (int)type);
        }
        break;
    }

    wrenGetVariable(vm, "image", "Image", 0);

    image_Image* region = (image_Image*)wrenSetSlotNewForeign(vm, 0, 0, sizeof(image_Image));
    WRENCH_SET_MAGIC_TAG(region, image, Image);

    region->width = rect[2];
    region->height = rect[3];
    region->color_channels = self->color_channels;
    region->bytes_per_channel = self->bytes_per_channel;

    region->pixels = wrench_malloc(region->width * region->height * self->color_channels * self->bytes_per_channel);

    if (region->pixels == NULL)
    {
        char error[1024 * 4];

        wrench_snprintf(error, sizeof(error), "Failed to allocate %ix%ix%ix%i image.",
                                                        region->width, region->height,
                                                        region->color_channels,
                                                        region->bytes_per_channel);

        wrenSetSlotString(vm, 0, (const char*)error);
        wrenAbortFiber(vm, 0);

        return;
    }

    for (int src_y = rect[1], src_end = rect[1] + region->height, dst_y = 0; src_y < src_end; src_y++, dst_y++)
    {
        uint8_t* dst = (uint8_t*)region->pixels + dst_y * region->width * region->color_channels * region->bytes_per_channel;

        uint8_t* src = (uint8_t*)self->pixels + src_y * self->width * self->color_channels * self->bytes_per_channel +
                                                            rect[0] * self->color_channels * self->bytes_per_channel;

        wrench_memcpy(dst, src, region->width * region->color_channels * region->bytes_per_channel);
    }
}

static void image_Image_clipRect(WrenVM* vm)
{
    image_Image* self = (image_Image*)wrenGetSlotForeign(vm, 0);
    WRENCH_CHECK_MAGIC_TAG(self, image, Image);

    wrenGetVariable(vm, "rect", "IntRect", 0);

    rect_IntRect* output = (rect_IntRect*)wrenSetSlotNewForeign(vm, 0, 0, sizeof(rect_IntRect));
    WRENCH_SET_MAGIC_TAG(output, rect, IntRect);

    const WrenType type = wrenGetSlotType(vm, 1);

    switch (type)
    {
        case WREN_TYPE_FOREIGN:
        {
            rect_IntRect* input = (rect_IntRect*)wrenGetSlotForeign(vm, 1);
            WRENCH_CHECK_MAGIC_TAG(input, rect, IntRect);

            image_Image_clipIntRect(input->xywh, output->xywh, self->width, self->height);
        }
        break;

        case WREN_TYPE_NULL:
        {
            image_Image_clipIntRect(NULL, output->xywh, self->width, self->height);
        }
        break;

        default:
        {
            wrench_assert(0, "%i", (int)type);
        }
        break;
    }
}

static void image_Image_getPixelFloatRGB(WrenVM* vm)
{
    image_Image* self = (image_Image*)wrenGetSlotForeign(vm, 0);
    WRENCH_CHECK_MAGIC_TAG(self, image, Image);

    wrench_assert(self->pixels != NULL, "invalid image");
    wrench_assert(self->color_channels == 3, "non-RGB format (%i channels)", self->color_channels);
    wrench_assert(self->bytes_per_channel == sizeof(float), "non-float type (%i bytes)", self->bytes_per_channel);

    const int x = wrenGetSlotInt(vm, 1);
    const int y = wrenGetSlotInt(vm, 2);

    wrench_assert(x >= 0, "%i", x);
    wrench_assert(x < self->width, "%i >= %i", x, self->width);
    wrench_assert(y >= 0, "%i", y);
    wrench_assert(y < self->height, "%i >= %i", y, self->height);

    vector_FltVector* rgba = (vector_FltVector*)wrenGetSlotForeign(vm, 3);
    WRENCH_CHECK_MAGIC_TAG(rgba, vector, FltVector);

    wrench_assert(rgba->dimensions == 3, "non-RGB pixel (%i dimensions)", (int)rgba->dimensions);
    wrench_assert(!image_Image_yUp, "");

    if (rgba->collect)
    {
        wrench_memmove(rgba->elements, (float*)self->pixels + y * self->width * 3 + x * 3, sizeof(float[3]));
    }
    else
    {
        rgba->elements = (float*)self->pixels + y * self->width * 3 + x * 3;
    }
}

static void image_Image_getPixelFloatRGBA(WrenVM* vm)
{
    image_Image* self = (image_Image*)wrenGetSlotForeign(vm, 0);
    WRENCH_CHECK_MAGIC_TAG(self, image, Image);

    wrench_assert(self->pixels != NULL, "invalid image");
    wrench_assert(self->color_channels == 4, "non-RGBA format (%i channels)", self->color_channels);
    wrench_assert(self->bytes_per_channel == sizeof(float), "non-float type (%i bytes)", self->bytes_per_channel);

    const int x = wrenGetSlotInt(vm, 1);
    const int y = wrenGetSlotInt(vm, 2);

    wrench_assert(x >= 0, "%i", x);
    wrench_assert(x < self->width, "%i >= %i", x, self->width);
    wrench_assert(y >= 0, "%i", y);
    wrench_assert(y < self->height, "%i >= %i", y, self->height);

    vector_FltVector* rgba = (vector_FltVector*)wrenGetSlotForeign(vm, 3);
    WRENCH_CHECK_MAGIC_TAG(rgba, vector, FltVector);

    wrench_assert(rgba->dimensions == 4, "non-RGBA pixel (%i dimensions)", (int)rgba->dimensions);
    wrench_assert(!image_Image_yUp, "");

    if (rgba->collect)
    {
        wrench_memmove(rgba->elements, (float*)self->pixels + y * self->width * 4 + x * 4, sizeof(float[4]));
    }
    else
    {
        rgba->elements = (float*)self->pixels + y * self->width * 4 + x * 4;
    }
}

static void image_Image_pixelIsBlack(WrenVM* vm)
{
    image_Image* self = (image_Image*)wrenGetSlotForeign(vm, 0);
    WRENCH_CHECK_MAGIC_TAG(self, image, Image);

    wrench_assert(self->pixels != NULL, "invalid image");
    wrench_assert(self->color_channels == 3, "TODO");
    wrench_assert(self->bytes_per_channel == sizeof(float), "TODO");

    const int x = wrenGetSlotInt(vm, 1);
    const int y = wrenGetSlotInt(vm, 2);

    wrench_assert(x >= 0, "%i", x);
    wrench_assert(x < self->width, "%i >= %i", x, self->width);
    wrench_assert(y >= 0, "%i", y);
    wrench_assert(y < self->height, "%i >= %i", y, self->height);

    const float* const pixel = (const float* const)self->pixels + y * self->width * 3 + x * 3;

    if (wrench_fabsf(pixel[0]) > 0.00001f)
    {
        wrenSetSlotBool(vm, 0, false);
        return;
    }

    if (wrench_fabsf(pixel[1]) > 0.00001f)
    {
        wrenSetSlotBool(vm, 0, false);
        return;
    }

    if (wrench_fabsf(pixel[2]) > 0.00001f)
    {
        wrenSetSlotBool(vm, 0, false);
        return;
    }

    wrenSetSlotBool(vm, 0, true);
}

static void image_Image_pixelIsWhite(WrenVM* vm)
{
    image_Image* self = (image_Image*)wrenGetSlotForeign(vm, 0);
    WRENCH_CHECK_MAGIC_TAG(self, image, Image);

    wrench_assert(self->pixels != NULL, "invalid image");
    wrench_assert(self->color_channels == 3, "TODO");
    wrench_assert(self->bytes_per_channel == sizeof(float), "TODO");

    const int x = wrenGetSlotInt(vm, 1);
    const int y = wrenGetSlotInt(vm, 2);

    wrench_assert(x >= 0, "%i", x);
    wrench_assert(x < self->width, "%i >= %i", x, self->width);
    wrench_assert(y >= 0, "%i", y);
    wrench_assert(y < self->height, "%i >= %i", y, self->height);

    const float* const pixel = (const float* const)self->pixels + y * self->width * 3 + x * 3;

    if (wrench_fabsf(pixel[0] - 1.0f) > 0.00001f)
    {
        wrenSetSlotBool(vm, 0, false);
        return;
    }

    if (wrench_fabsf(pixel[1] - 1.0f) > 0.00001f)
    {
        wrenSetSlotBool(vm, 0, false);
        return;
    }

    if (wrench_fabsf(pixel[2] - 1.0f) > 0.00001f)
    {
        wrenSetSlotBool(vm, 0, false);
        return;
    }

    wrenSetSlotBool(vm, 0, true);
}

/*
================================================================================
 * ~~ [ (un)hook ] ~~ *
--------------------------------------------------------------------------------
*/

#if WRENCH_IMAGE_EXTENDED
    /*
     * Enable user extension of stdlib modules.
     */
    #ifndef __IMAGE_EX_INL__
    #include <image_ex.inl>
    #endif
#else
    static bool imageWrenInitEx(WrenVM* vm)
    {
        return true;
    }

    static void imageWrenQuitEx(void)
    {
        //
    }

    static bool imageImageWrenInitEx(WrenVM* vm)
    {
        return true;
    }
#endif /* WRENCH_IMAGE_EXTENDED */

WRENCH_EXPORT bool imageWrenInit(WrenVM* vm)
{
    if (!wrenBeginModule(vm, "image")) { return false; } else
    {
        WREN_BEGIN_CLASS(image, Image);
        {
            WREN_CODE("construct new(width, height, colorChannels, bytesPerChannel) {}");

            WREN_METHOD(image, Image, true, load, "(filename, desiredColorChannels, desiredBytesPerChannel)", "(_,_,_)");
            WREN_CODE("static load(filename) { load(filename, 0, 0) }");

            WREN_METHOD(image, Image, true, loadFromBytes, "(data, desiredColorChannels, desiredBytesPerChannel)", "(_,_,_)");
            WREN_CODE("static loadFromBytes(data) { loadFromBytes(data, 0, 0) }");

            // TODO: info
            // TODO: infoFromBytes

            WREN_METHOD(image, Image, false, save, "(path)", "(_)");

            // TODO: saveToBytes

            // TODO: name
            // TODO: path

            // TODO: toString

            WREN_PROPERTY(image, Image, true, yUp);
            WREN_CODE("static yDown { !yUp }");
            WREN_CODE("static yDown=(value) { yUp = !value }");

            WREN_INDEX_PROPERTY(image, Image, false, 2);

            WREN_CODE("static MONO { 1 }");
            WREN_CODE("static RGB { 3 }");
            WREN_CODE("static RGBA { 4 }");

            WREN_CODE("static BYTE { 1 }");
            WREN_CODE("static SHORT { 2 }");
            WREN_CODE("static FLOAT { 4 }");

            // TODO: data

            WREN_GETTER(image, Image, false, width);
            WREN_GETTER(image, Image, false, height);
            WREN_GETTER(image, Image, false, colorChannels);
            WREN_GETTER(image, Image, false, bytesPerChannel);

            WREN_CODE("bytesPerPixel { colorChannels * bytesPerChannel }");

            WREN_CODE("isMono { colorChannels == 1 }");
            WREN_CODE("isRGB { colorChannels == 3 }");
            WREN_CODE("isRGBA { colorChannels == 4 }");

            WREN_CODE("isBytes { bytesPerChannel == 1 }");
            WREN_CODE("isShorts { bytesPerChannel == 2 }");
            WREN_CODE("isFloats { bytesPerChannel == 4 }");

            WREN_CODE("bytes { width * height * colorChannels * bytesPerChannel }");
            WREN_CODE("pitch { width * colorChannels * bytesPerChannel }");

            WREN_METHOD(image, Image, false, resize, "(width, height, filter)", "(_,_,_)");
            WREN_METHOD(image, Image, false, convert, "(colorChannels, bytesPerChannel)", "(_,_)");

            WREN_CODE("formatConvert(colorChannels) { convert(colorChannels, bytesPerChannel) }");
            WREN_CODE("typeConvert(bytesPerChannel) { convert(colorChannels, bytesPerChannel) }");

            WREN_CODE("copy { convert(colorChannels, bytesPerChannel) }");
            WREN_METHOD(image, Image, false, region, "(rect)", "(_)");

            WREN_METHOD(image, Image, false, clipRect, "(rect)", "(_)");

            /* Fast paths for this[x, y] that avoid creation of a temporary FltVector object.
             */
            WREN_METHOD(image, Image, false, getPixelFloatRGB, "(x, y, pixel)", "(_,_,_)");
            WREN_METHOD(image, Image, false, getPixelFloatRGBA, "(x, y, pixel)", "(_,_,_)");

            WREN_METHOD(image, Image, false, pixelIsBlack, "(x, y)", "(_,_)");
            WREN_METHOD(image, Image, false, pixelIsWhite, "(x, y)", "(_,_)");

            if (!imageImageWrenInitEx(vm))
            {
                return false;
            }
        }
        WREN_END_CLASS();
    }

    if (!imageWrenInitEx(vm))
    {
        return false;
    }

    return wrenEndModule(vm);
}

WRENCH_EXPORT void imageWrenQuit(void)
{
    imageWrenQuitEx();
}
