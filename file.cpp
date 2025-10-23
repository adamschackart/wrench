/* -----------------------------------------------------------------------------
--- Copyright (c) 2012-2026 Adam Schackart / "AJ Hackman", all rights reserved.
--- Distributed under the BSD license v2 (opensource.org/licenses/BSD-3-Clause)
----------------------------------------------------------------------------- */
#include <filesystem>

#define WRENCH_IMPLEMENTATION
#include <wrench.h>

/*
================================================================================
 * ~~ [ path ] ~~ *
--------------------------------------------------------------------------------
*/

static void path_list(WrenVM* vm)
{
    const char* path = wrenGetSlotString(vm, 1);
    const bool recursive = wrenGetSlotBool(vm, 2);
    const bool include_subdirectories = wrenGetSlotBool(vm, 3);

    const std::filesystem::path p{path};

    // TODO: Abort if directory doesn't exist, or empty list?
    wrenSetSlotNewList(vm, 0);

    #define ENTRY() do                                                      \
    {                                                                       \
        if (include_subdirectories == false && dir_entry.is_directory())    \
        {                                                                   \
            continue;                                                       \
        }                                                                   \
                                                                            \
        wrenSetSlotString(vm, 1, dir_entry.path().c_str());                 \
        wrenInsertInList(vm, 0, -1, 1);                                     \
    }                                                                       \
    while (0)

    if (recursive)
    {
        for (auto const& dir_entry : std::filesystem::recursive_directory_iterator{p})
        {
            ENTRY();
        }
    }
    else
    {
        for (auto const& dir_entry : std::filesystem::directory_iterator{p})
        {
            ENTRY();
        }
    }

    #undef ENTRY
}

/*
================================================================================
 * ~~ [ file ] ~~ *
--------------------------------------------------------------------------------
*/

typedef struct File
{
    WRENCH_MAGIC_TAG;
    FILE* file;
}
File;

static void file_ctor(WrenVM* vm)
{
    WRENCH_STUB();
}

static void file_dtor(void* data)
{
    WRENCH_CHECK_MAGIC_TAG(data, file, File);
    FILE* file = ((File*)data)->file;

    if (file != NULL) { fclose(file); }
}

static void file_open(WrenVM* vm)
{
    const char* path = wrenGetSlotString(vm, 1);
    const char* mode = wrenGetSlotString(vm, 2);

    FILE* file = fopen(path, mode);

    if (file != NULL)
    {
        File* data = (File*)wrenSetSlotNewForeign(vm, 0, 0, sizeof(File));
        WRENCH_SET_MAGIC_TAG(data, file, File);

        data->file = file;
    }
    else
    {
        char error[1024];
        wrench_snprintf(error, sizeof(error), "failed to open file \"%s\" with mode \"%s\"", path, mode);

        wrenSetSlotString(vm, 0, (const char*)error);
        wrenAbortFiber(vm, 0);
    }
}

static void file_close(WrenVM* vm)
{
    File* self = (File*)wrenGetSlotForeign(vm, 0);
    WRENCH_CHECK_MAGIC_TAG(self, file, File);

    if (fclose(self->file) != 0)
    {
        /* TODO: Keep/copy the name and mode of the file.
         */
        wrenSetSlotString(vm, 0, "failed to close file");
        wrenAbortFiber(vm, 0);
    }
}

static void file_stdout(WrenVM* vm)
{
    File* data = (File*)wrenSetSlotNewForeign(vm, 0, 0, sizeof(File));
    WRENCH_SET_MAGIC_TAG(data, file, File);

    data->file = stdout;
}

static void file_stderr(WrenVM* vm)
{
    File* data = (File*)wrenSetSlotNewForeign(vm, 0, 0, sizeof(File));
    WRENCH_SET_MAGIC_TAG(data, file, File);

    data->file = stderr;
}

static void file_stdin(WrenVM* vm)
{
    File* data = (File*)wrenSetSlotNewForeign(vm, 0, 0, sizeof(File));
    WRENCH_SET_MAGIC_TAG(data, file, File);

    data->file = stdin;
}

static void file_getc(WrenVM* vm)
{
    File* self = (File*)wrenGetSlotForeign(vm, 0);
    WRENCH_CHECK_MAGIC_TAG(self, file, File);

    wrenSetSlotInt(vm, 0, getc(self->file));
}

static void file_putc(WrenVM* vm)
{
    File* self = (File*)wrenGetSlotForeign(vm, 0);
    WRENCH_CHECK_MAGIC_TAG(self, file, File);

    switch (wrenGetSlotType(vm, 1))
    {
        case WREN_TYPE_NUM:
        {
            wrenSetSlotInt(vm, 0, putc(wrenGetSlotInt(vm, 1), self->file));
        }
        break;

        case WREN_TYPE_STRING:
        {
            const char* s = wrenGetSlotString(vm, 1);

            if (s[0] != '\0')
            {
                wrench_assert(s[1] == '\0', "multi-char string \"%s\"", s);
                wrenSetSlotInt(vm, 0, putc(s[0], self->file));
            }
            else
            {
                wrenSetSlotInt(vm, 0, 0);
            }
        }
        break;

        default:
        {
            wrenSetSlotString(vm, 0, "Invalid type for argument 1 of File.putc");
            wrenAbortFiber(vm, 0);
        }
        break;
    }
}

static void file_EOF(WrenVM* vm)
{
    wrenSetSlotInt(vm, 0, EOF);
}

static void file_eof(WrenVM* vm)
{
    File* self = (File*)wrenGetSlotForeign(vm, 0);
    WRENCH_CHECK_MAGIC_TAG(self, file, File);

    wrenSetSlotBool(vm, 0, feof(self->file) != 0);
}

static void file_flush(WrenVM* vm)
{
    File* self = (File*)wrenGetSlotForeign(vm, 0);
    WRENCH_CHECK_MAGIC_TAG(self, file, File);

    if (fflush(self->file) != 0)
    {
        /* TODO: Keep/copy the name and mode of the file.
         */
        wrenSetSlotString(vm, 0, "failed to flush file");
        wrenAbortFiber(vm, 0);
    }
}

/*
================================================================================
 * ~~ [ (un)hook ] ~~ *
--------------------------------------------------------------------------------
*/

WRENCH_EXPORT bool fileWrenInit(WrenVM* vm)
{
    if (!wrenRegisterModule(vm, "file", NULL)) { return false; } else
    {
        if (!wrenRegisterClass(vm, "file", "Path", NULL, NULL)) { return false; } else
        {
            if (!wrenRegisterMethod(vm, "file", "Path", true, "list(_,_,_)", path_list)) return false;
        }

        if (!wrenRegisterClass(vm, "file", "File", file_ctor, file_dtor)) { return false; } else
        {
            if (!wrenRegisterMethod(vm, "file", "File", true, "open(_,_)", file_open)) return false;
            if (!wrenRegisterMethod(vm, "file", "File", false, "close()", file_close)) return false;

            if (!wrenRegisterMethod(vm, "file", "File", true, "stdout", file_stdout)) return false;
            if (!wrenRegisterMethod(vm, "file", "File", true, "stderr", file_stderr)) return false;
            if (!wrenRegisterMethod(vm, "file", "File", true, "stdin", file_stdin)) return false;

            if (!wrenRegisterMethod(vm, "file", "File", false, "getc()", file_getc)) return false;
            if (!wrenRegisterMethod(vm, "file", "File", false, "putc(_)", file_putc)) return false;

            if (!wrenRegisterMethod(vm, "file", "File", true, "EOF", file_EOF)) return false;
            if (!wrenRegisterMethod(vm, "file", "File", false, "eof()", file_eof)) return false;

            if (!wrenRegisterMethod(vm, "file", "File", false, "flush()", file_flush)) return false;
        }
    }

    return true;
}

WRENCH_EXPORT void fileWrenQuit(void)
{
    //
}
