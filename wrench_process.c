/* -----------------------------------------------------------------------------
--- Copyright (c) 2012-2026 Adam Schackart / "AJ Hackman", all rights reserved.
--- Distributed under the BSD license v2 (opensource.org/licenses/BSD-3-Clause)
----------------------------------------------------------------------------- */

/* TODO inter-process communication (ipc) via:
 *
 * - remote procedure calls (rpc)
 * - shared memory
 * - memory-mapped files
 * - message passing / message queue
 * - signals
 * - named pipes
 * - sockets
 * - unix domain sockets
 */
#ifndef WRENCH_IMPLEMENTATION
#define WRENCH_IMPLEMENTATION 1
#endif
#include <wrench_file.h>
#include <wrench_process.h>

/*
================================================================================
 * ~~ [ process ] ~~ *
--------------------------------------------------------------------------------
*/

static void process_Process_ctor(WrenVM* vm)
{
    WRENCH_STUB();
}

static void process_Process_dtor(void* data)
{
    process_Process* self = (process_Process*)data;
    WRENCH_CHECK_MAGIC_TAG(self, process, Process);

    if (0)
    {
        wrench_assert(!subprocess_alive(&self->process), "process still running");
    }
    else /*if (subprocess_alive(&self->process))*/
    {
        int return_code = 0;

        if (subprocess_join(&self->process, &return_code) == 0)
        {
            // Disregard return code.
        }
        else
        {
            wrench_assert(0, "subprocess_join failed.");
        }
    }

    // TODO: Dump stdout and/or stderr if any bytes remaining?

    if (subprocess_destroy(&self->process) != 0)
    {
        wrench_assert(0, "subprocess_destroy failed");
    }
}

static void process_Process_system(WrenVM* vm)
{
    /* XXX: We're basically treating this as "security hardened" mode. Disable `system` (insecure).
     */
    if (!wrenGetForeignLibraryLoadEnabled(vm))
    {
        wrenSetSlotString(vm, 0, "Foreign code loading is disabled - cannot run system commands.");
        wrenAbortFiber(vm, 0);

        return;
    }

    wrenSetSlotInt(vm, 0, wrench_system(wrenGetSlotString(vm, 1)));
}

static void process_Process_create_(WrenVM* vm)
{
    /* XXX: We're basically treating this as "security hardened" mode. Disable process creation.
     */
    if (!wrenGetForeignLibraryLoadEnabled(vm))
    {
        wrenSetSlotString(vm, 0, "Foreign code loading is disabled - cannot spawn processes.");
        wrenAbortFiber(vm, 0);

        return;
    }

    char** command_line = NULL;
    int num_args = 0;

    char** environment = NULL;
    int num_env_vars = 0;

    struct subprocess_s process;
    int options = 0;

    switch (wrenGetSlotType(vm, 1)) // command_line
    {
        case WREN_TYPE_LIST:
        {
            num_args = wrenGetListCount(vm, 1);

            // TODO: Use stack allocator... must free arguments in reverse order.
            command_line = (char**)wrench_malloc((num_args + 1) * sizeof(char*));

            if (command_line == NULL)
            {
                wrenSetSlotString(vm, 0, "Out of memory - failed to allocate command-line argument list.");
                wrenAbortFiber(vm, 0);

                return;
            }

            for (int i = 0; i < num_args; i++)
            {
                wrenGetListElement(vm, 1, i, 4);
                command_line[i] = wrench_strdup(wrenGetSlotString(vm, 4));

                if (command_line[i] == NULL)
                {
                    for (int j = 0; j < i; j++)
                    {
                        wrench_free(command_line[j]);
                    }

                    wrench_free(command_line);

                    wrenSetSlotString(vm, 0, "Out of memory - failed to allocate command-line argument list.");
                    wrenAbortFiber(vm, 0);

                    return;
                }
            }

            command_line[num_args] = NULL;
        }
        break;

        case WREN_TYPE_STRING:
        {
            wrench_assert(0, "TODO use strtok to break command-line args into list");
        }
        break;

        default:
        {
            wrench_assert(0, "%i", (int)wrenGetSlotType(vm, 1));
        }
        break;
    }

    switch (wrenGetSlotType(vm, 2)) // options
    {
        case WREN_TYPE_NUM:
        {
            options = wrenGetSlotInt(vm, 2);
        }
        break;

        case WREN_TYPE_LIST:
        {
            wrench_assert(0, "TODO get process options from list");
        }
        break;

        case WREN_TYPE_MAP:
        {
            wrench_assert(0, "TODO get process options from str -> bool map");
        }
        break;

        case WREN_TYPE_NULL:
        {
            wrench_assert(options == 0, "%i", options);

            if (0) options |= subprocess_option_combined_stdout_stderr;
            if (1) options |= subprocess_option_inherit_environment;
            if (1) options |= subprocess_option_enable_async;
            if (1) options |= subprocess_option_no_window;
            if (1) options |= subprocess_option_search_user_path;
        }
        break;

        case WREN_TYPE_STRING:
        {
            wrench_assert(0, "TODO parse process options from string");
        }
        break;

        default:
        {
            wrench_assert(0, "%i", (int)wrenGetSlotType(vm, 2));
        }
        break;
    }

    switch (wrenGetSlotType(vm, 3)) // environment
    {
        case WREN_TYPE_LIST:
        {
            wrench_assert(0, "TODO get process environment variables from list");
        }
        break;

        case WREN_TYPE_MAP:
        {
            wrench_assert(0, "TODO get process environment variables from map");
        }
        break;

        case WREN_TYPE_NULL:
        {
            wrench_assert(environment == NULL, "%p", environment);
            wrench_assert(num_env_vars == 0, "%i", num_env_vars);
        }
        break;

        case WREN_TYPE_STRING:
        {
            wrench_assert(0, "TODO parse process environment variables from string");
        }
        break;

        default:
        {
            wrench_assert(0, "%i", (int)wrenGetSlotType(vm, 3));
        }
        break;
    }

    const bool success =
    subprocess_create_ex((const char *const*)command_line, options, (const char *const*)environment, &process) == 0;

    if (num_args)
    {
        for (int i = 0; i < num_args; i++)
        {
            wrench_free(command_line[i]);
        }

        wrench_free(command_line);
    }

    if (num_env_vars)
    {
        for (int i = 0; i < num_env_vars; i++)
        {
            wrench_free(environment[i]);
        }

        wrench_free(environment);
    }

    if (success)
    {
        process_Process* self = (process_Process*)wrenSetSlotNewForeign(vm, 0, 0, sizeof(process_Process));
        WRENCH_SET_MAGIC_TAG(self, process, Process);

        self->process = process;
    }
    else
    {
        wrenSetSlotString(vm, 0, "Failed to spawn process.");
        wrenAbortFiber(vm, 0);
    }
}

static void process_Process_stdin(WrenVM* vm)
{
    process_Process* self = (process_Process*)wrenGetSlotForeign(vm, 0);
    WRENCH_CHECK_MAGIC_TAG(self, process, Process);

    // TODO: Slow! Should use a WrenHandle.
    wrenGetVariable(vm, "file", "File", 0);

    file_File* file = (file_File*)wrenSetSlotNewForeign(vm, 0, 0, sizeof(file_File));
    WRENCH_SET_MAGIC_TAG(file, file, File);

    file->collect = false;
    file->file = subprocess_stdin(&self->process);
}

static void process_Process_stdout(WrenVM* vm)
{
    process_Process* self = (process_Process*)wrenGetSlotForeign(vm, 0);
    WRENCH_CHECK_MAGIC_TAG(self, process, Process);

    // TODO: Slow! Should use a WrenHandle.
    wrenGetVariable(vm, "file", "File", 0);

    file_File* file = (file_File*)wrenSetSlotNewForeign(vm, 0, 0, sizeof(file_File));
    WRENCH_SET_MAGIC_TAG(file, file, File);

    file->collect = false;
    file->file = subprocess_stdout(&self->process);
}

static void process_Process_stderr(WrenVM* vm)
{
    process_Process* self = (process_Process*)wrenGetSlotForeign(vm, 0);
    WRENCH_CHECK_MAGIC_TAG(self, process, Process);

    FILE* handle = subprocess_stderr(&self->process);

    if (handle == NULL)
    {
        wrenSetSlotNull(vm, 0);
        return;
    }

    // TODO: Slow! Should use a WrenHandle.
    wrenGetVariable(vm, "file", "File", 0);

    file_File* file = (file_File*)wrenSetSlotNewForeign(vm, 0, 0, sizeof(file_File));
    WRENCH_SET_MAGIC_TAG(file, file, File);

    file->collect = false;
    file->file = handle;
}

static void process_Process_join(WrenVM* vm)
{
    process_Process* self = (process_Process*)wrenGetSlotForeign(vm, 0);
    WRENCH_CHECK_MAGIC_TAG(self, process, Process);

    int return_code = 0;

    if (subprocess_join(&self->process, &return_code) == 0)
    {
        wrenSetSlotInt(vm, 0, return_code);
    }
    else
    {
        wrenSetSlotString(vm, 0, "Process join failed.");
        wrenAbortFiber(vm, 0);
    }
}

static void process_Process_terminate(WrenVM* vm)
{
    process_Process* self = (process_Process*)wrenGetSlotForeign(vm, 0);
    WRENCH_CHECK_MAGIC_TAG(self, process, Process);

    if (subprocess_terminate(&self->process) != 0)
    {
        wrenSetSlotString(vm, 0, "Process termination failed.");
        wrenAbortFiber(vm, 0);
    }
}

static void process_Process_readStdoutByte(WrenVM* vm)
{
    process_Process* self = (process_Process*)wrenGetSlotForeign(vm, 0);
    WRENCH_CHECK_MAGIC_TAG(self, process, Process);

    unsigned char c;

    if (subprocess_read_stdout(&self->process, (char*)&c, 1) != 0)
    {
        wrenSetSlotByte(vm, 0, c);
    }
    else
    {
        wrenSetSlotInt(vm, 0, EOF);
    }
}

static void process_Process_readStdout(WrenVM* vm)
{
    WRENCH_TEMP();

    process_Process* self = (process_Process*)wrenGetSlotForeign(vm, 0);
    WRENCH_CHECK_MAGIC_TAG(self, process, Process);

    /* TODO: Read in chunks into a resizable array, so we can request huge values to read all bytes.
     */
    const unsigned int requested_size = (unsigned int)wrenGetSlotInt(vm, 1);
    char* buffer = (char*)wrench_malloc(requested_size);

    if (buffer == NULL)
    {
        wrenSetSlotString(vm, 0, "Out of memory - failed to allocate buffer for process read");
        wrenAbortFiber(vm, 0);

        return;
    }

    unsigned int actual_size = subprocess_read_stdout(&self->process, buffer, requested_size);
    wrenSetSlotBytes(vm, 0, buffer, (size_t)requested_size);
    wrench_free(buffer);
}

static void process_Process_readStderrByte(WrenVM* vm)
{
    process_Process* self = (process_Process*)wrenGetSlotForeign(vm, 0);
    WRENCH_CHECK_MAGIC_TAG(self, process, Process);

    unsigned char c;

    if (subprocess_read_stderr(&self->process, (char*)&c, 1) != 0)
    {
        wrenSetSlotByte(vm, 0, c);
    }
    else
    {
        wrenSetSlotInt(vm, 0, EOF);
    }
}

static void process_Process_readStderr(WrenVM* vm)
{
    WRENCH_TEMP();

    process_Process* self = (process_Process*)wrenGetSlotForeign(vm, 0);
    WRENCH_CHECK_MAGIC_TAG(self, process, Process);

    /* TODO: Read in chunks into a resizable array, so we can request huge values to read all bytes.
     */
    const unsigned int requested_size = (unsigned int)wrenGetSlotInt(vm, 1);
    char* buffer = (char*)wrench_malloc(requested_size);

    if (buffer == NULL)
    {
        wrenSetSlotString(vm, 0, "Out of memory - failed to allocate buffer for process read");
        wrenAbortFiber(vm, 0);

        return;
    }

    unsigned int actual_size = subprocess_read_stderr(&self->process, buffer, requested_size);
    wrenSetSlotBytes(vm, 0, buffer, (size_t)requested_size);
    wrench_free(buffer);
}

static void process_Process_alive(WrenVM* vm)
{
    process_Process* self = (process_Process*)wrenGetSlotForeign(vm, 0);
    WRENCH_CHECK_MAGIC_TAG(self, process, Process);

    wrenSetSlotBool(vm, 0, subprocess_alive(&self->process) != 0);
}

/*
================================================================================
 * ~~ [ (un)hook ] ~~ *
--------------------------------------------------------------------------------
*/

#if WRENCH_PROCESS_EXTENDED
    /*
     * Enable user extension of stdlib modules.
     */
    #ifndef __PROCESS_EX_INL__
    #include <process_ex.inl>
    #endif
#else
    static bool processWrenInitEx(WrenVM* vm)
    {
        return true;
    }

    static void processWrenQuitEx(void)
    {
        //
    }

    static bool processProcessWrenInitEx(WrenVM* vm)
    {
        return true;
    }
#endif /* WRENCH_PROCESS_EXTENDED */

#undef stdin
#undef stdout
#undef stderr

WRENCH_EXPORT bool processWrenInit(WrenVM* vm)
{
    if (!wrenBeginModule(vm, "process")) { return false; } else
    {
        WREN_CODE("import \"file\" for File");

        WREN_BEGIN_CLASS(process, Process);
        {
            WREN_METHOD(process, Process, true, system, "(cmd)", "(_)");

            /* Easy mode - runs a program and blocks until it's finished, allowing std(out/err) to behave normally.
             */
            if (!wrenCode(vm,

            "static run(cmd) {\n"
                "var process = create(cmd)\n"
                "var code = process.join()\n"

                "System.write(process.readStdout())\n"
                "System.write(process.readStderr())\n"

                "return code\n"
            "}\n"

            )) { return false; }

            WREN_CODE("static option_combined_stdout_stderr { 0x1 }");
            WREN_CODE("static option_inherit_environment { 0x2 }");
            WREN_CODE("static option_enable_async { 0x4 }");
            WREN_CODE("static option_no_window { 0x8 }");
            WREN_CODE("static option_search_user_path { 0x10 }");

            WREN_METHOD(process, Process, true, create_, "(command_line, options, environment, extra_slot)", "(_,_,_,_)");

            /* XXX TODO: Temp stopgap solution. Need a function to count the tokens strtok would find.
             */
            if (!wrenCode(vm,

            "static create(command_line, options, environment) {\n"
                "if (command_line is String) {\n"
                    "command_line = command_line.split(\" \")\n"
                "}\n"

                "return create_(command_line, options, environment, null)\n"
            "}\n"

            )) { return false; }

            WREN_CODE("static create(command_line, options) { create(command_line, options, null) }");
            WREN_CODE("static create(command_line) { create(command_line, null) }");

            // TODO: self

            WREN_METHOD(process, Process, false, stdin, "", "");
            WREN_METHOD(process, Process, false, stdout, "", "");
            WREN_METHOD(process, Process, false, stderr, "", "");

            WREN_METHOD(process, Process, false, join, "()", "()");
            WREN_METHOD(process, Process, false, terminate, "()", "()");

            WREN_METHOD(process, Process, false, readStdoutByte, "()", "()");
            WREN_METHOD(process, Process, false, readStderrByte, "()", "()");

            if (0)
            {
                WREN_METHOD(process, Process, false, readStdout, "(size)", "(_)");
            }
            else
            {
                /* FIXME: This is much slower, but much safer and easier to use.
                 */
                if (!wrenCode(vm,

                "readStdout(count) {\n"
                    "var EOF = File.EOF\n"
                    "var s = []\n"

                    "for (i in 0...count) {\n"
                        "var c = readStdoutByte()\n"

                        "if (c < 0 || c == EOF) {\n"
                            "break\n"
                        "} else {\n"
                            "s.insert(-1, String.fromByte(c))\n"
                        "}\n"
                    "}\n"

                    "return s.join()\n"
                "}\n"

                )) { return false; }

                WREN_CODE("readStdout() { readStdout(Num.maxSafeInteger) }");
            }

            if (0)
            {
                WREN_METHOD(process, Process, false, readStderr, "(size)", "(_)");
            }
            else
            {
                /* FIXME: This is much slower, but much safer and easier to use.
                 */
                if (!wrenCode(vm,

                "readStderr(count) {\n"
                    "var EOF = File.EOF\n"
                    "var s = []\n"

                    "for (i in 0...count) {\n"
                        "var c = readStderrByte()\n"

                        "if (c < 0 || c == EOF) {\n"
                            "break\n"
                        "} else {\n"
                            "s.insert(-1, String.fromByte(c))\n"
                        "}\n"
                    "}\n"

                    "return s.join()\n"
                "}\n"

                )) { return false; }

                WREN_CODE("readStderr() { readStderr(Num.maxSafeInteger) }");
            }

            WREN_METHOD(process, Process, false, alive, "", "");

            if (!processProcessWrenInitEx(vm))
            {
                return false;
            }
        }
        WREN_END_CLASS();
    }

    if (!processWrenInitEx(vm))
    {
        return false;
    }

    return wrenEndModule(vm);
}

WRENCH_EXPORT void processWrenQuit(void)
{
    processWrenQuitEx();
}
