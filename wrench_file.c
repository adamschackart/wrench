/* -----------------------------------------------------------------------------
--- Copyright (c) 2012-2026 Adam Schackart / "AJ Hackman", all rights reserved.
--- Distributed under the BSD license v2 (opensource.org/licenses/BSD-3-Clause)
----------------------------------------------------------------------------- */

#ifndef WRENCH_IMPLEMENTATION
#define WRENCH_IMPLEMENTATION 1
#endif
#include <wrench_file.h>

#include <tinydir/tinydir.h>

/*
================================================================================
 * ~~ [ path ] ~~ *
--------------------------------------------------------------------------------
*/

static void file_Path_exists(WrenVM* vm)
{
    const char* filename = wrenGetSlotString(vm, 1);

    #if _WIN32
    {
        DWORD dwAttrib = GetFileAttributesA(filename);

        #if 0
        {
            if (dwAttrib != INVALID_FILE_ATTRIBUTES &&
                dwAttrib & FILE_ATTRIBUTE_DIRECTORY)
            {
                wrenSetSlotBool(vm, 0, true);
            }
            else
            {
                wrenSetSlotBool(vm, 0, PathFileExistsA(filename));
            }
        }
        #else
        {
            wrenSetSlotBool(vm, 0, dwAttrib != INVALID_FILE_ATTRIBUTES);
        }
        #endif
    }
    #else
    {
        wrenSetSlotBool(vm, 0, access(filename, F_OK) == 0);
    }
    #endif
}

static void file_Path_isDirectory(WrenVM* vm)
{
    const char* path = wrenGetSlotString(vm, 1);

    #if _WIN32
    {
        #if 1
        {
            const DWORD dwAttrib = GetFileAttributesA(path);

            wrenSetSlotBool(vm, 0, (dwAttrib != INVALID_FILE_ATTRIBUTES &&
                                    dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
        }
        #else
        {
            wrenSetSlotBool(vm, 0, PathIsDirectoryA(path));
        }
        #endif
    }
    #else
    {
        struct stat statbuf;

        if (stat(path, &statbuf) != 0)
        {
            wrenSetSlotBool(vm, 0, false);
        }
        else
        {
            wrenSetSlotBool(vm, 0, S_ISDIR(statbuf.st_mode));
        }
    }
    #endif
}

static void file_Path_isFile(WrenVM* vm)
{
    const char* path = wrenGetSlotString(vm, 1);

    #if _WIN32
    {
        const DWORD dwAttrib = GetFileAttributesA(path);

        wrenSetSlotBool(vm, 0, (dwAttrib != INVALID_FILE_ATTRIBUTES && \
                            ((dwAttrib & FILE_ATTRIBUTE_NORMAL) != 0 ||
                            (dwAttrib & FILE_ATTRIBUTE_ARCHIVE) != 0)));
    }
    #else
    {
        struct stat statbuf;

        if (stat(path, &statbuf) != 0)
        {
            wrenSetSlotBool(vm, 0, false);
        }
        else
        {
            wrenSetSlotBool(vm, 0, S_ISREG(statbuf.st_mode));
        }
    }
    #endif
}

static void file_Path_remove(WrenVM* vm)
{
    const char* path = wrenGetSlotString(vm, 1);

    if (remove(path) != 0)
    {
        char error[1024 * 4];
        wrench_snprintf(error, sizeof(error), "Failed to remove file \"%s\".", path);

        wrenSetSlotString(vm, 0, (const char*)error);
        wrenAbortFiber(vm, 0);
    }
}

static void file_Path_resolve(WrenVM* vm)
{
    const char* path = wrenGetSlotString(vm, 1);
    char out[1024 * 4];

    #if _WIN32
    {
        DWORD length = GetFullPathNameA(path, 0xFFFFFFFF, out, NULL);

        if (length == 0)
        {
            wrenSetSlotString(vm, 0, "GetFullPathNameA failed");
            wrenAbortFiber(vm, 0);

            return;
        }

        const DWORD dwAttrib = GetFileAttributesA((const char*)out);

        const bool is_directory = (dwAttrib != INVALID_FILE_ATTRIBUTES &&
                                    dwAttrib & FILE_ATTRIBUTE_DIRECTORY);

        if (is_directory && out[length - 1] != '\\')
        {
            out[length] = '\\';
            out[length + 1] = '\0';
        }
    }
    #else
    {
        const char* r = (const char*)wrench_realpath(path, out);

        if (r == NULL)
        {
            wrenSetSlotString(vm, 0, "realpath failed");
            wrenAbortFiber(vm, 0);

            return;
        }

        struct stat statbuf;

        if (wrench_stat(r, &statbuf) != 0)
        {
            wrenSetSlotString(vm, 0, "stat failed");
            wrenAbortFiber(vm, 0);

            return;
        }

        const bool is_directory = S_ISDIR(statbuf.st_mode);

        if (is_directory)
        {
            const size_t length = wrench_strlen(r);

            if (out[length - 1] != '/')
            {
                out[length] = '/';
                out[length + 1] = '\0';
            }
        }
    }
    #endif

    wrenSetSlotString(vm, 0, (const char*)out);
}

static void file_Path_list_entry(WrenVM* vm, const char* path, bool recursive, bool include_subdirectories)
{
    tinydir_dir dir;

    if (tinydir_open_sorted(&dir, path) < 0)
    {
        char error[1024 * 4];
        wrench_snprintf(error, sizeof(error), "Failed to list path \"%s\".", path);

        wrenSetSlotString(vm, 0, (const char*)error);
        wrenAbortFiber(vm, 0);

        return;
    }

    for (size_t i = 0; i < dir.n_files; i++)
    {
        tinydir_file file;
        tinydir_readfile_n(&dir, &file, i);

        if (wrench_strcmp(file.name, ".") == 0 || wrench_strcmp(file.name, "..") == 0)
        {
            continue;
        }

        if (file.is_dir)
        {
            if (recursive)
            {
                file_Path_list_entry(vm, file.path, recursive, include_subdirectories);
            }

            if (!include_subdirectories)
            {
                continue;
            }
        }

        wrenSetSlotString(vm, 1, file.path);
        wrenInsertInList(vm, 0, -1, 1);
    }

    tinydir_close(&dir);
}

static void file_Path_list(WrenVM* vm)
{
    const char* path = wrenGetSlotString(vm, 1);
    bool recursive = wrenGetSlotBool(vm, 2);
    bool include_subdirectories = wrenGetSlotBool(vm, 3);

    wrenSetSlotNewList(vm, 0);
    file_Path_list_entry(vm, path, recursive, include_subdirectories);
}

/*
================================================================================
 * ~~ [ file ] ~~ *
--------------------------------------------------------------------------------
*/

static void file_File_ctor(WrenVM* vm)
{
    WRENCH_STUB();
}

static void file_File_dtor(void* data)
{
    WRENCH_CHECK_MAGIC_TAG(data, file, File);
    file_File* self = (file_File*)data;

    if (self->collect && self->file != NULL)
    {
        fclose(self->file);
    }
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

        data->collect = true;
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

    self->file = NULL;
}

static bool file_File_ensureCRLF = false;

static void file_File_ensureCRLF_get(WrenVM* vm)
{
    wrenSetSlotBool(vm, 0, file_File_ensureCRLF);
}

static void file_File_ensureCRLF_set(WrenVM* vm)
{
    file_File_ensureCRLF = wrenGetSlotBool(vm, 1);
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

/* TODO: Better error handling.
 */
static void file_File_getc(WrenVM* vm)
{
    file_File* self = (file_File*)wrenGetSlotForeign(vm, 0);
    WRENCH_CHECK_MAGIC_TAG(self, file, File);

    wrenSetSlotInt(vm, 0, getc(self->file));
}

/* TODO: Better error handling.
 */
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

            #if !_WIN32
                if (s[0] == '\n' && file_File_ensureCRLF)
                {
                    wrenSetSlotInt(vm, 0, putc('\r', self->file) + putc('\n', self->file));
                }
                else
            #endif
                {
                    wrenSetSlotInt(vm, 0, putc(s[0], self->file));
                }
            }
            else
            {
                wrenSetSlotInt(vm, 0, 0);
            }
        }
        break;

        default:
        {
            wrenSetSlotString(vm, 0, "Invalid type for arg 1 of File.putc");
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

static void file_File_read(WrenVM* vm)
{
    WRENCH_STUB();
}

/* TODO: Better error handling.
 */
static void file_File_write_(WrenVM* vm)
{
    file_File* self = (file_File*)wrenGetSlotForeign(vm, 0);
    WRENCH_CHECK_MAGIC_TAG(self, file, File);

    int size;
    const char* data = wrenGetSlotBytes(vm, 1, &size);

    wrenSetSlotInt(vm, 0, fwrite(data, 1, size, self->file));
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

#if WRENCH_FILE_EXTENDED
    /*
     * Enable user extension of stdlib modules.
     */
    #ifndef __FILE_EX_INL__
    #include <file_ex.inl>
    #endif
#else
    static bool fileWrenInitEx(WrenVM* vm)
    {
        return true;
    }

    static void fileWrenQuitEx(void)
    {
        //
    }

    static bool filePathWrenInitEx(WrenVM* vm)
    {
        return true;
    }

    static bool fileFileWrenInitEx(WrenVM* vm)
    {
        return true;
    }
#endif /* WRENCH_FILE_EXTENDED */

#undef stdin
#undef stdout
#undef stderr

WRENCH_EXPORT bool fileWrenInit(WrenVM* vm)
{
    if (!wrenBeginModule(vm, "file")) { return false; } else
    {
        WREN_BEGIN_CLASS_EX(file, Path, NULL, NULL);
        {
            if (0)
            {
                // XXX FIXME: This only works on files, not directories.
                WREN_METHOD(file, Path, true, exists, "(path)", "(_)");
            }
            else
            {
                WREN_CODE("static exists(path) { isDirectory(path) || isFile(path) }");
            }

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

            WREN_METHOD(file, Path, true, isDirectory, "(path)", "(_)");
            WREN_METHOD(file, Path, true, isFile, "(path)", "(_)");

            // TODO: createDirectory
            // TODO: createFile
            // TODO: copyFile
            // TODO: moveFile

            WREN_METHOD(file, Path, true, remove, "(path)", "(_)");

            if (!wrenCode(vm,

            "static tryRemove(path) {\n"
                "var fiber = Fiber.new {\n"
                    "remove(path)\n"
                "}\n"

                "return fiber.try()\n"
            "}\n"

            )) { return false; }

            WREN_METHOD(file, Path, true, resolve, "(path)", "(_)");

            // TODO: parent (resolve "path/..", ensuring possible filename is stripped off the end)

            WREN_METHOD(file, Path, true, list, "(path, recursive, include_subdirectories)", "(_,_,_)");
            WREN_CODE("static list(path, recursive) { list(path, recursive, true) }");
            WREN_CODE("static list(path) { list(path, false, true) }");
            WREN_CODE("static walk(path) { list(path, true, true) }");

            if (!filePathWrenInitEx(vm))
            {
                return false;
            }
        }
        WREN_END_CLASS();

        WREN_BEGIN_CLASS(file, File);
        {
            WREN_METHOD(file, File, true, open, "(path, mode)", "(_,_)");
            WREN_METHOD(file, File, false, close, "()", "()");

            // TODO: collect
            // TODO: name
            // TODO: mode

            // TODO: toString

            // Write Windows-style line endings on Unix.
            WREN_PROPERTY(file, File, true, ensureCRLF);

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

            if (0)
            {
                WREN_METHOD(file, File, false, read, "(count)", "(_)");
            }
            else
            {
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

                )) { return false; }
            }

            WREN_METHOD(file, File, false, write_, "(string)", "(_)");

            if (!wrenCode(vm,

            "read() { read(Num.maxSafeInteger) }\n"

            "static read(path) {\n"
                "var file = open(path, \"rb\")\n"
                "var data = file.read()\n"

                "file.close()\n"
                "return data\n"
            "}\n"

            "write(string) {\n"
            #if _WIN32
                "if (true) {\n"
            #else
                "if (!type.ensureCRLF) {\n"
            #endif
                    "return write_(string)\n"
                "} else {\n"
                    "var r = 0\n"

                    "for (c in string) {\n"
                        "r = r + putc(c)\n"
                    "}\n"

                    "return r\n"
                "}\n"
            "}\n"

            )) { return false; }

            // TODO: seek
            // TODO: tell
            // TODO: size

            // TODO: bytesRemaining (ftell, fseek to end, fseek to saved pos, return difference)

            WREN_METHOD(file, File, false, flush, "()", "()");

            /* TODO: Native/foreign methods for performance.
             */
            if (!wrenCode(vm,

            "readLine(strip_newlines) {\n"
                "var s = []\n"

                "while (!eof()) {\n"
                    "s.insert(-1, read(1))\n"

                    "if (s[-1] == \"\\n\") {\n"
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

            if (!fileFileWrenInitEx(vm))
            {
                return false;
            }
        }
        WREN_END_CLASS();
    }

    if (!fileWrenInitEx(vm))
    {
        return false;
    }

    return wrenEndModule(vm);
}

WRENCH_EXPORT void fileWrenQuit(void)
{
    fileWrenQuitEx();
}
