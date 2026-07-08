/* -----------------------------------------------------------------------------
--- Copyright (c) 2012-2026 Adam Schackart / "AJ Hackman", all rights reserved.
--- Distributed under the BSD license v2 (opensource.org/licenses/BSD-3-Clause)
----------------------------------------------------------------------------- */
#ifndef __WRENCH_GASKET_H__
#define __WRENCH_GASKET_H__

/*
================================================================================
 * ~~ [ types ] ~~ *
--------------------------------------------------------------------------------
*/

struct wrench_preprocessor_t; // Opaque preprocessor struct.
typedef struct wrench_preprocessor_t* wrench_preprocessor_p;

struct gasket_context_t; // Opaque context struct.
typedef struct gasket_context_t* gasket_context_p;

/*
================================================================================
 * ~~ [ macros ] ~~ *
--------------------------------------------------------------------------------
*/

/* Function declaration.
 */
#ifndef GASKET_DECL
#define GASKET_DECL(ret, name, args) ret gasket_ ## name args
#endif

/* Function definition.
 */
#ifndef GASKET_IMPL
#define GASKET_IMPL(ret, name, args) ret gasket_ ## name args
#endif

/* Helper for foreign library init and quit functions.
 */
#if !defined(WRENCH_EXPORT)
    #if defined(__cplusplus)
        #if _MSC_VER
            #define WRENCH_EXPORT __declspec(dllexport) extern "C"
        #else
            #define WRENCH_EXPORT extern "C"
        #endif
    #else
        #if _MSC_VER
            #define WRENCH_EXPORT __declspec(dllexport)
        #else
            #define WRENCH_EXPORT extern
        #endif
    #endif
#endif /* !WRENCH_EXPORT */

/* Enables safety checks at a performance cost.
 */
#if !defined(WRENCH_DEBUG)
    #if defined(_DEBUG) || defined(DEBUG) || (defined(__GNUC__) && !defined(__OPTIMIZE__)) || !NDEBUG
        #define WRENCH_DEBUG 1
    #else
        #define WRENCH_DEBUG 0
    #endif
#endif /* !WRENCH_DEBUG */

/*
================================================================================
 * ~~ [ public API ] ~~ *
--------------------------------------------------------------------------------
*/

GASKET_DECL(gasket_context_p, context_create, (void));

/* Returns NULL on success, error string on failure.
 */
GASKET_DECL(const char*, context_init, (gasket_context_p context));
GASKET_DECL(void, context_free, (gasket_context_p context));

GASKET_DECL(void, set_error_string, (gasket_context_p context, const char* error));
GASKET_DECL(const char*, get_error_string, (gasket_context_p context));

GASKET_DECL(void, set_preprocessor, (gasket_context_p context, wrench_preprocessor_p preprocessor));
GASKET_DECL(wrench_preprocessor_p, get_preprocessor, (gasket_context_p context));

// TODO: (s/g)et marker_start
// TODO: (s/g)et marker_code_end
// TODO: (s/g)et marker_end

GASKET_DECL(const char*, process_ex, (gasket_context_p ctx, const char* source));
GASKET_DECL(const char*, process, (const char* source));

#endif /* __WRENCH_GASKET_H__ */

/*
================================================================================
 * ~~ [ implementation ] ~~ *
--------------------------------------------------------------------------------
*/

#ifdef WRENCH_GASKET_IMPLEMENTATION
/*
 * Enable multiple file inclusions with `WRENCH_GASKET_IMPLEMENTATION` for ease of use.
 */
#ifndef __WRENCH_GASKET_C__
#define __WRENCH_GASKET_C__

#ifndef __WRENCH_H__
#include <wrench.h>
#endif

#ifndef __WRENCH_PREPROCESSOR_H__
#include <wrench_preprocessor.h>
#endif

/* ===== [ standard library ] =============================================== */

/* Disable MSVC warnings about fopen() etc.
 */
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS 1
#endif

#if !WRENCH_NO_CSTDLIB
    #include <string.h>
#endif

#if !_WIN32 && !WRENCH_NO_POSIX_HEADERS
    #include <signal.h>
#endif

#ifndef wrench_abort
#define wrench_abort abort
#endif
#ifndef wrench_fprintf
#define wrench_fprintf fprintf
#endif
#ifndef wrench_fputs
#define wrench_fputs fputs
#endif
#ifndef wrench_free
#define wrench_free(x) free((void*)(x))
#endif
#ifndef wrench_malloc
#define wrench_malloc malloc
#endif
#ifndef wrench_memcpy
#define wrench_memcpy memcpy
#endif
#ifndef wrench_memset
#define wrench_memset memset
#endif
#ifndef wrench_realloc
#define wrench_realloc realloc
#endif
#ifndef wrench_snprintf
#define wrench_snprintf snprintf
#endif
/*
 * XXX TODO FIXME: We #undef std(in/err/out) in wrench.h - do the same thing here.
 */
#ifndef wrench_stderr
#define wrench_stderr stderr
#endif
#ifndef wrench_stdout
#define wrench_stdout stdout
#endif
#ifndef wrench_strlen
#define wrench_strlen strlen
#endif
#ifndef wrench_strstr
#define wrench_strstr strstr
#endif
#ifndef wrench_va_end
#define wrench_va_end va_end
#endif
#ifndef wrench_va_start
#define wrench_va_start va_start
#endif
#ifndef wrench_vsnprintf
#define wrench_vsnprintf vsnprintf
#endif

/* ===== [ structures ] ===================================================== */

typedef struct gasket_string_builder_t
{
    char* data;
    size_t size;
    size_t capacity;
}
gasket_string_builder_t;

typedef struct gasket_context_t
{
    WrenVM* vm;
    wrench_preprocessor_p preprocessor;
    gasket_string_builder_t capture_buffer;
    bool is_capturing;
    bool allocated;
    char marker_start[16];
    char marker_code_end[16];
    char marker_end[16];
    char error_string[1024];
}
gasket_context_t;

/* ===== [ utilities ] ====================================================== */

/* Indicates that a function is not implemented.
 */
#if !defined(WRENCH_STUB)
    #if 1
        #define WRENCH_STUB() wrench_fprintf(wrench_stderr, "TODO %s (file \"%s\", line %i)\n", __FUNCTION__, __FILE__, __LINE__)
    #else
        #define WRENCH_STUB() wrench_assert(0, "TODO")
    #endif
#endif /* !WRENCH_STUB */

/* Optional program entry point.
 */
//#ifndef GASKET_MAIN
//#define GASKET_MAIN main
//#endif

#if !defined(WRENCH_STRINGIFY)
    #define WRENCH_STRINGIFY(s) _WRENCH_STR_IMPL(s)
    #define _WRENCH_STR_IMPL(s) #s
#endif

#if !defined(wrench_breakpoint)
    #if _WIN32
        extern void __cdecl __debugbreak(void);

        #ifndef wrench_breakpoint
        #define wrench_breakpoint() __debugbreak()
        #endif
    #else
        #ifndef wrench_breakpoint
        #define wrench_breakpoint() raise(SIGTRAP)
        #endif
    #endif
#endif /* !wrench_breakpoint */

#if !defined(wrench_assert)
    #if WRENCH_DEBUG
        #define wrench_assert(cnd, ...) if ((cnd) == 0)                                                     \
        {                                                                                                   \
            wrench_fprintf(wrench_stderr, "assert \"%s\" failed in func \"%s\" (file \"%s\", line %i): ",   \
                                                WRENCH_STRINGIFY(cnd), __FUNCTION__, __FILE__, __LINE__);   \
                                                                                                            \
            wrench_fprintf(wrench_stderr, __VA_ARGS__);                                                     \
            wrench_fprintf(wrench_stderr, "\n");                                                            \
                                                                                                            \
            wrench_breakpoint();                                                                            \
        }
    #else
        #define wrench_assert(cnd, ...) ((void)sizeof(cnd))
    #endif
#endif /* !wrench_assert */

#if defined(__GNUC__) || defined(__clang__)
    #define WRENCH_PRINTF_ATTR(fmt, args) __attribute__((format(printf, fmt, args)))
#else
    #define WRENCH_PRINTF_ATTR(fmt, args)
#endif

static void gasket_set_error_string_ex(gasket_context_p context, const char* format, ...) WRENCH_PRINTF_ATTR(2, 3);
static void gasket_set_error_string_ex(gasket_context_p context, const char* format, ...)
{
    wrench_assert(context != NULL, "");
    va_list args;

    wrench_va_start(args, format);
    {
        wrench_vsnprintf(context->error_string, sizeof(context->error_string), format, args);
    }
    wrench_va_end(args);
}

static bool gasket_string_builder_init(gasket_string_builder_t* b)
{
    b->capacity = 64;
    b->size = 0;
    b->data = (char*)wrench_malloc(b->capacity);

    if (b->data == NULL)
    {
        return false;
    }

    b->data[0] = '\0';
    return true;
}

static bool gasket_string_builder_append(gasket_string_builder_t* b, const char* text, size_t len)
{
    while (b->size + len + 1 > b->capacity)
    {
        if (1)
        {
            b->capacity = (b->capacity * 3) / 2;
        }
        else
        {
            b->capacity = b->capacity * 2;
        }
    }

    char* new_data = (char*)wrench_realloc(b->data, b->capacity);

    if (new_data == NULL)
    {
        wrench_free(b->data);
        wrench_memset(b, 0, sizeof(*b));

        return false;
    }

    b->data = new_data;

    wrench_memcpy(b->data + b->size, text, len);
    b->size += len;
    b->data[b->size] = '\0';

    return true;
}

static void gasket_string_builder_free(gasket_string_builder_t* b)
{
    wrench_assert(b != NULL, "");

    wrench_free(b->data);
    wrench_memset(b, 0, sizeof(*b));
}

/* TODO: Remove this and write to an error string in the context that could be retrieved later.
 */
#ifndef wrench_error
#define wrench_error(...) do                    \
{                                               \
    wrench_fprintf(wrench_stderr, __VA_ARGS__); \
    wrench_abort();                             \
}                                               \
while (0)

#endif /* wrench_error */

#ifndef GASKET_EXTENDED_VM
#define GASKET_EXTENDED_VM 0
#endif
#ifndef GASKET_CALL_GLOBAL_HOOKS
#define GASKET_CALL_GLOBAL_HOOKS 0
#endif
#ifndef GASKET_DEFAULT_PREPROCESSOR
#define GASKET_DEFAULT_PREPROCESSOR 0
#endif

/* Wren write callback, extracting the context via user data.
 */
static void gasket_vm_write(WrenVM* vm, const char* text)
{
    gasket_context_p context;

    if (GASKET_EXTENDED_VM)
    {
        context = (gasket_context_p)wrenGetUserDataEx(vm, 0);
    }
    else
    {
        context = (gasket_context_p)wrenGetUserData(vm);
    }

    if (context != NULL && context->is_capturing)
    {
        if (!gasket_string_builder_append(&context->capture_buffer, text, wrench_strlen(text)))
        {
            wrench_error("Out of memory - gasket preprocessor failed!");
        }
    }
    else
    {
        wrench_fprintf(wrench_stdout, "%s", text);
    }
}

/* ===== [ public API ] ===================================================== */

GASKET_IMPL(gasket_context_p, context_create, (void))
{
    gasket_context_p context = (gasket_context_p)wrench_malloc(sizeof(gasket_context_t));

    if (context == NULL)
    {
        return NULL;
    }

    const char* init_error = gasket_context_init(context);

    if (init_error != NULL)
    {
        wrench_free(context);

        wrench_error("%s", init_error);
        return NULL;
    }

    context->allocated = true;
    return context;
}

GASKET_IMPL(const char*, context_init, (gasket_context_p context))
{
    wrench_assert(context != NULL, "");
    wrench_memset(context, 0, sizeof(gasket_context_t));

    #ifndef GASKET_MARKER_START
    #define GASKET_MARKER_START "/*[[[wren"
    #endif
    wrench_snprintf(context->marker_start, sizeof(context->marker_start), "%s", GASKET_MARKER_START);

    #ifndef GASKET_MARKER_CODE_END
    #define GASKET_MARKER_CODE_END "]]]*/"
    #endif
    wrench_snprintf(context->marker_code_end, sizeof(context->marker_code_end), "%s", GASKET_MARKER_CODE_END);

    #ifndef GASKET_MARKER_END
    #define GASKET_MARKER_END "/*[[[end]]]*/"
    #endif
    wrench_snprintf(context->marker_end, sizeof(context->marker_end), "%s", GASKET_MARKER_END);

    if (GASKET_DEFAULT_PREPROCESSOR)
    {
        wrench_assert(0, "TODO");
    }

    if (GASKET_EXTENDED_VM)
    {
        WrenConfiguration* config = wrenGetConfig();
        WrenWriteFn config_write_func = config->writeFn;
        config->writeFn = gasket_vm_write;

        context->vm = wrenNewExtendedVM(0, NULL, GASKET_CALL_GLOBAL_HOOKS);
        config->writeFn = config_write_func;

        wrench_assert(wrenGetUserDataEx(context->vm, 0) == NULL, "");
        wrenSetUserDataEx(context->vm, 0, context);
    }
    else
    {
        WrenConfiguration config;
        wrenInitConfiguration(&config);

        config.writeFn = gasket_vm_write;
        context->vm = wrenNewVM(&config);

        wrench_assert(wrenGetUserData(context->vm) == NULL, "");
        wrenSetUserData(context->vm, context);
    }

    return NULL;
}

GASKET_IMPL(void, context_free, (gasket_context_p context))
{
    wrench_assert(context != NULL, "");

    if (GASKET_DEFAULT_PREPROCESSOR)
    {
        wrench_assert(0, "TODO");
    }

    if (GASKET_EXTENDED_VM)
    {
        wrenFreeExtendedVM(context->vm, GASKET_CALL_GLOBAL_HOOKS);
    }
    else
    {
        wrenFreeVM(context->vm);
    }

    if (context->allocated)
    {
        wrench_free(context);
    }
}

GASKET_IMPL(void, set_error_string, (gasket_context_p context, const char* error))
{
    wrench_assert(context != NULL, "");
    wrench_snprintf(context->error_string, sizeof(context->error_string), "%s", error);
}

GASKET_IMPL(const char*, get_error_string, (gasket_context_p context))
{
    wrench_assert(context != NULL, "");
    return (const char*)context->error_string;
}

GASKET_IMPL(void, set_preprocessor, (gasket_context_p context, wrench_preprocessor_p preprocessor))
{
    wrench_assert(context != NULL, "");
    context->preprocessor = preprocessor;
}

GASKET_IMPL(wrench_preprocessor_p, get_preprocessor, (gasket_context_p context))
{
    wrench_assert(context != NULL, "");
    return context->preprocessor;
}

/* TODO: Make this func recursive (so that we could have gasket directives embedded inside gasket directives).
 */
GASKET_IMPL(const char*, process_ex, (gasket_context_p ctx, const char* source))
{
    wrench_assert(ctx != NULL, "");
    wrench_assert(ctx->vm != NULL, "");

    gasket_string_builder_t output;
    ctx->is_capturing = false;

    if (!gasket_string_builder_init(&output))
    {
        gasket_set_error_string(ctx, "Out of memory - gasket preprocessor failed to initialize output buffer.");
        return NULL;
    }

    if (!gasket_string_builder_init(&ctx->capture_buffer))
    {
        gasket_string_builder_free(&output);

        gasket_set_error_string(ctx, "Out of memory - gasket preprocessor failed to initialize capture buffer.");
        return NULL;
    }

    const char* cursor = source;

    while (*cursor != '\0')
    {
        const char* start_ptr = wrench_strstr(cursor, ctx->marker_start);

        if (start_ptr == NULL)
        {
            if (!gasket_string_builder_append(&output, cursor, wrench_strlen(cursor)))
            {
                gasket_set_error_string(ctx, "Out of memory - gasket preprocessor failed text append.");
                goto error_cleanup;
            }

            break;
        }

        /* Copy up to the start marker.
         */
        if (!gasket_string_builder_append(&output, cursor, start_ptr - cursor))
        {
            gasket_set_error_string(ctx, "Out of memory - gasket preprocessor failed text append.");
            goto error_cleanup;
        }

        /* Copy the start marker itself.
         */
        if (!gasket_string_builder_append(&output, ctx->marker_start, wrench_strlen(ctx->marker_start)))
        {
            gasket_set_error_string(ctx, "Out of memory - gasket preprocessor failed text append.");
            goto error_cleanup;
        }

        cursor = start_ptr + wrench_strlen(ctx->marker_start);

        // Search through to find the end of the script block.
        const char* code_end_ptr = wrench_strstr(cursor, ctx->marker_code_end);

        if (code_end_ptr == NULL)
        {
            gasket_set_error_string_ex(ctx, "Missing closing code marker `%s`", ctx->marker_code_end);
            goto error_cleanup;
        }

        size_t script_len = code_end_ptr - cursor;
        char* script = (char*)wrench_malloc(script_len + 1);

        if (script == NULL)
        {
            gasket_set_error_string(ctx, "Out of memory - gasket preprocessor failed copy script.");
            goto error_cleanup;
        }

        wrench_memcpy(script, cursor, script_len);
        script[script_len] = '\0';

        /* Copy script and code end marker to output.
         */
        if (!gasket_string_builder_append(&output, cursor, script_len))
        {
            wrench_free(script);

            gasket_set_error_string(ctx, "Out of memory - gasket preprocessor failed text append.");
            goto error_cleanup;
        }

        if (!gasket_string_builder_append(&output, ctx->marker_code_end, wrench_strlen(ctx->marker_code_end)))
        {
            wrench_free(script);

            gasket_set_error_string(ctx, "Out of memory - gasket preprocessor failed text append.");
            goto error_cleanup;
        }

        if (!gasket_string_builder_append(&output, "\n", 1))
        {
            wrench_free(script);

            gasket_set_error_string(ctx, "Out of memory - gasket preprocessor failed text append.");
            goto error_cleanup;
        }

        cursor = code_end_ptr + wrench_strlen(ctx->marker_code_end);

        if (ctx->preprocessor != NULL)
        {
            const char* processed_script = wrench_preprocessor_process_string(ctx->preprocessor, "<gasket_script>", script);
            wrench_free(script);

            if (processed_script == NULL)
            {
                gasket_set_error_string_ex(ctx, "Preprocessor error: %s", wrench_preprocessor_get_error_string(ctx->preprocessor));
                goto error_cleanup;
            }

            script = (char*)processed_script;
        }

        // Execute script with capturing enabled.
        ctx->is_capturing = true;
        ctx->capture_buffer.size = 0;
        ctx->capture_buffer.data[0] = '\0';

        WrenInterpretResult result = wrenInterpret(ctx->vm, "gasket_module", script);
        wrench_free(script);

        ctx->is_capturing = false;

        switch (result)
        {
            case WREN_RESULT_SUCCESS: break;

            case WREN_RESULT_COMPILE_ERROR:
            case WREN_RESULT_RUNTIME_ERROR:
            {
                if (GASKET_EXTENDED_VM)
                {
                    gasket_set_error_string(ctx, wrenGetErrorString(ctx->vm));
                }
                else
                {
                    // TODO: Set error handling callback that captures the entire traceback string.
                    gasket_set_error_string(ctx, "Wren execution failed in gasket preprocessor!");
                }

                goto error_cleanup;
            }
            break;

            default:
            {
                wrench_assert(0, "%i", (int)result);
            }
            break;
        }

        /* Append the generated text.
         */
        if (!gasket_string_builder_append(&output, ctx->capture_buffer.data, ctx->capture_buffer.size))
        {
            gasket_set_error_string(ctx, "Out of memory - gasket preprocessor failed text append.");
            goto error_cleanup;
        }

        // Find and append the final block end marker, skipping old code.
        const char* end_ptr = wrench_strstr(cursor, ctx->marker_end);

        if (end_ptr == NULL)
        {
            gasket_set_error_string_ex(ctx, "Missing block end marker '%s'", ctx->marker_end);
            goto error_cleanup;
        }

        if (!gasket_string_builder_append(&output, ctx->marker_end, wrench_strlen(ctx->marker_end)))
        {
            gasket_set_error_string(ctx, "Out of memory - gasket preprocessor failed text append.");
            goto error_cleanup;
        }

        cursor = end_ptr + wrench_strlen(ctx->marker_end);
    }

    gasket_string_builder_free(&ctx->capture_buffer);
    return output.data;

    error_cleanup:
    {
        gasket_string_builder_free(&ctx->capture_buffer);
        gasket_string_builder_free(&output);

        return NULL;
    }
}

GASKET_IMPL(const char*, process, (const char* source))
{
    gasket_context_t ctx;
    const char* init_error = gasket_context_init(&ctx);

    if (init_error != NULL)
    {
        wrench_error("%s", init_error);
        return NULL;
    }

    const char* r = gasket_process_ex(&ctx, source);
    gasket_context_free(&ctx);

    if (r == NULL)
    {
        wrench_error("%s", gasket_get_error_string(&ctx));
    }

    return r;
}

#if defined(GASKET_MAIN)

static char* gasket_read_entire_file(const char* filename)
{
    FILE* file = wrench_fopen(filename, "rb");

    if (file == NULL)
    {
        wrench_error("Could not open \"%s\" for reading.", filename);
        return NULL;
    }

    if (wrench_fseek(file, 0, SEEK_END) != 0)
    {
        wrench_error("Failed to seek to end of file \"%s\".", filename);

        wrench_fclose(file);
        return NULL;
    }

    const long length = wrench_ftell(file);

    if (length < 0)
    {
        wrench_error("Failed to determine file size for \"%s\".", filename);

        wrench_fclose(file);
        return NULL;
    }

    if (wrench_fseek(file, 0, SEEK_SET) != 0)
    {
        wrench_error("Failed to seek to beginning of file \"%s\".", filename);

        wrench_fclose(file);
        return NULL;
    }

    char* buffer = (char*)wrench_malloc(length + 1);

    if (buffer == NULL)
    {
        wrench_error("Out of memory! Failed to allocate \"%s\".", filename);

        wrench_fclose(file);
        return NULL;
    }

    const size_t read_len = wrench_fread(buffer, 1, length, file);

    if (read_len != (size_t)length)
    {
        wrench_error("Failed to read entire file \"%s\". Expected %ld bytes, got %zu.", filename, length, read_len);

        wrench_free(buffer);
        wrench_fclose(file);

        return NULL;
    }

    buffer[read_len] = '\0';

    if (wrench_fclose(file) != 0)
    {
        wrench_error("Failed to close file \"%s\" cleanly.", filename);
    }

    return buffer;
}

int GASKET_MAIN(int argc, char** argv)
{
    /* TODO: Allow for #defines and #include dirs to be passed via the
     * command-line, as well as an option to disable the preprocessor.
     */
    if (argc < 2)
    {
        wrench_fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char* filename = argv[1];
    char* source_code = gasket_read_entire_file(filename);

    if (!source_code)
    {
        return EXIT_FAILURE;
    }

    gasket_context_t gasket_context;
    const char* init_err = gasket_context_init(&gasket_context);

    if (init_err != NULL)
    {
        wrench_error("Error initializing gasket: %s", init_err);
        wrench_free(source_code);

        return EXIT_FAILURE;
    }

    wrench_preprocessor_p preprocessor = wrench_preprocessor_create(".");

    if (preprocessor == NULL)
    {
        wrench_error("Failed to create preprocessor context.");

        gasket_context_free(&gasket_context);
        wrench_free(source_code);

        return EXIT_FAILURE;
    }

    gasket_set_preprocessor(&gasket_context, preprocessor);

    const char* output = gasket_process_ex(&gasket_context, source_code);

    if (!output)
    {
        wrench_error("%s", gasket_get_error_string(&gasket_context));

        wrench_preprocessor_destroy(preprocessor);
        gasket_context_free(&gasket_context);
        wrench_free(source_code);

        return EXIT_FAILURE;
    }

    FILE* out_file = wrench_fopen(filename, "wb");

    if (out_file == NULL)
    {
        wrench_error("Could not open file %s for writing.", filename);

        wrench_preprocessor_destroy(preprocessor);
        gasket_context_free(&gasket_context);
        wrench_free(source_code);

        return EXIT_FAILURE;
    }

    // TODO: Error checking.
    wrench_fputs(output, out_file);

    // TODO: Error checking.
    wrench_fclose(out_file);

    wrench_fprintf(wrench_stdout, "Successfully processed and overwrote %s\n", filename);

    wrench_preprocessor_destroy(preprocessor);
    gasket_context_free(&gasket_context);
    wrench_free(source_code);
    wrench_free(output);

    return EXIT_SUCCESS;
}

#endif /* GASKET_MAIN */
#endif /* __WRENCH_GASKET_C__ */
#endif /* WRENCH_GASKET_IMPLEMENTATION */
