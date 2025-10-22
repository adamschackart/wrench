/* -----------------------------------------------------------------------------
--- Copyright (c) 2012-2026 Adam Schackart / "AJ Hackman", all rights reserved.
--- Distributed under the BSD license v2 (opensource.org/licenses/BSD-3-Clause)
----------------------------------------------------------------------------- */
#ifndef __WRENCH_H__
#define __WRENCH_H__

#include <wren.h>

/*
================================================================================
 * ~~ [ macros & types ] ~~ *
--------------------------------------------------------------------------------
*/

/* Function declaration.
 */
#ifndef WRENCH_DECL
#define WRENCH_DECL(ret, name, args) WREN_API ret wren ## name args
#endif

/* Function definition.
 */
#ifndef WRENCH_IMPL
#define WRENCH_IMPL(ret, name, args) ret wren ## name args
#endif

/* Helper for foreign libraries.
 */
#if !defined(WRENCH_EXPORT)
    #if _MSC_VER
        #define WRENCH_EXPORT __declspec(dllexport)
    #else
        #define WRENCH_EXPORT extern
    #endif
#endif /* !WRENCH_EXPORT */

typedef void* (*wrenFileReadFn)(WrenVM* vm, const char* name, size_t* size);
typedef void (*wrenFileFreeFn)(WrenVM* vm, void* data, size_t size);

typedef bool (*wrenLibraryInitFn)(WrenVM* vm);
typedef void (*wrenLibraryQuitFn)(void);

// Avoid including <stdint.h>.
typedef unsigned char uint8_t;

/* Win32 definitions.
 */
#undef GetCommandLine
#undef RegisterClass

/*
================================================================================
 * ~~ [ public API ] ~~ *
--------------------------------------------------------------------------------
*/

/* Create and destroy a Wren virtual machine with our config and internal data.
 * Note that creation of new VMs is not threadsafe (due to the global config).
 */
WRENCH_DECL(WrenConfiguration*, GetConfig, (void));

WRENCH_DECL(WrenVM*, NewExtendedVM, (int argc, char** argv, bool call_global_init_funcs));
WRENCH_DECL(void, FreeExtendedVM, (WrenVM* vm, bool call_global_quit_funcs));

/* Functions that are called on the creation and destructions of each VM.
 */
WRENCH_DECL(void, RegisterGlobalInitFunction, (wrenLibraryInitFn init));
WRENCH_DECL(void, RegisterGlobalQuitFunction, (wrenLibraryQuitFn quit));

/* Enabled by default - may be disabled for security hardening purposes.
 */
WRENCH_DECL(bool, GetForeignLibraryLoadEnabled, (WrenVM* vm));
WRENCH_DECL(void, SetForeignLibraryLoadEnabled, (WrenVM* vm, bool enabled));

/* Human-readable error messages.
 */
WRENCH_DECL(const char*, GetErrorString, (WrenVM* vm));
WRENCH_DECL(void, SetErrorString, (WrenVM* vm, const char* error));

/* Wrench sets the VM userdata pointer for internal state, so use this instead.
 * Users and libs must decide for themselves how slots are allocated/reserved.
 */
WRENCH_DECL(void*, GetUserDataEx, (WrenVM* vm, int slot));
WRENCH_DECL(void, SetUserDataEx, (WrenVM* vm, int slot, void* userData));

/* Get the arguments passed to VM init. `argv` is terminated by a NULL pointer.
 */
WRENCH_DECL(char**, GetCommandLine, (WrenVM* vm, int* argc));
WRENCH_DECL(bool, SetCommandLine, (WrenVM* vm, int argc, char** argv));

/* Get the source code of a module. If module doesn't exist, NULL is returned.
 */
WRENCH_DECL(const char*, GetModuleSource, (WrenVM* vm, const char* name));

/* Main EXE path (prefixed onto file paths passed into `wrenLoadSourceFile`).
 */
WRENCH_DECL(const char*, GetBasePath, (WrenVM* vm));
WRENCH_DECL(bool, SetBasePath, (WrenVM* vm, const char* path));

/* The optional functions we use to implement wrenLoadSourceFile. Can be overridden
 * to do things like read source code from zipfiles, check global tables, etc. etc.
 */
WRENCH_DECL(wrenFileReadFn, GetFileReadCallback, (WrenVM* vm));
WRENCH_DECL(void, SetFileReadCallback, (WrenVM* vm, wrenFileReadFn callback));

WRENCH_DECL(wrenFileFreeFn, GetFileFreeCallback, (WrenVM* vm));
WRENCH_DECL(void, SetFileFreeCallback, (WrenVM* vm, wrenFileFreeFn callback));

WRENCH_DECL(void*, DefaultFileRead, (WrenVM* vm, const char* name, size_t* size));
WRENCH_DECL(void, DefaultFileFree, (WrenVM* vm, void* data, size_t size));

/* Load code from disk. Name is prefixed by base path & suffixed with ".wren".
 */
WRENCH_DECL(const char*, LoadSourceFile, (WrenVM* vm, const char* name, size_t* num_chars));

/* Build a Wren module incrementally.
 */
WRENCH_DECL(bool, BeginModule, (WrenVM* vm, const char* moduleName));
WRENCH_DECL(bool, CodeEx, (WrenVM* vm, const char* source, size_t num_chars));
WRENCH_DECL(bool, Code, (WrenVM* vm, const char* source));
WRENCH_DECL(bool, EndModule, (WrenVM* vm));

/* Declare and define modules so they can be imported in Wren code.
 * Source may be NULL, in which case we fallback to load from disk.
 */
WRENCH_DECL(bool, RegisterModuleEx, (WrenVM* vm, const char* moduleName, const char* source, size_t num_chars, bool copy_source));
WRENCH_DECL(bool, RegisterModule, (WrenVM* vm, const char* moduleName, const char* source));

/* Declare a foreign class within a module.
 */
WRENCH_DECL(bool, RegisterClass, (WrenVM* vm, const char* moduleName, const char* className, WrenForeignMethodFn ctor, WrenFinalizerFn dtor));

/* Declare a foreign method within a class.
 */
WRENCH_DECL(bool, RegisterMethod, (WrenVM* vm, const char* moduleName, const char* className, bool isStatic, const char* signature, WrenForeignMethodFn method));

// TODO: wrenForEachModule
// TODO: wrenForEachClassInModule
// TODO: wrenForEachMethodInClass

// TODO: wrenCountModules
// TODO: wrenListModules

// TODO: wrenCountClassesInModule
// TODO: wrenListClassesInModule

// TODO: wrenCountMethodsInClass
// TODO: wrenListMethodsInClass

WRENCH_DECL(float, GetSlotFloat, (WrenVM* vm, int slot));
WRENCH_DECL(void, SetSlotFloat, (WrenVM* vm, int slot, float value));

WRENCH_DECL(int, GetSlotInt, (WrenVM* vm, int slot));
WRENCH_DECL(void, SetSlotInt, (WrenVM* vm, int slot, int value));

WRENCH_DECL(uint8_t, GetSlotByte, (WrenVM* vm, int slot));
WRENCH_DECL(void, SetSlotByte, (WrenVM* vm, int slot, uint8_t value));

/* WrenConfiguration callbacks.
 */
WRENCH_DECL(void*, DefaultReallocate, (void* ptr, size_t newSize, void* userData));
WRENCH_DECL(const char*, DefaultResolveModule, (WrenVM* vm, const char* importer, const char* name));
WRENCH_DECL(WrenLoadModuleResult, DefaultLoadModule, (WrenVM* vm, const char* name));
WRENCH_DECL(WrenForeignMethodFn, DefaultBindForeignMethod, (WrenVM* vm, const char* moduleName, const char* className, bool is_static, const char* signature));
WRENCH_DECL(WrenForeignClassMethods, DefaultBindForeignClass, (WrenVM* vm, const char* moduleName, const char* className));
WRENCH_DECL(void, DefaultWrite, (WrenVM* vm, const char* text));
WRENCH_DECL(void, DefaultError, (WrenVM* vm, WrenErrorType type, const char* moduleName, int line, const char* message));

#endif /* __WRENCH_H__ */

/*
================================================================================
 * ~~ [ implementation ] ~~ *
--------------------------------------------------------------------------------
*/

#ifdef WRENCH_IMPLEMENTATION
/*
 * Enable multiple file inclusions with `WRENCH_IMPLEMENTATION` for ease of use.
 */
#ifndef __WRENCH_C__
#define __WRENCH_C__

/* ===== [ standard library ] =============================================== */

#ifndef WRENCH_NO_CSTDLIB
    #include <assert.h>
    #include <float.h>
    #include <limits.h>
    #include <math.h>
    #include <stdint.h>
    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>
#endif

#if _WIN32 && !WRENCH_NO_WINDOWS_H
    /*
     * Attempt to speed up build.
     */
    #ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN 1
    #endif
    #ifndef VC_EXTRALEAN
    #define VC_EXTRALEAN 1
    #endif
    #include <windows.h>
#endif

#if !_WIN32 && !WRENCH_NO_POSIX_HEADERS
    #include <dlfcn.h>
#endif

#if !defined(wrench_alloca)
    #if _MSC_VER
        #define wrench_alloca _alloca
    #else
        #define wrench_alloca alloca
    #endif
#endif
#ifndef wrench_assert
#define wrench_assert assert
#endif
#ifndef wrench_calloc
#define wrench_calloc calloc
#endif
#ifndef wrench_fabs
#define wrench_fabs fabs
#endif
#ifndef wrench_fclose
#define wrench_fclose fclose
#endif
#ifndef wrench_fopen
#define wrench_fopen fopen
#endif
#ifndef wrench_fprintf
#define wrench_fprintf fprintf
#endif
#ifndef wrench_fread
#define wrench_fread fread
#endif
#ifndef wrench_free
#define wrench_free free
#endif
#ifndef wrench_fseek
#define wrench_fseek fseek
#endif
#ifndef wrench_ftell
#define wrench_ftell ftell
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
#ifndef wrench_putchar
#define wrench_putchar putchar
#endif
#ifndef wrench_realloc
#define wrench_realloc realloc
#endif
#ifndef wrench_snprintf
#define wrench_snprintf snprintf
#endif
#ifndef wrench_stderr
#define wrench_stderr stderr
#endif
#ifndef wrench_stdout
#define wrench_stdout stdout
#endif
#ifndef wrench_strchr
#define wrench_strchr strchr
#endif
#ifndef wrench_strcmp
#define wrench_strcmp strcmp
#endif
#if !defined(wrench_strdup)
    #if _MSC_VER
        #define wrench_strdup _strdup
    #else
        #define wrench_strdup strdup
    #endif
#endif
#ifndef wrench_strlen
#define wrench_strlen strlen
#endif
#ifndef wrench_strstr
#define wrench_strstr strstr
#endif
#ifndef wrench_trunc
#define wrench_trunc trunc
#endif

/* ===== [ utilities ] ====================================================== */

/* Indicates that a function is not implemented.
 */
#if !defined(WRENCH_STUB)
    #if 1
        #define WRENCH_STUB() wrench_fprintf(wrench_stderr, "TODO %s (file \"%s\", line %i)\n", __FUNCTION__, __FILE__, __LINE__)
    #else
        #define WRENCH_STUB() wrench_assert(!"TODO")
    #endif
#endif /* !WRENCH_STUB */

/* Indicates that a function is partially implemented.
 */
#ifndef WRENCH_TEMP
#define WRENCH_TEMP() //wrench_fprintf(wrench_stderr, "TEMP %s (file \"%s\", line %i)\n", __FUNCTION__, __FILE__, __LINE__)
#endif

/* Optional program entry point.
 */
//#ifndef WRENCH_MAIN
//#define WRENCH_MAIN main
//#endif

#ifndef WRENCH_ARRAY_COUNT
#define WRENCH_ARRAY_COUNT(a) (sizeof(a) / sizeof((a)[0]))
#endif

#if !defined(WRENCH_STRINGIFY)
    #define WRENCH_STRINGIFY(s) _WRENCH_STR_IMPL(s)
    #define _WRENCH_STR_IMPL(s) #s
#endif

/* Macros for type conversion.
 */
#ifndef WRENCH_DBL_EPSILON
#define WRENCH_DBL_EPSILON 0.000001
#endif

#ifndef WRENCH_NUM_IS_SAFE_FLT
#define WRENCH_NUM_IS_SAFE_FLT(x) (wrench_fabs(x) < ((double)FLT_MAX + WRENCH_DBL_EPSILON))
#endif

#ifndef WRENCH_NUM_TO_INT
#define WRENCH_NUM_TO_INT(x) ((int)wrench_trunc(x))
#endif
#ifndef WRENCH_NUM_IS_INT
#define WRENCH_NUM_IS_INT(x) (wrench_fabs(WRENCH_NUM_TO_INT(x) - (x)) < WRENCH_DBL_EPSILON)
#endif

#ifndef WRENCH_MIN_SAFE_INT
#define WRENCH_MIN_SAFE_INT -9007199254740991
#endif
#ifndef WRENCH_MAX_SAFE_INT
#define WRENCH_MAX_SAFE_INT 9007199254740991 // 2 ^ 53 - 1
#endif

#ifndef WRENCH_NUM_IS_SAFE_INT
#define WRENCH_NUM_IS_SAFE_INT(x) ((x) > ((double)WRENCH_MIN_SAFE_INT - WRENCH_DBL_EPSILON) \
                                && (x) < ((double)WRENCH_MAX_SAFE_INT + WRENCH_DBL_EPSILON))
#endif

/* Macros for C type checking.
 */
#if !defined(WRENCH_DEBUG)
    #if defined(_DEBUG) || defined(DEBUG) || (defined(__GNUC__) && !defined(__OPTIMIZE__))
        #define WRENCH_DEBUG 1
    #else
        #define WRENCH_DEBUG 0
    #endif
#endif /* !WRENCH_DEBUG */

#if WRENCH_DEBUG
    #ifndef WRENCH_MAGIC_TAG
    #define WRENCH_MAGIC_TAG const char* _magic_tag
    #endif

    #ifndef WRENCH_CHECK_MAGIC_TAG
    #define WRENCH_CHECK_MAGIC_TAG(data, type_name) do                                                      \
    {                                                                                                       \
        wrench_assert((data) != NULL);                                                                      \
        wrench_assert(((type_name*)(data))->_magic_tag != NULL);                                            \
        wrench_assert(wrench_strcmp(((type_name*)(data))->_magic_tag, WRENCH_STRINGIFY(type_name)) == 0);   \
    }                                                                                                       \
    while (0)

    #endif /* WRENCH_CHECK_MAGIC_TAG */

    #ifndef WRENCH_SET_MAGIC_TAG
    #define WRENCH_SET_MAGIC_TAG(data, type_name) ((type_name*)(data))->_magic_tag = WRENCH_STRINGIFY(type_name)
    #endif
#else
    #ifndef WRENCH_MAGIC_TAG
    #define WRENCH_MAGIC_TAG
    #endif

    #ifndef WRENCH_CHECK_MAGIC_TAG
    #define WRENCH_CHECK_MAGIC_TAG(data, type_name) ((void)0)
    #endif

    #ifndef WRENCH_SET_MAGIC_TAG
    #define WRENCH_SET_MAGIC_TAG(data, type_name) ((void)0)
    #endif
#endif /* WRENCH_DEBUG */

/* ===== [ context & nodes ] ================================================ */

typedef struct WrenchMethod
{
//  struct WrenchMethod* prev;
    struct WrenchMethod* next;

//  struct WrenchClass* klass;

    bool is_static;
    const char* signature;
    WrenForeignMethodFn method;
}
WrenchMethod;

typedef struct WrenchClass
{
//  struct WrenchClass* prev;
    struct WrenchClass* next;

    WrenchMethod* method_head;
    WrenchMethod* method_tail;

//  struct WrenchModule* module;

    const char* name;
    WrenForeignMethodFn ctor;
    WrenFinalizerFn dtor;
}
WrenchClass;

typedef struct WrenchModule
{
//  struct WrenchModule* prev;
    struct WrenchModule* next;

    WrenchClass* class_head;
    WrenchClass* class_tail;

    const char* name;
    const char* source;

    void* library;
}
WrenchModule;

typedef struct WrenchContext
{
    WRENCH_MAGIC_TAG;

    char error[1024 * 4];
    char* base_path;

    int argc;
    char** argv;

    WrenchModule* module_head;
    WrenchModule* module_tail;

    char* node_alloc_base;
    char* node_alloc_end;
    char* node_alloc_mark;

    char* source_code_alloc_base;
    char* source_code_alloc_end;
    char* source_code_alloc_mark;

    WrenchModule* module_being_built;
    char* module_builder_base;
    bool foreign_library_load_disabled;

    wrenFileReadFn file_read_callback;
    wrenFileFreeFn file_free_callback;

    WrenVM* vm;
    void* userdata[16];
}
WrenchContext;

static WrenchMethod* wrenchGetMethod(WrenchContext* context, WrenchClass* klass, bool is_static, const char* signature, WrenchMethod** previous)
{
    wrench_assert(klass != NULL);

    WrenchMethod* node = klass->method_head;
    WrenchMethod* prev = NULL;

    while (node != NULL)
    {
        if (is_static == node->is_static && wrench_strcmp(signature, node->signature) == 0)
        {
            if (previous != NULL)
            {
                *previous = prev;
            }

            return node;
        }

        prev = node;
        node = node->next;
    }

    if (previous != NULL)
    {
        *previous = NULL;
    }

    return NULL;
}

static WrenchClass* wrenchGetClass(WrenchContext* context, WrenchModule* module, const char* name, WrenchClass** previous)
{
    wrench_assert(module != NULL);

    WrenchClass* node = module->class_head;
    WrenchClass* prev = NULL;

    while (node != NULL)
    {
        if (wrench_strcmp(name, node->name) == 0)
        {
            if (previous != NULL)
            {
                *previous = prev;
            }

            return node;
        }

        prev = node;
        node = node->next;
    }

    if (previous != NULL)
    {
        *previous = NULL;
    }

    return NULL;
}

static WrenchModule* wrenchGetModule(WrenchContext* context, const char* name, WrenchModule** previous)
{
    WrenchModule* node = context->module_head;
    WrenchModule* prev = NULL;

    while (node != NULL)
    {
        if (wrench_strcmp(name, node->name) == 0)
        {
            if (previous != NULL)
            {
                *previous = prev;
            }

            return node;
        }

        prev = node;
        node = node->next;
    }

    if (previous != NULL)
    {
        *previous = NULL;
    }

    return NULL;
}

static void wrenchUnlinkMethod(WrenchContext* context, WrenchClass* klass, WrenchMethod* method, WrenchMethod* previous)
{
    if (previous != NULL)
    {
        previous->next = method->next;
    }

    if (klass->method_head == method)
    {
        klass->method_head = method->next;
    }

    if (klass->method_tail == method)
    {
        klass->method_tail = previous;
    }
}

static void wrenchUnlinkClass(WrenchContext* context, WrenchModule* module, WrenchClass* klass, WrenchClass* previous)
{
    if (previous != NULL)
    {
        previous->next = klass->next;
    }

    if (module->class_head == klass)
    {
        module->class_head = klass->next;
    }

    if (module->class_tail == klass)
    {
        module->class_tail = previous;
    }
}

static void wrenchUnlinkModule(WrenchContext* context, WrenchModule* module, WrenchModule* previous)
{
    if (previous != NULL)
    {
        previous->next = module->next;
    }

    if (context->module_head == module)
    {
        context->module_head = module->next;
    }

    if (context->module_tail == module)
    {
        context->module_tail = previous;
    }
}

static void* wrenchNodeAlloc(WrenchContext* context, size_t size, bool clear)
{
    wrench_assert(context->node_alloc_base != NULL);
    wrench_assert(context->node_alloc_end != NULL);
    wrench_assert(context->node_alloc_mark != NULL);

    char* data = context->node_alloc_mark;
    char* next = context->node_alloc_mark + size;

    if (next > context->node_alloc_end)
    {
        // TODO: Out of memory - set error message here.
        return NULL;
    }

    if (clear)
    {
        wrench_memset(data, 0, size);
    }

    context->node_alloc_mark = next;
    return (void*)data;
}

static char* wrenchStringAlloc(WrenchContext* context, size_t num_chars)
{
    char* data = (char*)wrenchNodeAlloc(context, num_chars + 1, false);

    if (data != NULL)
    {
        data[num_chars] = '\0';
    }

    return data;
}

static const char* wrenchStringCopyEx(WrenchContext* context, const char* string, size_t num_chars)
{
    char* data = wrenchStringAlloc(context, num_chars);

    if (data != NULL)
    {
        wrench_memcpy(data, string, num_chars);
    }

    return (const char*)data;
}

static const char* wrenchStringCopy(WrenchContext* context, const char* string)
{
    return wrenchStringCopyEx(context, string, wrench_strlen(string));
}

static char* wrenchSourceCodeAlloc(WrenchContext* context, size_t num_chars)
{
    wrench_assert(context->source_code_alloc_base != NULL);
    wrench_assert(context->source_code_alloc_end != NULL);
    wrench_assert(context->source_code_alloc_mark != NULL);

    // Can't allocate source code between wren(Begin/End)Module pair.
    wrench_assert(context->module_being_built == NULL);
    wrench_assert(context->module_builder_base == NULL);

    char* data = context->source_code_alloc_mark;
    char* next = context->source_code_alloc_mark + (num_chars + 1);

    if (next > context->source_code_alloc_end)
    {
        // TODO: Out of memory - set error message here.
        return NULL;
    }

    context->source_code_alloc_mark = next;
    data[num_chars] = '\0';

    return data;
}

static const char* wrenchSourceCodeCopyEx(WrenchContext* context, const char* source, size_t num_chars)
{
    char* data = wrenchSourceCodeAlloc(context, num_chars);

    if (data != NULL)
    {
        wrench_memcpy(data, source, num_chars);
    }

    return (const char*)data;
}

static const char* wrenchSourceCodeCopy(WrenchContext* context, const char* source)
{
    return wrenchSourceCodeCopyEx(context, source, wrench_strlen(source));
}

static bool wrenchGetForeignLibraryLoadEnabled(WrenchContext* context)
{
    return !context->foreign_library_load_disabled;
}

static void wrenchSetForeignLibraryLoadEnabled(WrenchContext* context, bool value)
{
    context->foreign_library_load_disabled = !value;
}

static const char* wrenchGetErrorString(WrenchContext* context)
{
    return (const char*)context->error;
}

static void wrenchSetErrorString(WrenchContext* context, const char* error)
{
    size_t length = wrench_strlen(error);

    if (length > sizeof(context->error) - 1)
    {
        length = sizeof(context->error) - 1;
    }

    char* dst = context->error;
    char* src = (char*)error;

    while (*src)
    {
        *dst++ = *src++;
    }

    context->error[length] = '\0';
}

static void* wrenchGetUserDataEx(WrenchContext* context, int slot)
{
    wrench_assert(slot < (int)WRENCH_ARRAY_COUNT(context->userdata));
    return context->userdata[slot];
}

static void wrenchSetUserDataEx(WrenchContext* context, int slot, void* userData)
{
    wrench_assert(slot < (int)WRENCH_ARRAY_COUNT(context->userdata));
    context->userdata[slot] = userData;
}

static void wrenchFreeCommandLine(WrenchContext* context)
{
    if (context->argv != NULL)
    {
        for (int i = 0, n = context->argc; i < n; i++)
        {
            wrench_free(context->argv[i]);
        }

        wrench_free(context->argv);
        context->argv = NULL;
    }

    context->argc = 0;
}

static char** wrenchGetCommandLine(WrenchContext* context, int* argc)
{
    if (argc != NULL)
    {
        *argc = context->argc;
    }

    return context->argv;
}

static bool wrenchSetCommandLine(WrenchContext* context, int argc, char** argv)
{
    // Just clear everything to a clean slate for simplicity's sake.
    wrenchFreeCommandLine(context);

    // TODO: Could just malloc this buffer and write the final NULL.
    context->argv = (char**)wrench_calloc(argc + 1, sizeof(char**));

    if (context->argv == NULL)
    {
        wrenchSetErrorString(context, "Out of memory - failed to copy argv.");
        return false;
    }

    for (int i = 0; i < argc; i++)
    {
        context->argv[i] = wrench_strdup(argv[i]);

        if (context->argv[i] == NULL)
        {
            for (int j = 0; j < i; j++)
            {
                wrench_free(context->argv[j]);
            }

            wrench_free(context->argv);
            context->argv = NULL;

            wrenchSetErrorString(context, "Out of memory - failed to copy argv.");
            return false;
        }
    }

    context->argc = argc;
    return true;
}

static const char* wrenchGetModuleSource(WrenchContext* context, const char* name)
{
    WrenchModule* module = wrenchGetModule(context, name, NULL);

    if (module != NULL)
    {
        return module->source;
    }
    else
    {
        return NULL;
    }
}

static const char* wrenchGetBasePath(WrenchContext* context)
{
    if (context->base_path != NULL)
    {
        return (const char*)context->base_path;
    }
    else
    {
        return "./";
    }
}

static bool wrenchSetBasePath(WrenchContext* context, const char* path)
{
    wrench_free(context->base_path);

    context->base_path = wrench_strdup(path);
    return context->base_path != NULL;
}

static wrenFileReadFn wrenchGetFileReadCallback(WrenchContext* context)
{
    return context->file_read_callback;
}

static void wrenchSetFileReadCallback(WrenchContext* context, wrenFileReadFn callback)
{
    context->file_read_callback = callback;
}

static wrenFileFreeFn wrenchGetFileFreeCallback(WrenchContext* context)
{
    return context->file_free_callback;
}

static void wrenchSetFileFreeCallback(WrenchContext* context, wrenFileFreeFn callback)
{
    context->file_free_callback = callback;
}

static const char* wrenchLoadSourceFile(WrenchContext* context, const char* name, size_t* num_chars)
{
    if (wrench_strstr(name, ".wren") != NULL) // Strip file extension.
    {
        char b[1024], *src = (char*)name, *dst = b;

        for (;;)
        {
            if (*src == '.')
            {
                *dst = '\0';
                break;
            }
            else
            {
                *dst++ = *src++;
            }
        }

        return wrenchLoadSourceFile(context, (const char*)b, num_chars);
    }

    // Don't expect the user's file read callbacks to check for NULL.
    // TODO: Optionally preprocess Wren files with a C preprocessor.
    size_t _num_chars;

    if (num_chars == NULL)
    {
        num_chars = &_num_chars;
    }

    *num_chars = 0;
    wrenFileReadFn read_func = wrenchGetFileReadCallback(context);

    if (read_func == NULL)
    {
        read_func = wrenDefaultFileRead;
    }

    void* code = read_func(context->vm, name, num_chars);

    if (code != NULL)
    {
        const char* copy = wrenchSourceCodeCopyEx(context, (const char*)code, *num_chars);

        wrenFileFreeFn free_func = wrenchGetFileFreeCallback(context);

        if (free_func == NULL)
        {
            free_func = wrenDefaultFileFree;
        }

        free_func(context->vm, code, *num_chars);
        return copy;
    }
    else
    {
        return NULL;
    }
}

static bool wrenchRegisterModuleImpl(WrenchContext* context, WrenchModule* node, const char* source, size_t num_chars, bool copy_source)
{
    if (copy_source && source != NULL)
    {
        node->source = wrenchSourceCodeCopyEx(context, source, num_chars);

        if (node->source == NULL)
        {
            char error[1024 * 4];

            wrench_snprintf(error, sizeof(error), "Failed to copy source code for \"%s\".", node->name);
            wrenchSetErrorString(context, (const char*)error);

            return false;
        }
    }
    else
    {
        node->source = source;
    }

    if (context->module_head == NULL)
    {
        context->module_head = node;
    }
    else
    {
        context->module_tail->next = node;
    }

    context->module_tail = node;
    return true;
}

static bool wrenchBeginModule(WrenchContext* context, const char* name)
{
    wrench_assert(wrenchGetModule(context, name, NULL) == NULL);

    wrench_assert(context->module_builder_base == NULL);
    context->module_builder_base = context->source_code_alloc_mark;

    wrench_assert(context->module_being_built == NULL);
    context->module_being_built = (WrenchModule*)wrenchNodeAlloc(context, sizeof(WrenchModule), true);

    if (context->module_being_built == NULL)
    {
        context->module_builder_base = NULL;

        // TODO: Out of memory - set error message here.
        return false;
    }

    context->module_being_built->name = wrenchStringCopy(context, name);

    if (context->module_being_built->name == NULL)
    {
        context->module_builder_base = NULL;
        context->module_being_built = NULL;

        // TODO: Out of memory - set error message here.
        return false;
    }

    return true;
}

static bool wrenchCodeEx(WrenchContext* context, const char* source, size_t num_chars)
{
    char* data = context->source_code_alloc_mark;
    char* next = context->source_code_alloc_mark + num_chars;

    if (next > context->source_code_alloc_end)
    {
        // TODO: Out of memory - set error message here.
        return false;
    }

    wrench_memcpy(context->source_code_alloc_mark, source, num_chars);
    context->source_code_alloc_mark = next;

    return true;
}

static bool wrenchCode(WrenchContext* context, const char* source)
{
    return wrenchCodeEx(context, source, wrench_strlen(source));
}

static bool wrenchEndModule(WrenchContext* context)
{
    size_t num_chars;
    bool r = true;

    if (context->source_code_alloc_mark == context->source_code_alloc_end)
    {
        // TODO: Out of memory - set error message here.
        r = false;
        goto done;
    }

    num_chars = (size_t)(context->source_code_alloc_mark - context->module_builder_base);
    *context->source_code_alloc_mark++ = '\0';

    wrench_assert(wrench_strlen(context->module_builder_base) == num_chars);

    if (!wrenchRegisterModuleImpl(context, context->module_being_built,
        (const char *)context->module_builder_base, num_chars, false))
    {
        r = false;
    }

    done:
    {
        context->module_builder_base = NULL;
        context->module_being_built = NULL;

        return r;
    }
}

static bool wrenchRegisterModuleEx(WrenchContext* context, const char* moduleName, const char* source, size_t num_chars, bool copy_source)
{
    // Module already registered - get name in GDB backtrace arguments.
    wrench_assert(wrenchGetModule(context, moduleName, NULL) == NULL);

    WrenchModule* node = (WrenchModule*)wrenchNodeAlloc(context, sizeof(WrenchModule), true);

    if (node == NULL)
    {
        // TODO: Out of memory - set error message here.
        return false;
    }

    node->name = wrenchStringCopy(context, moduleName);

    if (node->name == NULL)
    {
        // TODO: Out of memory - set error message here.
        return false;
    }

    return wrenchRegisterModuleImpl(context, node, source, num_chars, copy_source);
}

static bool wrenchRegisterModule(WrenchContext* context, const char* moduleName, const char* source)
{
    return wrenchRegisterModuleEx(context, moduleName, source, source != NULL ? wrench_strlen(source) : 0, true);
}

static bool wrenchRegisterClass(WrenchContext* context, const char* moduleName, const char* className, WrenForeignMethodFn ctor, WrenFinalizerFn dtor)
{
    WrenchModule* module = context->module_being_built;

    if (module != NULL)
    {
        wrench_assert(context->module_builder_base != NULL);
    }
    else
    {
        module = wrenchGetModule(context, moduleName, NULL);
    }

    wrench_assert(module != NULL);
    wrench_assert(wrenchGetClass(context, module, className, NULL) == NULL);

    WrenchClass* node = (WrenchClass*)wrenchNodeAlloc(context, sizeof(WrenchClass), true);

    if (node == NULL)
    {
        // TODO: Out of memory - set error message here.
        return false;
    }

    node->name = wrenchStringCopy(context, className);

    if (node->name == NULL)
    {
        // TODO: Out of memory - set error message here.
        return false;
    }

    //node->module = module;

    node->ctor = ctor;
    node->dtor = dtor;

    /* Add to the class linked list.
     */
    if (module->class_head == NULL)
    {
        module->class_head = node;
    }
    else
    {
        module->class_tail->next = node;
    }

    module->class_tail = node;
    return true;
}

static bool wrenchRegisterMethod(WrenchContext* context, const char* moduleName, const char* className, bool is_static, const char* signature, WrenForeignMethodFn method)
{
    WrenchModule* module = context->module_being_built;

    if (module != NULL)
    {
        wrench_assert(wrench_strcmp(module->name, moduleName) == 0);
        wrench_assert(context->module_builder_base != NULL);
    }
    else
    {
        module = wrenchGetModule(context, moduleName, NULL);
    }

    wrench_assert(module != NULL);

    WrenchClass* klass = wrenchGetClass(context, module, className, NULL);
    wrench_assert(klass != NULL);

    wrench_assert(wrenchGetMethod(context, klass, is_static, signature, NULL) == NULL);
    WrenchMethod* node = (WrenchMethod*)wrenchNodeAlloc(context, sizeof(WrenchMethod), true);

    if (node == NULL)
    {
        // TODO: Out of memory - set error message here.
        return false;
    }

    node->signature = wrenchStringCopy(context, signature);

    if (node->signature == NULL)
    {
        // TODO: Out of memory - set error message here.
        return false;
    }

    //node->klass = klass

    node->is_static = is_static;
    node->method = method;

    /* Add to the method linked list.
     */
    if (klass->method_head == NULL)
    {
        klass->method_head = node;
    }
    else
    {
        klass->method_tail->next = node;
    }

    klass->method_tail = node;
    return true;
}

static void* wrenchLoadLibrary(WrenchContext* context, const char* name)
{
    if (context->foreign_library_load_disabled)
    {
        return NULL;
    }

    #if _WIN32
    {
        char path[1024 * 4];

        wrench_snprintf(path, sizeof(path), "%s%s.dll", wrenchGetBasePath(context), name);
        HMODULE library = LoadLibraryA((LPCSTR)path);

        if (library == NULL)
        {
            char error[1024 * 4];

            wrench_snprintf(error, sizeof(error), "Failed to load library \"%s\"", name);
            wrenchSetErrorString(context, (const char*)error);
        }

        return (void*)library;
    }
    #else
    {
        // TODO: lib prefix?

        char path[1024 * 4];
        void* library;

        wrench_snprintf(path, sizeof(path), "%s%s.so", wrenchGetBasePath(context), name);
        library = dlopen(path, RTLD_LAZY);

        if (library != NULL)
        {
            return library;
        }

        wrench_snprintf(path, sizeof(path), "%s.so", name);
        library = dlopen(path, RTLD_LAZY);

        if (library != NULL)
        {
            return library;
        }

        // TODO: Try *.so.1 etc?

        wrenchSetErrorString(context, (const char*)dlerror());
        return NULL;
    }
    #endif
}

static void* wrenchLibraryFunc(WrenchContext* context, void* library, const char* name)
{
    if (0)
    {
        wrench_assert(library != NULL);
    }
    else if (library == NULL)
    {
        char s[1024];

        wrench_snprintf(s, sizeof(s), "Cannot load function \"%s\": library not open.", name);
        wrenchSetErrorString(context, (const char*)s);

        return NULL;
    }

    #if _WIN32
    {
        FARPROC func = GetProcAddress((HMODULE)library, name);

        if (func == NULL)
        {
            char error[1024 * 4];

            wrench_snprintf(error, sizeof(error), "Failed to load function \"%s\"", name);
            wrenchSetErrorString(context, (const char*)error);
        }

        return (void*)func;
    }
    #else
    {
        void* func = dlsym(library, name);

        if (func == NULL)
        {
            wrenchSetErrorString(context, (const char*)dlerror());
        }

        return func;
    }
    #endif
}

static void wrenchFreeLibrary(WrenchContext* context, void* library)
{
    if (0)
    {
        wrench_assert(library != NULL);
    }
    else if (library == NULL)
    {
        return;
    }

    #if _WIN32
    {
        // TODO: Technically this can fail - check GetLastError.
        FreeLibrary((HMODULE)library);
    }
    #else
    {
        if (dlclose(library) != 0)
        {
            wrench_fprintf(wrench_stderr, "%s\n", dlerror());
        }
    }
    #endif
}

static WrenchContext* wrenchNewContext(WrenVM* vm)
{
    WrenchContext* context = (WrenchContext*)wrench_calloc(1, sizeof(WrenchContext));

    if (context == NULL)
    {
        // Don't set error string here, as we're going to free the VM soon anyways.
        return NULL;
    }

    // For Wren calls.
    context->vm = vm;

    #ifndef WRENCH_NODE_BUFFER_SIZE
    #define WRENCH_NODE_BUFFER_SIZE (1024 * 1024 * 1)
    #endif
    context->node_alloc_base = (char*)wrench_malloc(WRENCH_NODE_BUFFER_SIZE);

    if (context->node_alloc_base == NULL)
    {
        wrench_free(context);
        return NULL;
    }

    context->node_alloc_end = context->node_alloc_base + WRENCH_NODE_BUFFER_SIZE;
    context->node_alloc_mark = context->node_alloc_base;

    #ifndef WRENCH_SOURCE_CODE_BUFFER_SIZE
    #define WRENCH_SOURCE_CODE_BUFFER_SIZE (1024 * 1024 * 1)
    #endif
    context->source_code_alloc_base = (char*)wrench_malloc(WRENCH_SOURCE_CODE_BUFFER_SIZE);

    if (context->source_code_alloc_base == NULL)
    {
        wrench_free(context->node_alloc_base);
        wrench_free(context);

        return NULL;
    }

    context->source_code_alloc_end = context->source_code_alloc_base + WRENCH_SOURCE_CODE_BUFFER_SIZE;
    context->source_code_alloc_mark = context->source_code_alloc_base;

    return context;
}

static void wrenchFreeContext(WrenchContext* context)
{
    for (WrenchModule* node = context->module_head; node != NULL; node = node->next)
    {
        if (node->library != NULL)
        {
            char quit_name[1024];
            wrench_snprintf(quit_name, sizeof(quit_name), "%sWrenQuit", node->name);

            wrenLibraryQuitFn quit = (
            wrenLibraryQuitFn)wrenchLibraryFunc(context, node->library, (const char*)quit_name);

            if (quit != NULL)
            {
                quit();
            }

            wrenchFreeLibrary(context, node->library);
        }
    }

    if (0) // Internal, vestigial debugging code.
    {
        for (int i = 0; i < 80; i++)
        {
            wrench_putchar('-');
        }

        wrench_putchar('\n');

        for (char* s = context->source_code_alloc_base; s < context->source_code_alloc_mark; s++)
        {
            if (*s)
            {
                wrench_putchar(*s);
            }
        }

        for (int i = 0; i < 80; i++)
        {
            wrench_putchar('-');
        }

        wrench_putchar('\n');
    }

    wrenchFreeCommandLine(context);

    wrench_free(context->source_code_alloc_base);
    wrench_free(context->node_alloc_base);
    wrench_free(context->base_path);

    wrench_free(context);
}

static wrenLibraryInitFn wrenchGlobalInitFunc[16];
static size_t wrenchGlobalInitFuncCount;

static wrenLibraryQuitFn wrenchGlobalQuitFunc[16];
static size_t wrenchGlobalQuitFuncCount;

/* ===== [ public API ] ===================================================== */

WRENCH_IMPL(WrenConfiguration*, GetConfig, (void))
{
    static WrenConfiguration wrench_config;
    static bool wrench_config_is_init;

    if (!wrench_config_is_init)
    {
        wrenInitConfiguration(&wrench_config);

        wrench_config.reallocateFn = wrenDefaultReallocate;
        wrench_config.resolveModuleFn = wrenDefaultResolveModule;
        wrench_config.loadModuleFn = wrenDefaultLoadModule;
        wrench_config.bindForeignMethodFn = wrenDefaultBindForeignMethod;
        wrench_config.bindForeignClassFn = wrenDefaultBindForeignClass;
        wrench_config.writeFn = wrenDefaultWrite;
        wrench_config.errorFn = wrenDefaultError;

        wrench_config_is_init = true;
    }

    return &wrench_config;
}

WRENCH_IMPL(WrenVM*, NewExtendedVM, (int argc, char** argv, bool call_global_init_funcs))
{
    WrenVM* vm = wrenNewVM(wrenGetConfig());

    if (vm == NULL)
    {
        return NULL;
    }

    WrenchContext* context = wrenchNewContext(vm);

    if (context == NULL)
    {
        wrenFreeVM(vm);
        return NULL;
    }

    wrenSetUserData(vm, context);

    if (!wrenchSetCommandLine(context, argc, argv))
    {
        wrenFreeExtendedVM(vm, false);
        return NULL;
    }

    if (call_global_init_funcs)
    {
        for (size_t i = 0; i < wrenchGlobalInitFuncCount; i++)
        {
            wrench_assert(wrenchGlobalInitFunc[i] != NULL);

            if (!wrenchGlobalInitFunc[i](vm))
            {
                wrenFreeExtendedVM(vm, false);
                return NULL;
            }
        }
    }

    return vm;
}

WRENCH_IMPL(void, FreeExtendedVM, (WrenVM* vm, bool call_global_quit_funcs))
{
    if (vm == NULL)
    {
        return;
    }

    if (call_global_quit_funcs)
    {
        /* TODO: Call in reverse order?
         */
        for (size_t i = 0; i < wrenchGlobalQuitFuncCount; i++)
        {
            wrench_assert(wrenchGlobalQuitFunc[i] != NULL);
            wrenchGlobalQuitFunc[i]();
        }
    }

    WrenchContext* context = (WrenchContext*)wrenGetUserData(vm);
    wrench_assert(context != NULL);

    // We must free the VM first, before dtors in shared libs are unmapped.
    wrenFreeVM(vm);

    wrenchFreeContext(context);
}

WRENCH_IMPL(void, RegisterGlobalInitFunction, (wrenLibraryInitFn init))
{
    wrench_assert(init != NULL); // TODO: Don't register funcs that are already in array.
    wrench_assert(wrenchGlobalInitFuncCount < WRENCH_ARRAY_COUNT(wrenchGlobalInitFunc));

    wrenchGlobalInitFunc[wrenchGlobalInitFuncCount++] = init;
}

WRENCH_IMPL(void, RegisterGlobalQuitFunction, (wrenLibraryQuitFn quit))
{
    wrench_assert(quit != NULL); // TODO: Don't register funcs that are already in array.
    wrench_assert(wrenchGlobalQuitFuncCount < WRENCH_ARRAY_COUNT(wrenchGlobalQuitFunc));

    wrenchGlobalQuitFunc[wrenchGlobalQuitFuncCount++] = quit;
}

WRENCH_IMPL(bool, GetForeignLibraryLoadEnabled, (WrenVM* vm))
{
    if (vm != NULL)
    {
        WrenchContext* context = (WrenchContext*)wrenGetUserData(vm);
        wrench_assert(context != NULL);

        return wrenchGetForeignLibraryLoadEnabled(context);
    }
    else
    {
        return false;
    }
}

WRENCH_IMPL(void, SetForeignLibraryLoadEnabled, (WrenVM* vm, bool enabled))
{
    if (vm == NULL)
    {
        return;
    }

    WrenchContext* context = (WrenchContext*)wrenGetUserData(vm);
    wrench_assert(context != NULL);

    wrenchSetForeignLibraryLoadEnabled(context, enabled);
}

WRENCH_IMPL(const char*, GetErrorString, (WrenVM* vm))
{
    if (vm != NULL)
    {
        WrenchContext* context = (WrenchContext*)wrenGetUserData(vm);
        wrench_assert(context != NULL);

        return wrenchGetErrorString(context);
    }
    else
    {
        return "NULL Wren virtual machine.";
    }
}

WRENCH_IMPL(void, SetErrorString, (WrenVM* vm, const char* error))
{
    if (vm == NULL)
    {
        return;
    }

    WrenchContext* context = (WrenchContext*)wrenGetUserData(vm);
    wrench_assert(context != NULL);

    wrenchSetErrorString(context, error);
}

WRENCH_IMPL(void*, GetUserDataEx, (WrenVM* vm, int slot))
{
    if (vm != NULL)
    {
        WrenchContext* context = (WrenchContext*)wrenGetUserData(vm);
        wrench_assert(context != NULL);

        return wrenchGetUserDataEx(context, slot);
    }
    else
    {
        return NULL;
    }
}

WRENCH_IMPL(void, SetUserDataEx, (WrenVM* vm, int slot, void* userData))
{
    if (vm == NULL)
    {
        return;
    }

    WrenchContext* context = (WrenchContext*)wrenGetUserData(vm);
    wrench_assert(context != NULL);

    wrenchSetUserDataEx(context, slot, userData);
}

WRENCH_IMPL(char**, GetCommandLine, (WrenVM* vm, int* argc))
{
    if (vm != NULL)
    {
        WrenchContext* context = (WrenchContext*)wrenGetUserData(vm);
        wrench_assert(context != NULL);

        return wrenchGetCommandLine(context, argc);
    }
    else
    {
        if (argc != NULL)
        {
            *argc = 0;
        }

        return NULL;
    }
}

WRENCH_IMPL(bool, SetCommandLine, (WrenVM* vm, int argc, char** argv))
{
    if (vm != NULL)
    {
        WrenchContext* context = (WrenchContext*)wrenGetUserData(vm);
        wrench_assert(context != NULL);

        return wrenchSetCommandLine(context, argc, argv);
    }
    else
    {
        return false;
    }
}

WRENCH_IMPL(const char*, GetModuleSource, (WrenVM* vm, const char* name))
{
    if (vm != NULL)
    {
        WrenchContext* context = (WrenchContext*)wrenGetUserData(vm);
        wrench_assert(context != NULL);

        return wrenchGetModuleSource(context, name);
    }
    else
    {
        return "";
    }
}

WRENCH_IMPL(const char*, GetBasePath, (WrenVM* vm))
{
    if (vm != NULL)
    {
        WrenchContext* context = (WrenchContext*)wrenGetUserData(vm);
        wrench_assert(context != NULL);

        return wrenchGetBasePath(context);
    }
    else
    {
        return "./";
    }
}

WRENCH_IMPL(bool, SetBasePath, (WrenVM* vm, const char* path))
{
    if (vm != NULL)
    {
        WrenchContext* context = (WrenchContext*)wrenGetUserData(vm);
        wrench_assert(context != NULL);

        return wrenchSetBasePath(context, path);
    }
    else
    {
        return false;
    }
}

WRENCH_IMPL(wrenFileReadFn, GetFileReadCallback, (WrenVM* vm))
{
    if (vm != NULL)
    {
        WrenchContext* context = (WrenchContext*)wrenGetUserData(vm);
        wrench_assert(context != NULL);

        return wrenchGetFileReadCallback(context);
    }
    else
    {
        return NULL;
    }
}

WRENCH_IMPL(void, SetFileReadCallback, (WrenVM* vm, wrenFileReadFn callback))
{
    if (vm == NULL)
    {
        return;
    }

    WrenchContext* context = (WrenchContext*)wrenGetUserData(vm);
    wrench_assert(context != NULL);

    wrenchSetFileReadCallback(context, callback);
}

WRENCH_IMPL(wrenFileFreeFn, GetFileFreeCallback, (WrenVM* vm))
{
    if (vm != NULL)
    {
        WrenchContext* context = (WrenchContext*)wrenGetUserData(vm);
        wrench_assert(context != NULL);

        return wrenchGetFileFreeCallback(context);
    }
    else
    {
        return NULL;
    }
}

WRENCH_IMPL(void, SetFileFreeCallback, (WrenVM* vm, wrenFileFreeFn callback))
{
    if (vm == NULL)
    {
        return;
    }

    WrenchContext* context = (WrenchContext*)wrenGetUserData(vm);
    wrench_assert(context != NULL);

    wrenchSetFileFreeCallback(context, callback);
}

WRENCH_IMPL(void*, DefaultFileRead, (WrenVM* vm, const char* name, size_t* file_size))
{
    char path[1024 * 4], error[1024 * 4];

    if (wrench_snprintf(path, sizeof(path), "%s%s.wren", wrenGetBasePath(vm), name) < 0)
    {
        // TODO: Handle truncation error.
    }

    FILE* file = wrench_fopen((const char*)path, "rb");

    if (file == NULL)
    {
        wrench_snprintf(error, sizeof(error), "Source file \"%s\" not found.", (const char*)path);
        wrenSetErrorString(vm, (const char*)error);

        return NULL;
    }

    if (wrench_fseek(file, 0, SEEK_END) != 0)
    {
        wrench_snprintf(error, sizeof(error), "Failed to get size of source file \"%s\".", (const char*)path);
        wrenSetErrorString(vm, (const char*)error);

        wrench_fclose(file);
        return NULL;
    }

    *file_size = (size_t)wrench_ftell(file);

    if (wrench_fseek(file, 0, SEEK_SET) != 0)
    {
        wrench_snprintf(error, sizeof(error), "Failed to rewind source file \"%s\".", (const char*)path);
        wrenSetErrorString(vm, (const char*)error);

        wrench_fclose(file);
        return NULL;
    }

    void* file_data = wrench_malloc(*file_size);

    if (file_data == NULL)
    {
        wrench_snprintf(error, sizeof(error), "Failed to allocate space for source file \"%s\".", (const char*)path);
        wrenSetErrorString(vm, (const char*)error);

        wrench_fclose(file);
        return NULL;
    }

    if (wrench_fread(file_data, *file_size, 1, file) != 1)
    {
        wrench_snprintf(error, sizeof(error), "Failed to read source file \"%s\".", (const char*)path);
        wrenSetErrorString(vm, (const char*)error);

        wrench_fclose(file);
        return NULL;
    }

    if (wrench_fclose(file) != 0)
    {
        wrench_snprintf(error, sizeof(error), "Failed to close source file \"%s\".", (const char*)path);
        wrenSetErrorString(vm, (const char*)error);

        return NULL;
    }

    return file_data;
}

WRENCH_IMPL(void, DefaultFileFree, (WrenVM* vm, void* data, size_t size))
{
    wrench_free(data);
}

WRENCH_IMPL(const char*, LoadSourceFile, (WrenVM* vm, const char* name, size_t* num_chars))
{
    if (vm != NULL)
    {
        WrenchContext* context = (WrenchContext*)wrenGetUserData(vm);
        wrench_assert(context != NULL);

        return wrenchLoadSourceFile(context, name, num_chars);
    }
    else
    {
        return NULL;
    }
}

WRENCH_IMPL(bool, BeginModule, (WrenVM* vm, const char* moduleName))
{
    if (vm != NULL)
    {
        WrenchContext* context = (WrenchContext*)wrenGetUserData(vm);
        wrench_assert(context != NULL);

        return wrenchBeginModule(context, moduleName);
    }
    else
    {
        return false;
    }
}

WRENCH_IMPL(bool, CodeEx, (WrenVM* vm, const char* source, size_t num_chars))
{
    if (vm != NULL)
    {
        WrenchContext* context = (WrenchContext*)wrenGetUserData(vm);
        wrench_assert(context != NULL);

        return wrenchCodeEx(context, source, num_chars);
    }
    else
    {
        return false;
    }
}

WRENCH_IMPL(bool, Code, (WrenVM* vm, const char* source))
{
    if (vm != NULL)
    {
        WrenchContext* context = (WrenchContext*)wrenGetUserData(vm);
        wrench_assert(context != NULL);

        return wrenchCode(context, source);
    }
    else
    {
        return false;
    }
}

WRENCH_IMPL(bool, EndModule, (WrenVM* vm))
{
    if (vm != NULL)
    {
        WrenchContext* context = (WrenchContext*)wrenGetUserData(vm);
        wrench_assert(context != NULL);

        return wrenchEndModule(context);
    }
    else
    {
        return false;
    }
}

WRENCH_IMPL(bool, RegisterModuleEx, (WrenVM* vm, const char* moduleName, const char* source, size_t num_chars, bool copy_source))
{
    if (vm != NULL)
    {
        WrenchContext* context = (WrenchContext*)wrenGetUserData(vm);
        wrench_assert(context != NULL);

        return wrenchRegisterModuleEx(context, moduleName, source, num_chars, copy_source);
    }
    else
    {
        return false;
    }
}

WRENCH_IMPL(bool, RegisterModule, (WrenVM* vm, const char* moduleName, const char* source))
{
    if (vm != NULL)
    {
        WrenchContext* context = (WrenchContext*)wrenGetUserData(vm);
        wrench_assert(context != NULL);

        return wrenchRegisterModule(context, moduleName, source);
    }
    else
    {
        return false;
    }
}

WRENCH_IMPL(bool, RegisterClass, (WrenVM* vm, const char* moduleName, const char* className, WrenForeignMethodFn ctor, WrenFinalizerFn dtor))
{
    if (vm != NULL)
    {
        WrenchContext* context = (WrenchContext*)wrenGetUserData(vm);
        wrench_assert(context != NULL);

        return wrenchRegisterClass(context, moduleName, className, ctor, dtor);
    }
    else
    {
        return false;
    }
}

WRENCH_IMPL(bool, RegisterMethod, (WrenVM* vm, const char* moduleName, const char* className, bool isStatic, const char* signature, WrenForeignMethodFn method))
{
    if (vm != NULL)
    {
        WrenchContext* context = (WrenchContext*)wrenGetUserData(vm);
        wrench_assert(context != NULL);

        return wrenchRegisterMethod(context, moduleName, className, isStatic, signature, method);
    }
    else
    {
        return false;
    }
}

WRENCH_IMPL(float, GetSlotFloat, (WrenVM* vm, int slot))
{
    const double value = wrenGetSlotDouble(vm, slot);

    /* TODO: Check for NaN.
     * TODO: Check for Inf.
     */
    wrench_assert(WRENCH_NUM_IS_SAFE_FLT(value));
    return (float)value;
}

WRENCH_IMPL(void, SetSlotFloat, (WrenVM* vm, int slot, float value))
{
    wrenSetSlotDouble(vm, slot, (double)value);
}

WRENCH_IMPL(int, GetSlotInt, (WrenVM* vm, int slot))
{
    switch (wrenGetSlotType(vm, slot))
    {
        case WREN_TYPE_NUM:
        {
            const double value = wrenGetSlotDouble(vm, slot);

            /* TODO: Check for NaN.
             * TODO: Check for Inf.
             */
            wrench_assert(WRENCH_NUM_IS_INT(value));
            wrench_assert(WRENCH_NUM_IS_SAFE_INT(value));

            return WRENCH_NUM_TO_INT(value);
        }
        break;

        case WREN_TYPE_BOOL:
        {
            return (int)wrenGetSlotBool(vm, slot);
        }
        break;

        default:
        {
            wrench_assert(0);
        }
        break;
    }

    return 0;
}

WRENCH_IMPL(void, SetSlotInt, (WrenVM* vm, int slot, int value))
{
    // This will always be within the range of a signed 32-bit int.
    wrenSetSlotDouble(vm, slot, (double)value);
}

WRENCH_IMPL(uint8_t, GetSlotByte, (WrenVM* vm, int slot))
{
    const int value = wrenGetSlotInt(vm, slot);

    wrench_assert(value >= 0 && value <= UINT8_MAX);
    return (uint8_t)value;
}

WRENCH_IMPL(void, SetSlotByte, (WrenVM* vm, int slot, uint8_t value))
{
    wrenSetSlotInt(vm, slot, (int)value);
}

WRENCH_IMPL(void*, DefaultReallocate, (void* ptr, size_t newSize, void* userData))
{
    // TODO: Put a fixed-size small-block allocator in front of this.

    if (newSize == 0)
    {
        wrench_free(ptr);
        return NULL;
    }

    return wrench_realloc(ptr, newSize);
}

WRENCH_IMPL(const char*, DefaultResolveModule, (WrenVM* vm, const char* importer, const char* name))
{
    WRENCH_TEMP(); return name;
}

WRENCH_IMPL(WrenLoadModuleResult, DefaultLoadModule, (WrenVM* vm, const char* name))
{
    WrenLoadModuleResult result = {};

    if (wrench_strcmp(name, "meta") == 0 || wrench_strcmp(name, "random") == 0)
    {
        return result;
    }

    WrenchContext* context = (WrenchContext*)wrenGetUserData(vm);
    wrench_assert(context != NULL);

    void* library = wrenchLoadLibrary(context, name);

    if (library != NULL)
    {
        char init_name[1024];
        wrench_snprintf(init_name, sizeof(init_name), "%sWrenInit", name);

        wrenLibraryInitFn init = ( // TODO: Should lib initialization be able to fail?
        wrenLibraryInitFn)wrenchLibraryFunc(context, library, (const char*)init_name);

        if (init != NULL)
        {
            if (!init(vm))
            {
                wrenchFreeLibrary(context, library);
                return result;
            }
        }
    }

    WrenchModule* module = wrenchGetModule(context, name, NULL);

    if (module != NULL)
    {
        module->library = library;
        result.source = module->source;

        if (result.source != NULL)
        {
            return result;
        }
    }

    size_t num_chars;
    result.source = wrenchLoadSourceFile(context, name, &num_chars);

    if (result.source == NULL)
    {
        return result;
    }

    if (module == NULL)
    {
        if (!wrenchRegisterModuleEx(context, name, result.source, num_chars, false))
        {
            // TODO: Should we do something if this fails? Set result.source = NULL?
            return result;
        }
    }

    return result;
}

WRENCH_IMPL(WrenForeignMethodFn, DefaultBindForeignMethod, (WrenVM* vm, const char* moduleName, const char* className, bool is_static, const char* signature))
{
    if (wrench_strcmp(moduleName, "meta") == 0 || wrench_strcmp(moduleName, "random") == 0)
    {
        return NULL;
    }

    WrenchContext* context = (WrenchContext*)wrenGetUserData(vm);
    wrench_assert(context != NULL);

    WrenchModule* previous_module;
    WrenchModule* module = wrenchGetModule(context, moduleName, &previous_module);

    wrench_assert(module != NULL);

    WrenchClass* previous_class;
    WrenchClass* klass = wrenchGetClass(context, module, className, &previous_class);

    wrench_assert(klass != NULL);

    WrenchMethod* previous_method;
    WrenchMethod* method = wrenchGetMethod(context, klass, is_static, signature, &previous_method);

    wrench_assert(method != NULL);

    WrenForeignMethodFn function = method->method;
    wrench_assert(function != NULL);

    if (1) // Unlink to reduce search space for the next bind operation.
    {
        wrenchUnlinkMethod(context, klass, method, previous_method);

        if (klass->method_head == NULL)
        {
            wrenchUnlinkClass(context, module, klass, previous_class);

            // Don't unlink modules (so wrenGetModuleSource may work).
            if (0 && module->class_head == NULL)
            {
                wrenchUnlinkModule(context, module, previous_module);
            }
        }
    }

    return function;
}

WRENCH_IMPL(WrenForeignClassMethods, DefaultBindForeignClass, (WrenVM* vm, const char* moduleName, const char* className))
{
    WrenForeignClassMethods methods = {};

    if (wrench_strcmp(moduleName, "meta") == 0 || wrench_strcmp(moduleName, "random") == 0)
    {
        return methods;
    }

    WrenchContext* context = (WrenchContext*)wrenGetUserData(vm);
    wrench_assert(context != NULL);

    WrenchModule* module = wrenchGetModule(context, moduleName, NULL);
    wrench_assert(module != NULL);

    WrenchClass* klass = wrenchGetClass(context, module, className, NULL);
    wrench_assert(klass != NULL);

    methods.allocate = klass->ctor;
    methods.finalize = klass->dtor;

    return methods;
}

WRENCH_IMPL(void, DefaultWrite, (WrenVM* vm, const char* text))
{
    wrench_fprintf(wrench_stdout, "%s", text);
}

WRENCH_IMPL(void, DefaultError, (WrenVM* vm, WrenErrorType type, const char* moduleName, int line, const char* message))
{
    // TODO: wrenSetErrorString?

    switch (type)
    {
        case WREN_ERROR_COMPILE:
        {
            wrench_fprintf(wrench_stderr, "COMPILE ERROR [%s line %d]: %s\n", moduleName, line, message);
        }
        break;

        case WREN_ERROR_RUNTIME:
        {
            wrench_fprintf(wrench_stderr, "RUNTIME ERROR: %s\n", message);
        }
        break;

        case WREN_ERROR_STACK_TRACE:
        {
            wrench_fprintf(wrench_stderr, "[%s line %d] in %s\n", moduleName, line, message);
        }
        break;
    }
}

#if defined(WRENCH_MAIN)

int WRENCH_MAIN(int argc, char** argv)
{
    if (argc < 2)
    {
        wrench_fprintf(wrench_stderr, "Usage: %s main_wren_filename\n", argv[0]);
        return EXIT_SUCCESS;
    }

    WrenVM* vm = wrenNewExtendedVM(argc, argv, true);

    if (vm == NULL)
    {
        wrench_fprintf(wrench_stderr, "Failed to create a Wren VM!\n");
        return EXIT_FAILURE;
    }

    WrenInterpretResult result;

    if (wrench_strstr(argv[1], ".wren") != NULL)
    {
        result = wrenInterpret(vm, "main", wrenLoadSourceFile(vm, argv[1], NULL));
    }
    else
    {
        char code[1024]; // Import for native code modules or scripts.
        wrench_snprintf(code, sizeof(code), "import \"%s\"", argv[1]);

        result = wrenInterpret(vm, "main", (const char*)code);
    }

    switch (result)
    {
        case WREN_RESULT_COMPILE_ERROR:
        case WREN_RESULT_RUNTIME_ERROR:
        {
            //wrench_fprintf(wrench_stderr, "%s\n", wrenGetErrorString(vm));
            //wrenFreeExtendedVM(vm);

            return EXIT_FAILURE;
        }
        break;

        default: break;
    }

    wrenFreeExtendedVM(vm, true);
    return EXIT_SUCCESS;
}

#endif /* WRENCH_MAIN */
#endif /* __WRENCH_C__ */
#endif /* WRENCH_IMPLEMENTATION */
