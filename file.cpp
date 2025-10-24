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

static void file_Path_list(WrenVM* vm)
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

typedef struct file_File
{
    WRENCH_MAGIC_TAG;
    FILE* file;
}
file_File;

static void file_File_ctor(WrenVM* vm)
{
    WRENCH_STUB();
}

static void file_File_dtor(void* data)
{
    WRENCH_CHECK_MAGIC_TAG(data, file, File);
    FILE* file = ((file_File*)data)->file;

    if (file != NULL) { fclose(file); }
}

static void file_File_open(WrenVM* vm)
{
    const char* path = wrenGetSlotString(vm, 1);
    const char* mode = wrenGetSlotString(vm, 2);

    FILE* file = fopen(path, mode);

    if (file != NULL)
    {
        file_File* data = (file_File*)wrenSetSlotNewForeign(vm, 0, 0, sizeof(file_File));
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

static void file_File_close(WrenVM* vm)
{
    file_File* self = (file_File*)wrenGetSlotForeign(vm, 0);
    WRENCH_CHECK_MAGIC_TAG(self, file, File);

    if (fclose(self->file) != 0)
    {
        /* TODO: Keep/copy the name and mode of the file.
         */
        wrenSetSlotString(vm, 0, "failed to close file");
        wrenAbortFiber(vm, 0);
    }
}

static void file_File_stdout(WrenVM* vm)
{
    file_File* data = (file_File*)wrenSetSlotNewForeign(vm, 0, 0, sizeof(file_File));
    WRENCH_SET_MAGIC_TAG(data, file, File);

    data->file = stdout;
}

static void file_File_stderr(WrenVM* vm)
{
    file_File* data = (file_File*)wrenSetSlotNewForeign(vm, 0, 0, sizeof(file_File));
    WRENCH_SET_MAGIC_TAG(data, file, File);

    data->file = stderr;
}

static void file_File_stdin(WrenVM* vm)
{
    file_File* data = (file_File*)wrenSetSlotNewForeign(vm, 0, 0, sizeof(file_File));
    WRENCH_SET_MAGIC_TAG(data, file, File);

    data->file = stdin;
}

static void file_File_getc(WrenVM* vm)
{
    file_File* self = (file_File*)wrenGetSlotForeign(vm, 0);
    WRENCH_CHECK_MAGIC_TAG(self, file, File);

    wrenSetSlotInt(vm, 0, getc(self->file));
}

static void file_File_putc(WrenVM* vm)
{
    file_File* self = (file_File*)wrenGetSlotForeign(vm, 0);
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

static void file_File_EOF(WrenVM* vm)
{
    wrenSetSlotInt(vm, 0, EOF);
}

static void file_File_eof(WrenVM* vm)
{
    file_File* self = (file_File*)wrenGetSlotForeign(vm, 0);
    WRENCH_CHECK_MAGIC_TAG(self, file, File);

    wrenSetSlotBool(vm, 0, feof(self->file) != 0);
}

static void file_File_flush(WrenVM* vm)
{
    file_File* self = (file_File*)wrenGetSlotForeign(vm, 0);
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
    if (!wrenBeginModule(vm, "file")) { return false; } else
    {
        WREN_BEGIN_CLASS_EX(file, Path, NULL, NULL);
        {
            // TODO: exists
            // TODO: current
            // TODO: base

            // TODO: home
            // TODO: desktop
            // TODO: documents
            // TODO: downloads
            // TODO: music
            // TODO: pictures
            // TODO: public_share
            // TODO: saved_games
            // TODO: screenshots
            // TODO: templates
            // TODO: videos

            // TODO: path
            // TODO: fileName
            // TODO: extension

            // TODO: split
            // TODO: join

            // TODO: isDirectory
            // TODO: isFile

            // TODO: createDirectory
            // TODO: createFile
            // TODO: copyFile
            // TODO: moveFile
            // TODO: deleteFile

            WREN_METHOD(file, Path, true, list, "(path, recursive, include_subdirectories)", "(_,_,_)");
            WREN_CODE("static list(path, recursive) { list(path, recursive, true) }");
            WREN_CODE("static list(path) { list(path, false, true) }");
            WREN_CODE("static walk(path) { list(path, true, true) }");
        }
        WREN_END_CLASS();

        WREN_BEGIN_CLASS(file, File);
        {
            WREN_METHOD(file, File, true, open, "(path, mode)", "(_,_)");
            WREN_METHOD(file, File, false, close, "()", "()");

            // TODO: name
            // TODO: mode

            /* XXX: `stdout` et al. are #defined on most platforms, requiring a bit of a workaround here.
             */
            WREN_METHOD_EX(file, File, true, stdout, "", "", file_File_stdout);
            WREN_METHOD_EX(file, File, true, stderr, "", "", file_File_stderr);
            WREN_METHOD_EX(file, File, true, stdin, "", "", file_File_stdin);

            /* XXX: getc and putc are also macros (which is all that differentiates them from fgetc/fputc).
             */
            WREN_METHOD_EX(file, File, false, getc, "()", "()", file_File_getc);
            WREN_METHOD_EX(file, File, false, putc, "(c)", "(_)", file_File_putc);

            WREN_METHOD_EX(file, File, true, EOF, "", "", file_File_EOF);
            WREN_METHOD(file, File, false, eof, "()", "()");

            /* TODO: Native/foreign methods for performance.
             */
            if (!wrenCode(vm,

            "read(count) {\n"
                "var EOF = type.EOF\n"
                "var s = []\n"

                "for (i in 0...count) {\n"
                    "var c = getc()\n"

                    "if (c < 0 || c == EOF) {\n"
                        "break\n"
                    "} else {\n"
                        "s.insert(-1, String.fromByte(c))\n"
                    "}\n"
                "}\n"

                "return s.join()\n"
            "}\n"

            "read() { read(Num.maxSafeInteger) }\n"

            "static read(path) {\n"
                "var file = open(path, \"rb\")\n"
                "var data = file.read()\n"

                "file.close()\n"
                "return data\n"
            "}\n"

            )) { return false; }

            // TODO: write
            // TODO: seek
            // TODO: tell
            // TODO: size

            WREN_METHOD(file, File, false, flush, "()", "()");

            /* TODO: Native/foreign methods for performance.
             */
            if (!wrenCode(vm,

            "readLine(strip_newlines) {\n"
                "var s = []\n"

                "while (!eof()) {\n"
                    "s.insert(-1, read(1))\n"

                    "if (s[-1] == \"\n\") {\n"
                        "if (strip_newlines) {\n"
                            "s.removeAt(-1)\n"
                        "}\n"

                        "break\n"
                    "}\n"
                "}\n"

                "return s.join()\n"
            "}\n"

            "readLine() { readLine(true) }\n"

            "readLines(strip_newlines) {\n"
                "var s = []\n"

                "while (!eof()) {\n"
                    "s.insert(-1, readLine(strip_newlines))\n"
                "}\n"

                "return s\n"
            "}\n"

            "readLines() { readLines(true) }\n"

            "static readLines(path) {\n"
                "var file = open(path, \"rb\")\n"
                "var text = file.readLines()\n"

                "file.close()\n"
                "return text\n"
            "}\n"

            )) { return false; }
        }
        WREN_END_CLASS();
    }

    return wrenEndModule(vm);
}

WRENCH_EXPORT void fileWrenQuit(void)
{
    //
}
