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

static bool file_Path_isDirectory_impl(const char* path)
{
    #if _WIN32
    {
        #if 1
        {
            const DWORD dwAttrib = GetFileAttributesA(path);

            return (dwAttrib != INVALID_FILE_ATTRIBUTES && (
                (dwAttrib & FILE_ATTRIBUTE_DIRECTORY) != 0));
        }
        #else
        {
            return PathIsDirectoryA(path);
        }
        #endif
    }
    #else
    {
        struct stat statbuf;

        if (wrench_stat(path, &statbuf) != 0)
        {
            return false;
        }
        else
        {
            return S_ISDIR(statbuf.st_mode);
        }
    }
    #endif /* _WIN32 */
}

static bool file_Path_isFile_impl(const char* path)
{
    #if _WIN32
    {
        const DWORD dwAttrib = GetFileAttributesA(path);

        return (dwAttrib != INVALID_FILE_ATTRIBUTES && (
            (dwAttrib & FILE_ATTRIBUTE_NORMAL) != 0 ||
            (dwAttrib & FILE_ATTRIBUTE_ARCHIVE) != 0));
    }
    #else
    {
        struct stat statbuf;

        if (wrench_stat(path, &statbuf) != 0)
        {
            return false;
        }
        else
        {
            return S_ISREG(statbuf.st_mode);
        }
    }
    #endif /* _WIN32 */
}

static bool file_Path_exists_impl(const char* path)
{
    return file_Path_isDirectory_impl(path) || file_Path_isFile_impl(path);
}

#if _WIN32
    extern int _mkdir(const char *dirname);
#endif

static bool file_Path_createDirectory_impl(const char* path) // mkdir
{
    #if _WIN32
    {
        return _mkdir(path) == 0;
    }
    #else
    {
        return mkdir(path, 0777) == 0;
    }
    #endif
}

static bool file_Path_createFile_impl(const char* path) // touch
{
    // If already existing, don't change.
    FILE* file = wrench_fopen(path, "a");

    if (file != NULL)
    {
        return wrench_fclose(file) == 0;
    }
    else
    {
        return false;
    }
}

static bool file_Path_move_impl(const char* old_name, const char* new_name) // mv
{
    return wrench_rename(old_name, new_name) == 0;
}

static bool file_Path_remove_impl(const char* path) // rm
{
    return wrench_remove(path) == 0;
}

/* ===== [ wren ] =========================================================== */

static void file_Path_exists(WrenVM* vm)
{
    wrenSetSlotBool(vm, 0, file_Path_exists_impl(wrenGetSlotString(vm, 1)));
}

static void file_Path_isDirectory(WrenVM* vm)
{
    wrenSetSlotBool(vm, 0, file_Path_isDirectory_impl(wrenGetSlotString(vm, 1)));
}

static void file_Path_isFile(WrenVM* vm)
{
    wrenSetSlotBool(vm, 0, file_Path_isFile_impl(wrenGetSlotString(vm, 1)));
}

static void file_Path_temp(WrenVM* vm)
{
    #if _WIN32
    {
        char buffer[1024 * 4];

    #if 0
        if (GetTempPath2A(sizeof(buffer) - 1, buffer) == 0)
    #else
        if (GetTempPathA(sizeof(buffer) - 1, buffer) == 0)
    #endif
        {
            /* TODO: win32 error string function that calls/interprets GetLastError().
             */
            wrenSetSlotString(vm, 0, "GetTempPath failed");
            wrenAbortFiber(vm, 0);
        }
        else
        {
            wrenSetSlotString(vm, 0, (const char*)buffer);
        }
    }
    #else
    {
        /* This is what `std::filesystem::temp_directory_path` does.
         */
        char* path = wrench_getenv("TMPDIR");

        if (path != NULL)
        {
            wrenSetSlotString(vm, 0, (const char*)path);
            return;
        }

        path = wrench_getenv("TMP");

        if (path != NULL)
        {
            wrenSetSlotString(vm, 0, (const char*)path);
            return;
        }

        path = wrench_getenv("TEMP");

        if (path != NULL)
        {
            wrenSetSlotString(vm, 0, (const char*)path);
            return;
        }

        path = wrench_getenv("TEMPDIR");

        if (path != NULL)
        {
            wrenSetSlotString(vm, 0, (const char*)path);
            return;
        }

        #if __ANDROID__
        {
            wrenSetSlotString(vm, 0, "/data/local/tmp");
        }
        #else
        {
            wrenSetSlotString(vm, 0, "/tmp");
        }
        #endif
    }
    #endif
}

static void file_Path_createDirectory(WrenVM* vm)
{
    const char* path = wrenGetSlotString(vm, 1);

    if (!file_Path_createDirectory_impl(path))
    {
        char error[1024 * 4];
        wrench_snprintf(error, sizeof(error), "Failed to create directory \"%s\"", path);

        wrenSetSlotString(vm, 0, (const char*)error);
        wrenAbortFiber(vm, 0);
    }
}

static void file_Path_createFile(WrenVM* vm)
{
    const char* path = wrenGetSlotString(vm, 1);

    if (!file_Path_createFile_impl(path))
    {
        char error[1024 * 4];
        wrench_snprintf(error, sizeof(error), "Failed to create file \"%s\"", path);

        wrenSetSlotString(vm, 0, (const char*)error);
        wrenAbortFiber(vm, 0);
    }
}

static void file_Path_move(WrenVM* vm)
{
    const char* old_name = wrenGetSlotString(vm, 1);
    const char* new_name = wrenGetSlotString(vm, 2);

    if (!file_Path_move_impl(old_name, new_name))
    {
        char error[1024 * 4];
        wrench_snprintf(error, sizeof(error), "Failed to move \"%s\" to \"%s\".", old_name, new_name);

        wrenSetSlotString(vm, 0, (const char*)error);
        wrenAbortFiber(vm, 0);
    }
}

static void file_Path_remove(WrenVM* vm)
{
    const char* path = wrenGetSlotString(vm, 1);

    if (!file_Path_remove_impl(path))
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

static size_t file_File_bytesConsumed_impl(FILE* file)
{
    return (size_t)wrench_ftell(file);
}

static bool file_File_position_impl(FILE* file, long* origin, long* ending)
{
    wrench_assert(origin != NULL, "");
    wrench_assert(ending != NULL, "");

    *origin = wrench_ftell(file);

    if (*origin == -1)
    {
        return false;
    }

    if (wrench_fseek(file, 0, SEEK_END) != 0)
    {
        return false;
    }

    *ending = wrench_ftell(file);

    if (*ending == -1)
    {
        // TODO: Try to seek back?
        return false;
    }

    if (wrench_fseek(file, *origin, SEEK_SET) != 0)
    {
        return false;
    }

    return true;
}

static size_t file_File_bytesRemaining_impl(FILE* file)
{
    long origin;
    long ending;

    if (!file_File_position_impl(file, &origin, &ending))
    {
        return SIZE_MAX;
    }

    return (size_t)(ending - origin);
}

static size_t file_File_bytesTotal_impl(FILE* file)
{
    long origin;
    long ending;

    if (!file_File_position_impl(file, &origin, &ending))
    {
        return SIZE_MAX;
    }

    return (size_t)ending;
}

/* ===== [ wren ] =========================================================== */

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
        if (wrench_fclose(self->file) != 0)
        {
            wrench_assert(0, "File(\"%s\", \"%s\") dtor failed",
                        self->path != NULL ? self->path : "",
                        self->mode != NULL ? self->mode : "");
        }
    }

    wrench_free(self->path);
    wrench_free(self->mode);
}

static void file_File_open(WrenVM* vm)
{
    const char* path = wrenGetSlotString(vm, 1);
    const char* mode = wrenGetSlotString(vm, 2);

    FILE* file = wrench_fopen(path, mode);

    if (file != NULL)
    {
        file_File* data = (file_File*)wrenSetSlotNewForeign(vm, 0, 0, sizeof(file_File));
        WRENCH_SET_MAGIC_TAG(data, file, File);

        data->collect = true;
        data->file = file;

        data->path = wrench_strdup(path);
        data->mode = wrench_strdup(mode);
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

    if (wrench_fclose(self->file) != 0)
    {
        char error[1024 * 4];

        wrench_snprintf(error, sizeof(error), "failed to close file \"%s\" with mode \"%s\"",
                                                        self->path != NULL ? self->path : "",
                                                        self->mode != NULL ? self->mode : "");

        wrenSetSlotString(vm, 0, (const char*)error);
        wrenAbortFiber(vm, 0);
    }

    // NOTE: Path and mode can stay around until the dtor.
    self->file = NULL;
}

static void file_File_path_get(WrenVM* vm)
{
    file_File* self = (file_File*)wrenGetSlotForeign(vm, 0);
    WRENCH_CHECK_MAGIC_TAG(self, file, File);

    if (self->path != NULL)
    {
        wrenSetSlotString(vm, 0, (const char*)self->path);
    }
    else if (self->file == wrench_stderr)
    {
        wrenSetSlotString(vm, 0, "stderr");
    }
    else if (self->file == wrench_stdin)
    {
        wrenSetSlotString(vm, 0, "stdin");
    }
    else if (self->file == wrench_stdout)
    {
        wrenSetSlotString(vm, 0, "stdout");
    }
    else
    {
        wrenSetSlotString(vm, 0, "");
    }
}

static void file_File_mode_get(WrenVM* vm)
{
    file_File* self = (file_File*)wrenGetSlotForeign(vm, 0);
    WRENCH_CHECK_MAGIC_TAG(self, file, File);

    if (self->mode != NULL)
    {
        wrenSetSlotString(vm, 0, (const char*)self->mode);
    }
    else if (self->file == wrench_stderr)
    {
        wrenSetSlotString(vm, 0, "a");
    }
    else if (self->file == wrench_stdin)
    {
        wrenSetSlotString(vm, 0, "r");
    }
    else if (self->file == wrench_stdout)
    {
        wrenSetSlotString(vm, 0, "a");
    }
    else
    {
        wrenSetSlotString(vm, 0, "");
    }
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

static void file_File_stdout_get(WrenVM* vm)
{
    file_File* data = (file_File*)wrenSetSlotNewForeign(vm, 0, 0, sizeof(file_File));
    WRENCH_SET_MAGIC_TAG(data, file, File);

    data->file = wrench_stdout;

    // TODO: path
    // TODO: mode
}

static void file_File_stderr_get(WrenVM* vm)
{
    file_File* data = (file_File*)wrenSetSlotNewForeign(vm, 0, 0, sizeof(file_File));
    WRENCH_SET_MAGIC_TAG(data, file, File);

    data->file = wrench_stderr;

    // TODO: path
    // TODO: mode
}

static void file_File_stdin_get(WrenVM* vm)
{
    file_File* data = (file_File*)wrenSetSlotNewForeign(vm, 0, 0, sizeof(file_File));
    WRENCH_SET_MAGIC_TAG(data, file, File);

    data->file = wrench_stdin;

    // TODO: path
    // TODO: mode
}

/* TODO: Better error handling.
 */
static void file_File_getc(WrenVM* vm)
{
    file_File* self = (file_File*)wrenGetSlotForeign(vm, 0);
    WRENCH_CHECK_MAGIC_TAG(self, file, File);

    wrenSetSlotInt(vm, 0, wrench_getc(self->file));
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
            wrenSetSlotInt(vm, 0, wrench_putc(wrenGetSlotInt(vm, 1), self->file));
        }
        break;

        case WREN_TYPE_STRING:
        {
            const char* s = wrenGetSlotString(vm, 1);

            if (s[0] != '\0')
            {
                if (0)
                {
                    wrench_assert(s[1] == '\0', "multi-char string \"%s\"", s);
                }
                else
                {
                    /* XXX: This is super hacky. Consider using fwrite here?
                     */
                    switch (wrench_strlen(s))
                    {
                        case 1: break;

                        case 2:
                        {
                            const int s0 = wrench_putc(s[0], self->file);

                            if (s0 == EOF)
                            {
                                wrenSetSlotInt(vm, 0, EOF);
                                return;
                            }

                            const int s1 = wrench_putc(s[1], self->file);

                            if (s1 == EOF)
                            {
                                wrenSetSlotInt(vm, 0, EOF);
                                return;
                            }

                            // TODO: Untested - is this correct?
                            wrenSetSlotInt(vm, 0, s0 << 8 | s1);

                            return;
                        }
                        break;

                        case 3:
                        {
                            const int s0 = wrench_putc(s[0], self->file);

                            if (s0 == EOF)
                            {
                                wrenSetSlotInt(vm, 0, EOF);
                                return;
                            }

                            const int s1 = wrench_putc(s[1], self->file);

                            if (s1 == EOF)
                            {
                                wrenSetSlotInt(vm, 0, EOF);
                                return;
                            }

                            const int s2 = wrench_putc(s[2], self->file);

                            if (s2 == EOF)
                            {
                                wrenSetSlotInt(vm, 0, EOF);
                                return;
                            }

                            // TODO: Untested - are values ordered correctly?
                            wrenSetSlotInt(vm, 0, s0 << 16 | s1 << 8 | s2);

                            return;
                        }
                        break;

                        case 4:
                        {
                            const int s0 = wrench_putc(s[0], self->file);

                            if (s0 == EOF)
                            {
                                wrenSetSlotInt(vm, 0, EOF);
                                return;
                            }

                            const int s1 = wrench_putc(s[1], self->file);

                            if (s1 == EOF)
                            {
                                wrenSetSlotInt(vm, 0, EOF);
                                return;
                            }

                            const int s2 = wrench_putc(s[2], self->file);

                            if (s2 == EOF)
                            {
                                wrenSetSlotInt(vm, 0, EOF);
                                return;
                            }

                            const int s3 = wrench_putc(s[3], self->file);

                            if (s3 == EOF)
                            {
                                wrenSetSlotInt(vm, 0, EOF);
                                return;
                            }

                            // TODO: Untested - are these values in the correct order?
                            wrenSetSlotInt(vm, 0, s0 << 24 | s1 << 16 | s2 << 8 | s3);

                            return;
                        }
                        break;

                        default:
                        {
                            wrench_assert(0, "%s", s);
                        }
                        break;
                    }
                }

            #if !_WIN32
                if (s[0] == '\n' && file_File_ensureCRLF)
                {
                    wrenSetSlotInt(vm, 0, wrench_putc('\r', self->file) + wrench_putc('\n', self->file));
                }
                else
            #endif
                {
                    wrenSetSlotInt(vm, 0, wrench_putc(s[0], self->file));
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

    wrenSetSlotBool(vm, 0, wrench_feof(self->file) != 0);
}

static void file_File_read(WrenVM* vm)
{
    file_File* self = (file_File*)wrenGetSlotForeign(vm, 0);
    WRENCH_CHECK_MAGIC_TAG(self, file, File);

    const size_t bytes_requested = (size_t)wrenGetSlotInt(vm, 1);
    const size_t bytes_remaining = file_File_bytesRemaining_impl(self->file);

    if (bytes_remaining == SIZE_MAX)
    {
        wrenSetSlotString(vm, 0, "Failed to get file bytes remaining for read.");
        wrenAbortFiber(vm, 0);

        return;
    }

    // Avoid over-allocating in case huge values are passed.
    size_t bytes_to_read;

    if (bytes_remaining < bytes_requested)
    {
        bytes_to_read = bytes_remaining;
    }
    else
    {
        bytes_to_read = bytes_requested;
    }

    void* data = wrenStackMalloc(vm, bytes_to_read);

    if (data == NULL)
    {
        data = wrench_malloc(bytes_to_read);
    }

    if (data == NULL)
    {
        wrenSetSlotString(vm, 0, "Out of memory - failed to allocate for file read!");
        wrenAbortFiber(vm, 0);

        return;
    }

    if (wrench_fread(data, sizeof(char), bytes_to_read, self->file) != bytes_to_read)
    {
        wrenSetSlotString(vm, 0, "File read failed.");
        wrenAbortFiber(vm, 0);

        return;
    }

    wrenSetSlotBytes(vm, 0, (const char*)data, bytes_to_read);

    if (wrenIsStackMemory(vm, data))
    {
        wrenStackFree(vm, data, bytes_to_read);
    }
    else
    {
        wrench_free(data);
    }
}

/* TODO: Better error handling.
 */
static void file_File_write_(WrenVM* vm)
{
    file_File* self = (file_File*)wrenGetSlotForeign(vm, 0);
    WRENCH_CHECK_MAGIC_TAG(self, file, File);

    int size;
    const char* data = wrenGetSlotBytes(vm, 1, &size);

    wrenSetSlotInt(vm, 0, wrench_fwrite(data, 1, size, self->file));
}

static void file_File_bytesConsumed_get(WrenVM* vm)
{
    file_File* self = (file_File*)wrenGetSlotForeign(vm, 0);
    WRENCH_CHECK_MAGIC_TAG(self, file, File);

    wrenSetSlotInt(vm, 0, (int)file_File_bytesConsumed_impl(self->file));
}

static void file_File_bytesRemaining_get(WrenVM* vm)
{
    file_File* self = (file_File*)wrenGetSlotForeign(vm, 0);
    WRENCH_CHECK_MAGIC_TAG(self, file, File);

    size_t value = file_File_bytesRemaining_impl(self->file);

    if (value == SIZE_MAX)
    {
        /* TODO: Keep/copy the name and mode of the file.
         */
        wrenSetSlotString(vm, 0, "failed to get file bytes remaining");
        wrenAbortFiber(vm, 0);
    }
    else
    {
        wrenSetSlotInt(vm, 0, (int)value);
    }
}

static void file_File_bytesTotal_get(WrenVM* vm)
{
    file_File* self = (file_File*)wrenGetSlotForeign(vm, 0);
    WRENCH_CHECK_MAGIC_TAG(self, file, File);

    size_t value = file_File_bytesTotal_impl(self->file);

    if (value == SIZE_MAX)
    {
        /* TODO: Keep/copy the name and mode of the file.
         */
        wrenSetSlotString(vm, 0, "failed to get file total bytes");
        wrenAbortFiber(vm, 0);
    }
    else
    {
        wrenSetSlotInt(vm, 0, (int)value);
    }
}

static void file_File_flush(WrenVM* vm)
{
    file_File* self = (file_File*)wrenGetSlotForeign(vm, 0);
    WRENCH_CHECK_MAGIC_TAG(self, file, File);

    if (wrench_fflush(self->file) != 0)
    {
        /* TODO: Keep/copy the name and mode of the file.
         */
        wrenSetSlotString(vm, 0, "failed to flush file");
        wrenAbortFiber(vm, 0);
    }
}

static void file_File_readLine(WrenVM* vm)
{
    /* TODO: Win32 getline().
     */
    file_File* self = (file_File*)wrenGetSlotForeign(vm, 0);
    WRENCH_CHECK_MAGIC_TAG(self, file, File);

    const bool strip_newline = wrenGetSlotBool(vm, 1);

    /* Over-allocate.
     */
    const size_t bytes_remaining = file_File_bytesRemaining_impl(self->file);

    if (bytes_remaining == SIZE_MAX)
    {
        wrenSetSlotString(vm, 0, "Failed to get file bytes remaining for readLine.");
        wrenAbortFiber(vm, 0);

        return;
    }

    char* data = (char*)wrenStackMalloc(vm, bytes_remaining);

    if (data == NULL)
    {
        data = (char*)wrench_malloc(bytes_remaining);
    }

    if (data == NULL)
    {
        wrenSetSlotString(vm, 0, "Out of memory - failed to allocate for file readLine!");
        wrenAbortFiber(vm, 0);

        return;
    }

    size_t i = 0;
    while (true)
    {
        const int c = wrench_getc(self->file);

        if (c == EOF)
        {
            break;
        }

        data[i++] = (char)c;

        if (c == '\n')
        {
            if (strip_newline)
            {
                data[--i] = 0;
            }

            break;
        }
    }

    wrenSetSlotBytes(vm, 0, (const char*)data, i);

    if (wrenIsStackMemory(vm, data))
    {
        wrenStackFree(vm, data, bytes_remaining);
    }
    else
    {
        wrench_free(data);
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

            WREN_METHOD(file, Path, true, temp, "", "");

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

            if (!wrenCode(vm,

            "static split(path) {\n"
                "return path.replace(\"\\\\\", \"/\").split(\"/\")\n"
            "}\n"

            "static join(lhs, rhs) {\n"
            #if _WIN32
                "var mid = \"\\\\\"\n"
                "if (lhs.endsWith(\"/\") || lhs.endsWith(\"\\\\\")) {\n"
            #else
                "var mid = \"/\"\n"
                "if (lhs.endsWith(\"/\")) {\n"
            #endif
                    "return lhs + rhs\n"
                "} else {\n"
                    "return lhs + mid + rhs\n"
                "}\n"
            "}\n"

            )) { return false; }

            // TODO: join(list)

            WREN_METHOD(file, Path, true, isDirectory, "(path)", "(_)");
            WREN_METHOD(file, Path, true, isFile, "(path)", "(_)");

            WREN_METHOD(file, Path, true, createDirectory, "(path)", "(_)");
            WREN_METHOD(file, Path, true, createFile, "(path)", "(_)");

            if (!wrenCode(vm,

            "static copyDirectory(old_name, new_name) {\n"
                "Fiber.abort(\"TODO\")\n"
            "}\n"

            "static copyFile(old_name, new_name) {\n"
                "var old_file = File.open(old_name, \"rb\")\n"
                "var new_file = File.open(new_name, \"wb\")\n"

                "new_file.write(old_file.read())\n"

                "old_file.close()\n"
                "new_file.close()\n"
            "}\n"

            "static copy(old_name, new_name) {\n"
                "if (isDirectory(old_name)) {\n"
                    "return copyDirectory(old_name, new_name)\n"
                "} else {\n"
                    "return copyFile(old_name, new_name)\n"
                "}\n"
            "}\n"

            )) { return false; }

            WREN_METHOD(file, Path, true, move, "(old_name, new_name)", "(_,_)");
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

            // TODO: isOpen
            // TODO: collect

            WREN_GETTER(file, File, false, path);
            WREN_GETTER(file, File, false, mode);

            WREN_CODE("toString { \"%(type)(\\\"%(path)\\\", \\\"%(mode)\\\")\" }");

            // Write Windows-style line endings on Unix.
            WREN_PROPERTY(file, File, true, ensureCRLF);

            WREN_GETTER(file, File, true, stdout);
            WREN_GETTER(file, File, true, stderr);
            WREN_GETTER(file, File, true, stdin);

            /* XXX: getc and putc are also macros (which is all that differentiates them from fgetc/fputc).
             * We should probably #undef them in the same way we deal with standard input & output streams.
             */
            WREN_METHOD_EX(file, File, false, getc, "()", "()", file_File_getc);
            WREN_METHOD_EX(file, File, false, putc, "(c)", "(_)", file_File_putc);

            if (1)
            {
                WREN_METHOD_EX(file, File, true, EOF, "", "", file_File_EOF);
            }
            else
            {
                WREN_CODE("static EOF { " WRENCH_STRINGIFY(EOF) " }");
            }

            WREN_METHOD(file, File, false, eof, "()", "()");

            if (1)
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

            if (0)
            {
                // FIXME: wrenGetSlotInt fires bogus assert here.
                WREN_CODE("read() { read(Num.maxSafeInteger) }");
            }
            else
            {
                WREN_CODE("read() { read(bytesRemaining) }");
            }

            if (!wrenCode(vm,

            "static read(path) {\n"
                "var file = open(path, \"rb\")\n"
                "var data = file.read()\n"

                "file.close()\n"
                "return data\n"
            "}\n"

            )) { return false; }

            WREN_METHOD(file, File, false, write_, "(string)", "(_)");

            if (!wrenCode(vm,

            "write(string) {\n"
            #if _WIN32
                "return write_(string)\n"
            #else
                "if (!type.ensureCRLF) {\n"
                    "return write_(string)\n"
                "} else {\n"
                    "var r = 0\n"

                    "for (c in string) {\n"
                        "r = r + putc(c)\n"
                    "}\n"

                    "return r\n"
                "}\n"
            #endif
            "}\n"

            "static write(filename, contents) {\n"
                "var file = File.open(filename, \"wb\")\n"
                "file.write(contents)\n"
                "file.close()\n"
            "}\n"

            )) { return false; }

            // TODO: seek
            // TODO: tell (bytesConsumed)
            // TODO: size (bytesTotal)

            WREN_GETTER(file, File, false, bytesConsumed);
            WREN_GETTER(file, File, false, bytesRemaining);
            WREN_GETTER(file, File, false, bytesTotal);

            WREN_METHOD(file, File, false, flush, "()", "()");

            if (1)
            {
                WREN_METHOD(file, File, false, readLine, "(strip_newline)", "(_)");
            }
            else
            {
                if (!wrenCode(vm,

                "readLine(strip_newline) {\n"
                    "var s = []\n"

                    "while (!eof()) {\n"
                        "s.insert(-1, read(1))\n"

                        "if (s[-1] == \"\\n\") {\n"
                            "if (strip_newline) {\n"
                                "s.removeAt(-1)\n"
                            "}\n"

                            "break\n"
                        "}\n"
                    "}\n"

                    "return s.join()\n"
                "}\n"

                )) { return false; }
            }

            if (!wrenCode(vm,

            "readLine() { readLine(true) }\n"

            "readLines(strip_newlines) {\n"
                "var s = []\n"

                "while (!eof()) {\n"
                    "s.insert(-1, readLine(strip_newlines))\n"
                "}\n"

                "return s\n"
            "}\n"

            "readLines() { readLines(true) }\n"

            "static readLines(path, strip_newlines) {\n"
                "var file = open(path, \"rb\")\n"
                "var text = file.readLines(strip_newlines)\n"

                "file.close()\n"
                "return text\n"
            "}\n"

            "static readLines(path) {\n"
                "return readLines(path, true)\n"
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
