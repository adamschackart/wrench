/* -----------------------------------------------------------------------------
--- Copyright (c) 2012-2026 Adam Schackart / "AJ Hackman", all rights reserved.
--- Distributed under the BSD license v2 (opensource.org/licenses/BSD-3-Clause)
----------------------------------------------------------------------------- */

#ifndef WRENCH_IMPLEMENTATION
#define WRENCH_IMPLEMENTATION 1
#endif
#include <wrench_zip.h>

#ifdef MICROSOFT_WINDOWS_WINBASE_H_DEFINE_INTERLOCKED_CPLUSPLUS_OVERLOADS
#undef MICROSOFT_WINDOWS_WINBASE_H_DEFINE_INTERLOCKED_CPLUSPLUS_OVERLOADS
#endif

#if !defined(ZIP_H)
    /*
     * Override memory allocation.
     */
    #define calloc(num, size) wrench_calloc(num, size)
    #define free(ptr) wrench_free(ptr)
    #define malloc(size) wrench_malloc(size)
    #define realloc(ptr, size) wrench_realloc(ptr, size)

    #include <zip/src/zip.h>

    #ifndef ZIP_C
    #define ZIP_C
    #include <zip/src/zip.c>
    #endif

    #undef calloc
    #undef free
    #undef malloc
    #undef realloc

    /* XXX: miniz #defines these.
     */
    #undef adler32
    #undef crc32
    #undef alloc_func
    #undef free_func

    /* XXX: zip #defines this.
     */
    #undef fileno
#endif

/*
================================================================================
 * ~~ [ 32-bit cyclic redundancy check ] ~~ *
--------------------------------------------------------------------------------
*/

#if defined(ZIP_C) && 0
    #define wrench_crc32 mz_crc32
#else
    static uint32_t wrench_crc32(uint32_t crc, const uint8_t* data, size_t size)
    {
        static uint32_t crc_table[256];

        if (crc_table[1] == 0)
        {
            for (uint32_t i = 0; i < 256; i++)
            {
                uint32_t s, j;

                for (s = i, j = 0; j < 8; j++)
                {
                    s = (s >> 1) ^ (s & 1 ? 0xedb88320 : 0);
                }

                crc_table[i] = s;
            }
        }

        // Initial mixing step.
        crc = ~crc;

        for (size_t i = 0; i < size; i++)
        {
            crc = (crc >> 8) ^ crc_table[data[i] ^ (crc & 0xff)];
        }

        // Final mixing step.
        crc = ~crc;

        return crc;
    }
#endif

static void zip_CRC32_call(WrenVM* vm)
{
    int crc = wrenGetSlotInt(vm, 1);

    int size;
    const uint8_t* data = (const uint8_t*)wrenGetSlotBytes(vm, 2, &size);

    wrenSetSlotUnsignedInt(vm, 0, wrench_crc32(crc, data, size));
}

/*
================================================================================
 * ~~ [ archive ] ~~ *
--------------------------------------------------------------------------------
*/

static void zip_Archive_ctor(WrenVM* vm)
{
    WRENCH_STUB();
}

static void zip_Archive_dtor(void* data)
{
    WRENCH_CHECK_MAGIC_TAG(data, zip, Archive);
    zip_Archive* self = (zip_Archive*)data;

    if (self->zip != NULL)
    {
        zip_close(self->zip);
    }
}

static void zip_Archive_open(WrenVM* vm)
{
    char error[1024 * 4];

    const char* path = wrenGetSlotString(vm, 1);
    const int level = wrenGetSlotInt(vm, 2);
    const char* mode = wrenGetSlotString(vm, 3);

    wrench_assert(mode[0] != '\0', "%s", mode);
    wrench_assert(mode[1] == '\0', "%s", mode);

    int error_num;
    struct zip_t* zip = zip_openwitherror(path, level, mode[0], &error_num);

    if (zip == NULL)
    {
        wrench_snprintf(error, sizeof(error), "Failed to open archive \"%s\": %s.",
                                                    path, zip_strerror(error_num));

        wrenSetSlotString(vm, 0, (const char*)error);
        wrenAbortFiber(vm, 0);
    }
    else
    {
        zip_Archive* self = (zip_Archive*)wrenSetSlotNewForeign(vm, 0, 0, sizeof(zip_Archive));
        WRENCH_SET_MAGIC_TAG(self, zip, Archive);

        self->zip = zip;
    }
}

static void zip_Archive_close(WrenVM* vm)
{
    zip_Archive* self = (zip_Archive*)wrenGetSlotForeign(vm, 0);
    WRENCH_CHECK_MAGIC_TAG(self, zip, Archive);

    if (self->zip != NULL)
    {
        zip_close(self->zip);
        self->zip = NULL;
    }
}

static void zip_Archive_save(WrenVM* vm)
{
    WRENCH_STUB();
}

static void zip_Archive_is64(WrenVM* vm)
{
    zip_Archive* self = (zip_Archive*)wrenGetSlotForeign(vm, 0);
    WRENCH_CHECK_MAGIC_TAG(self, zip, Archive);

    const int status = zip_is64(self->zip);

    if (status < 0)
    {
        wrenSetSlotString(vm, 0, zip_strerror(status));
        wrenAbortFiber(vm, 0);
    }
    else
    {
        wrenSetSlotBool(vm, 0, status);
    }
}

static void zip_Archive_listEntries_(WrenVM* vm)
{
    zip_Archive* self = (zip_Archive*)wrenGetSlotForeign(vm, 0);
    WRENCH_CHECK_MAGIC_TAG(self, zip, Archive);

    wrenSetSlotNewList(vm, 0);

    for (ssize_t i = 0, n = zip_entries_total(self->zip); i < n; i++)
    {
        int status = zip_entry_openbyindex(self->zip, (size_t)i);

        if (status != 0)
        {
            wrenSetSlotString(vm, 0, zip_strerror(status));
            wrenAbortFiber(vm, 0);
        }

        const char* name = zip_entry_name(self->zip);

        if (name != NULL && name[0] != '\0')
        {
            wrenSetSlotString(vm, 1, name);
            wrenInsertInList(vm, 0, -1, 1);
        }

        status = zip_entry_close(self->zip);

        if (status != 0)
        {
            wrenSetSlotString(vm, 0, zip_strerror(status));
            wrenAbortFiber(vm, 0);
        }
    }
}

static void zip_Archive_hasEntry(WrenVM* vm)
{
    WRENCH_STUB();
}

static void zip_Archive_entryIsFile(WrenVM* vm)
{
    WRENCH_STUB();
}

static void zip_Archive_entryIsDir(WrenVM* vm)
{
    WRENCH_STUB();
}

static void zip_Archive_entryCRC32(WrenVM* vm)
{
    char error[1024 * 4];
    int status;

    zip_Archive* self = (zip_Archive*)wrenGetSlotForeign(vm, 0);
    WRENCH_CHECK_MAGIC_TAG(self, zip, Archive);

    const char* name = wrenGetSlotString(vm, 1);
    const bool case_sensitive = wrenGetSlotBool(vm, 2);

    if (case_sensitive)
    {
        status = zip_entry_opencasesensitive(self->zip, name);
    }
    else
    {
        status = zip_entry_open(self->zip, name);
    }

    if (status != 0)
    {
        wrench_snprintf(error, sizeof(error), "Failed to open archive entry \"%s\": %s.",
                                                            name, zip_strerror(status));

        wrenSetSlotString(vm, 0, (const char*)error);
        wrenAbortFiber(vm, 0);

        return;
    }

    wrenSetSlotInt(vm, 0, (int)zip_entry_crc32(self->zip));
    status = zip_entry_close(self->zip);

    if (status != 0)
    {
        wrench_snprintf(error, sizeof(error), "Failed to close archive entry \"%s\": %s.",
                                                            name, zip_strerror(status));

        wrenSetSlotString(vm, 0, (const char*)error);
        wrenAbortFiber(vm, 0);

        return;
    }
}

static void zip_Archive_entryCompressedSize(WrenVM* vm)
{
    WRENCH_STUB();
}

static void zip_Archive_entryDecompressedSize(WrenVM* vm)
{
    WRENCH_STUB();
}

static void zip_Archive_readEntry(WrenVM* vm)
{
    char error[1024 * 4];
    int status;

    zip_Archive* self = (zip_Archive*)wrenGetSlotForeign(vm, 0);
    WRENCH_CHECK_MAGIC_TAG(self, zip, Archive);

    const char* name = wrenGetSlotString(vm, 1);
    const bool case_sensitive = wrenGetSlotBool(vm, 2);
    const bool checksum = wrenGetSlotBool(vm, 3);

    if (case_sensitive)
    {
        status = zip_entry_opencasesensitive(self->zip, name);
    }
    else
    {
        status = zip_entry_open(self->zip, name);
    }

    if (status != 0)
    {
        wrench_snprintf(error, sizeof(error), "Failed to open archive entry \"%s\": %s.",
                                                            name, zip_strerror(status));

        wrenSetSlotString(vm, 0, (const char*)error);
        wrenAbortFiber(vm, 0);

        return;
    }

    const size_t size = zip_entry_size(self->zip);
    void* data = wrench_malloc(size);

    if (data == NULL)
    {
        wrench_snprintf(error, sizeof(error), "Out of memory - failed to allocate"
                                        " space for archive entry \"%s\".", name);

        wrenSetSlotString(vm, 0, (const char*)error);
        wrenAbortFiber(vm, 0);

        zip_entry_close(self->zip);
        return;
    }

    status = (int)zip_entry_noallocread(self->zip, data, size);

    if (status < 0)
    {
        wrench_snprintf(error, sizeof(error), "Failed to read archive entry \"%s\": %s.",
                                                            name, zip_strerror(status));

        wrenSetSlotString(vm, 0, (const char*)error);
        wrenAbortFiber(vm, 0);

        zip_entry_close(self->zip);
        wrench_free(data);

        return;
    }

    if (checksum)
    {
        const unsigned int expected_checksum = zip_entry_crc32(self->zip);
        const unsigned int computed_checksum = wrench_crc32(0, (const uint8_t*)data, size);

        if (expected_checksum != computed_checksum)
        {
            wrench_snprintf(error, sizeof(error), "Archive entry \"%s\" is corrupt.", name);

            wrenSetSlotString(vm, 0, (const char*)error);
            wrenAbortFiber(vm, 0);

            zip_entry_close(self->zip);
            wrench_free(data);

            return;
        }
    }

    status = zip_entry_close(self->zip);

    if (status != 0)
    {
        wrench_snprintf(error, sizeof(error), "Failed to close archive entry \"%s\": %s.",
                                                            name, zip_strerror(status));

        wrenSetSlotString(vm, 0, (const char*)error);
        wrenAbortFiber(vm, 0);

        wrench_free(data);
        return;
    }

    wrenSetSlotBytes(vm, 0, (const char*)data, size);
    wrench_free(data);
}

static void zip_Archive_writeEntry(WrenVM* vm)
{
    WRENCH_STUB();
}

static void zip_Archive_appendEntry(WrenVM* vm)
{
    WRENCH_STUB();
}

static void zip_Archive_removeEntry(WrenVM* vm)
{
    WRENCH_STUB();
}

/*
================================================================================
 * ~~ [ (un)hook ] ~~ *
--------------------------------------------------------------------------------
*/

#if WRENCH_ZIP_EXTENDED
    /*
     * Enable user extension of stdlib modules.
     */
    #ifndef __ZIP_EX_INL__
    #include <zip_ex.inl>
    #endif
#else
    static bool zipWrenInitEx(WrenVM* vm)
    {
        return true;
    }

    static void zipWrenQuitEx(void)
    {
        //
    }

    static bool zipCRC32WrenInitEx(WrenVM* vm)
    {
        return true;
    }

    static bool zipArchiveWrenInitEx(WrenVM* vm)
    {
        return true;
    }
#endif /* WRENCH_ZIP_EXTENDED */

WRENCH_EXPORT bool zipWrenInit(WrenVM* vm)
{
    if (!wrenBeginModule(vm, "zip")) { return false; } else
    {
        WREN_BEGIN_CLASS_EX(zip, CRC32, NULL, NULL);
        {
            WREN_METHOD(zip, CRC32, true, call, "(crc, data)", "(_,_)");
            WREN_CODE("static call(data) { call(0, data) }");

            if (!zipCRC32WrenInitEx(vm))
            {
                return false;
            }
        }
        WREN_END_CLASS();

        WREN_BEGIN_CLASS(zip, Archive);
        {
            // TODO: ==
            // TODO: !=

            WREN_METHOD(zip, Archive, true, open, "(path, level, mode)", "(_,_,_)");
            WREN_CODE("static open(path) { open(path, 0, \"r\") }");

            WREN_METHOD(zip, Archive, false, close, "()", "()");

            WREN_METHOD(zip, Archive, false, save, "(filename)", "(_)");
            WREN_METHOD(zip, Archive, false, is64, "", "");

            /* HACK: Can't call wrenEnsureSlots due to potential stack corruption.
             * Broker getter through a private call so we can build our Wren list.
             */
            WREN_METHOD(zip, Archive, false, listEntries_, "(unused)", "(_)");
            WREN_CODE("entries { listEntries_(null) }");

            WREN_METHOD(zip, Archive, false, hasEntry, "(name, case_sensitive)", "(_,_)");
            WREN_CODE("hasEntry(name) { hasEntry(name, true) }");

            WREN_METHOD(zip, Archive, false, entryIsFile, "(name, case_sensitive)", "(_,_)");
            WREN_CODE("entryIsFile(name) { entryIsFile(name, true) }");

            WREN_METHOD(zip, Archive, false, entryIsDir, "(name, case_sensitive)", "(_,_)");
            WREN_CODE("entryIsDir(name) { entryIsDir(name, true) }");

            WREN_METHOD(zip, Archive, false, entryCRC32, "(name, case_sensitive)", "(_,_)");
            WREN_CODE("entryCRC32(name) { entryCRC32(name, true) }");

            WREN_METHOD(zip, Archive, false, entryCompressedSize, "(name, case_sensitive)", "(_,_)");
            WREN_CODE("entryCompressedSize(name) { entryCompressedSize(name, true) }");

            WREN_METHOD(zip, Archive, false, entryDecompressedSize, "(name, case_sensitive)", "(_,_)");
            WREN_CODE("entryDecompressedSize(name) { entryDecompressedSize(name, true) }");

            WREN_METHOD(zip, Archive, false, readEntry, "(name, case_sensitive, checksum)", "(_,_,_)");
            WREN_CODE("readEntry(name) { readEntry(name, true, true) }");

            WREN_METHOD(zip, Archive, false, writeEntry, "(name, data)", "(_,_)");
            WREN_METHOD(zip, Archive, false, appendEntry, "(filename)", "(_)");

            WREN_METHOD(zip, Archive, false, removeEntry, "(name, case_sensitive)", "(_,_)");
            WREN_CODE("removeEntry(name) { removeEntry(name, true, true) }");

            if (!zipArchiveWrenInitEx(vm))
            {
                return false;
            }
        }
        WREN_END_CLASS();
    }

    if (!zipWrenInitEx(vm))
    {
        return false;
    }

    return wrenEndModule(vm);
}

WRENCH_EXPORT void zipWrenQuit(void)
{
    zipWrenQuitEx();
}
