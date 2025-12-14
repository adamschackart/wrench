/* -----------------------------------------------------------------------------
--- Copyright (c) 2012-2026 Adam Schackart / "AJ Hackman", all rights reserved.
--- Distributed under the BSD license v2 (opensource.org/licenses/BSD-3-Clause)
----------------------------------------------------------------------------- */

#ifndef WRENCH_IMPLEMENTATION
#define WRENCH_IMPLEMENTATION 1
#endif
#include <wrench_file.h>
#include <wrench_vm.h>

/*
================================================================================
 * ~~ [ wren configuration ] ~~ *
--------------------------------------------------------------------------------
*/

static void vm_WrenConfiguration_ctor(WrenVM* vm)
{
    vm_WrenConfiguration* self = (
    vm_WrenConfiguration*)wrenSetSlotNewForeign(vm, 0, 0, sizeof(vm_WrenConfiguration));

    WRENCH_SET_MAGIC_TAG(self, vm, WrenConfiguration);
    self->config = &self->_config;

    wrenInitConfiguration(self->config);
}

static void vm_WrenConfiguration_dtor(void* data)
{
    //
}

static void vm_WrenConfiguration_primary(WrenVM* vm)
{
    vm_WrenConfiguration* self = (
    vm_WrenConfiguration*)wrenSetSlotNewForeign(vm, 0, 0, sizeof(vm_WrenConfiguration));

    WRENCH_SET_MAGIC_TAG(self, vm, WrenConfiguration);
    self->config = wrenGetConfig();
}

/*
================================================================================
 * ~~ [ wren virtual machine ] ~~ *
--------------------------------------------------------------------------------
*/

static void vm_WrenVM_ctor(WrenVM* vm)
{
    /* TODO: If slot 1 is list, new extended VM. If foreign (config), new VM.
     * If slot 1 is null, create an extended VM with our command-line args.
     * Make sure to set the same ForeignLibraryLoadEnabled as the calling VM.
     */
    WRENCH_STUB();
}

static void vm_WrenVM_dtor(void* data)
{
    WRENCH_CHECK_MAGIC_TAG(data, vm, WrenVM);

    if (((vm_WrenVM*)data)->collect)
    {
        if (((vm_WrenVM*)data)->extended)
        {
            wrenFreeExtendedVM(((vm_WrenVM*)data)->vm, true);
        }
        else
        {
            wrenFreeVM(((vm_WrenVM*)data)->vm);
        }
    }
}

static void vm_WrenVM_self(WrenVM* vm)
{
    vm_WrenVM* self = (vm_WrenVM*)wrenSetSlotNewForeign(vm, 0, 0, sizeof(vm_WrenVM));
    WRENCH_SET_MAGIC_TAG(self, vm, WrenVM);

    self->extended = wrenGetSlotBool(vm, 1);
    self->collect = wrenGetSlotBool(vm, 2);
    self->vm = vm;
}

static void vm_WrenVM_collect_get(WrenVM* vm)
{
    vm_WrenVM* self = (vm_WrenVM*)wrenGetSlotForeign(vm, 0);
    WRENCH_CHECK_MAGIC_TAG(self, vm, WrenVM);

    wrenSetSlotBool(vm, 0, self->collect);
}

static void vm_WrenVM_collect_set(WrenVM* vm)
{
    vm_WrenVM* self = (vm_WrenVM*)wrenGetSlotForeign(vm, 0);
    WRENCH_CHECK_MAGIC_TAG(self, vm, WrenVM);

    self->collect = wrenGetSlotBool(vm, 1);
}

static void vm_WrenVM_extended_get(WrenVM* vm)
{
    vm_WrenVM* self = (vm_WrenVM*)wrenGetSlotForeign(vm, 0);
    WRENCH_CHECK_MAGIC_TAG(self, vm, WrenVM);

    wrenSetSlotBool(vm, 0, self->extended);
}

static void vm_WrenVM_extended_set(WrenVM* vm)
{
    vm_WrenVM* self = (vm_WrenVM*)wrenGetSlotForeign(vm, 0);
    WRENCH_CHECK_MAGIC_TAG(self, vm, WrenVM);

    self->extended = wrenGetSlotBool(vm, 1);
}

static void vm_WrenVM_outputFile_get(WrenVM* vm)
{
    vm_WrenVM* self = (vm_WrenVM*)wrenGetSlotForeign(vm, 0);
    WRENCH_CHECK_MAGIC_TAG(self, vm, WrenVM);

    wrenGetVariable(vm, "file", "File", 0);

    file_File* file = (file_File*)wrenSetSlotNewForeign(vm, 0, 0, sizeof(file_File));
    WRENCH_SET_MAGIC_TAG(file, file, File);

    file->file = wrenGetOutputFile(self->vm);
}

static void vm_WrenVM_outputFile_set(WrenVM* vm)
{
    vm_WrenVM* self = (vm_WrenVM*)wrenGetSlotForeign(vm, 0);
    WRENCH_CHECK_MAGIC_TAG(self, vm, WrenVM);

    file_File* file = (file_File*)wrenGetSlotForeign(vm, 1);
    WRENCH_CHECK_MAGIC_TAG(file, file, File);

    wrenSetOutputFile(self->vm, file->file);
}

static void vm_WrenVM_errorString_get(WrenVM* vm)
{
    vm_WrenVM* self = (vm_WrenVM*)wrenGetSlotForeign(vm, 0);
    WRENCH_CHECK_MAGIC_TAG(self, vm, WrenVM);

    wrenSetSlotString(vm, 0, wrenGetErrorString(self->vm));
}

static void vm_WrenVM_errorString_set(WrenVM* vm)
{
    vm_WrenVM* self = (vm_WrenVM*)wrenGetSlotForeign(vm, 0);
    WRENCH_CHECK_MAGIC_TAG(self, vm, WrenVM);

    wrenSetErrorString(self->vm, wrenGetSlotString(vm, 1));
}

static void vm_WrenVM_getCommandLine_(WrenVM* vm)
{
    vm_WrenVM* self = (vm_WrenVM*)wrenGetSlotForeign(vm, 0);
    WRENCH_CHECK_MAGIC_TAG(self, vm, WrenVM);

    int argc;
    char** argv = wrenGetCommandLine(self->vm, &argc);

    wrenSetSlotNewList(vm, 0);

    for (int i = 0; i < argc; i++)
    {
        wrenSetSlotString(vm, 1, (const char*)argv[i]);
        wrenInsertInList(vm, 0, -1, 1);
    }
}

static void vm_WrenVM_getModuleSource(WrenVM* vm)
{
    vm_WrenVM* self = (vm_WrenVM*)wrenGetSlotForeign(vm, 0);
    WRENCH_CHECK_MAGIC_TAG(self, vm, WrenVM);

    const char* source = wrenGetModuleSource(self->vm, wrenGetSlotString(vm, 1));

    if (source == NULL)
    {
        wrenSetSlotNull(vm, 0); // Not found.
    }
    else
    {
        wrenSetSlotString(vm, 0, source);
    }
}

static void vm_WrenVM_printModuleSource(WrenVM* vm)
{
    vm_WrenVM* self = (vm_WrenVM*)wrenGetSlotForeign(vm, 0);
    WRENCH_CHECK_MAGIC_TAG(self, vm, WrenVM);

    const char* name = wrenGetSlotString(vm, 1);

    file_File* stream = (file_File*)wrenGetSlotForeign(vm, 2);
    WRENCH_CHECK_MAGIC_TAG(stream, file, File);

    const bool indent = wrenGetSlotBool(vm, 3);
    const bool strip_comments = wrenGetSlotBool(vm, 4);

    if (!wrenPrintModuleSource(self->vm, name, stream->file, indent, strip_comments))
    {
        wrenSetSlotString(vm, 0, wrenGetErrorString(self->vm));
        wrenAbortFiber(vm, 0);
    }
}

static void vm_WrenVM_basePath_get(WrenVM* vm)
{
    vm_WrenVM* self = (vm_WrenVM*)wrenGetSlotForeign(vm, 0);
    WRENCH_CHECK_MAGIC_TAG(self, vm, WrenVM);

    wrenSetSlotString(vm, 0, wrenGetBasePath(self->vm));
}

static void vm_WrenVM_basePath_set(WrenVM* vm)
{
    vm_WrenVM* self = (vm_WrenVM*)wrenGetSlotForeign(vm, 0);
    WRENCH_CHECK_MAGIC_TAG(self, vm, WrenVM);

    if (!wrenSetBasePath(self->vm, wrenGetSlotString(vm, 1)))
    {
        wrenSetSlotString(vm, 0, wrenGetErrorString(self->vm));
        wrenAbortFiber(vm, 0);
    }
}

static void vm_WrenVM_beginModule(WrenVM* vm)
{
    vm_WrenVM* self = (vm_WrenVM*)wrenGetSlotForeign(vm, 0);
    WRENCH_CHECK_MAGIC_TAG(self, vm, WrenVM);

    if (!wrenBeginModule(self->vm, wrenGetSlotString(vm, 1)))
    {
        wrenSetSlotString(vm, 0, wrenGetErrorString(self->vm));
        wrenAbortFiber(vm, 0);
    }
}

static void vm_WrenVM_code(WrenVM* vm)
{
    vm_WrenVM* self = (vm_WrenVM*)wrenGetSlotForeign(vm, 0);
    WRENCH_CHECK_MAGIC_TAG(self, vm, WrenVM);

    if (!wrenCode(self->vm, wrenGetSlotString(vm, 1)))
    {
        wrenSetSlotString(vm, 0, wrenGetErrorString(self->vm));
        wrenAbortFiber(vm, 0);
    }
}

static void vm_WrenVM_endModule(WrenVM* vm)
{
    vm_WrenVM* self = (vm_WrenVM*)wrenGetSlotForeign(vm, 0);
    WRENCH_CHECK_MAGIC_TAG(self, vm, WrenVM);

    if (!wrenEndModule(self->vm))
    {
        wrenSetSlotString(vm, 0, wrenGetErrorString(self->vm));
        wrenAbortFiber(vm, 0);
    }
}

static void vm_WrenVM_registerModule(WrenVM* vm)
{
    vm_WrenVM* self = (vm_WrenVM*)wrenGetSlotForeign(vm, 0);
    WRENCH_CHECK_MAGIC_TAG(self, vm, WrenVM);

    if (!wrenRegisterModule(self->vm, wrenGetSlotString(vm, 1), wrenGetSlotString(vm, 2)))
    {
        wrenSetSlotString(vm, 0, wrenGetErrorString(self->vm));
        wrenAbortFiber(vm, 0);
    }
}

static void vm_WrenVM_static_primary_get(WrenVM* vm)
{
    vm_WrenVM* self = (vm_WrenVM*)wrenSetSlotNewForeign(vm, 0, 0, sizeof(vm_WrenVM));
    WRENCH_SET_MAGIC_TAG(self, vm, WrenVM);

    self->extended = true;
    self->collect = false;
    self->vm = wrenGetPrimaryVM();
}

static void vm_WrenVM_static_primary_set(WrenVM* vm)
{
    vm_WrenVM* self = (vm_WrenVM*)wrenGetSlotForeign(vm, 1);
    WRENCH_CHECK_MAGIC_TAG(self, vm, WrenVM);

    wrenSetPrimaryVM(self->vm);
}

static void vm_WrenVM_primary_get(WrenVM* vm)
{
    vm_WrenVM* self = (vm_WrenVM*)wrenGetSlotForeign(vm, 0);
    WRENCH_CHECK_MAGIC_TAG(self, vm, WrenVM);

    wrenSetSlotBool(vm, 0, self->vm == wrenGetPrimaryVM());
}

static void vm_WrenVM_primary_set(WrenVM* vm)
{
    vm_WrenVM* self = (vm_WrenVM*)wrenGetSlotForeign(vm, 0);
    WRENCH_CHECK_MAGIC_TAG(self, vm, WrenVM);

    wrench_assert(wrenGetSlotBool(vm, 1), "");
    wrenSetPrimaryVM(self->vm);
}

static void vm_WrenVM_moduleVisit(WrenVM* vm, const char* moduleName, void* caller)
{
    wrenSetSlotString((WrenVM*)caller, 1, moduleName);
    wrenInsertInList((WrenVM*)caller, 0, -1, 1);
}

static void vm_WrenVM_getModules_(WrenVM* vm)
{
    vm_WrenVM* self = (vm_WrenVM*)wrenGetSlotForeign(vm, 0);
    WRENCH_CHECK_MAGIC_TAG(self, vm, WrenVM);

    wrenSetSlotNewList(vm, 0);
    wrenForEachModule(self->vm, vm_WrenVM_moduleVisit, vm);
}

static void vm_WrenVM_classVisit(WrenVM* vm, const char* moduleName,
                                const char* className, void* caller)
{
    wrenSetSlotString((WrenVM*)caller, 1, className);
    wrenInsertInList((WrenVM*)caller, 0, -1, 1);
}

static void vm_WrenVM_getClassesInModule(WrenVM* vm)
{
    vm_WrenVM* self = (vm_WrenVM*)wrenGetSlotForeign(vm, 0);
    WRENCH_CHECK_MAGIC_TAG(self, vm, WrenVM);

    wrenSetSlotNewList(vm, 0);
    const char* moduleName = wrenGetSlotString(vm, 1);

    wrenForEachClassInModule(self->vm, moduleName, vm_WrenVM_classVisit, vm);
}

static void vm_WrenVM_methodVisit(WrenVM* vm, const char* moduleName, const char* className,
                                        bool is_static, const char* signature, void* caller)
{
    char method[1024 * 4];

    if (wrench_snprintf(method, sizeof(method), /*"foreign "*/"%s%s", is_static ? "static " : "", signature) < 0)
    {
        // TODO: Handle truncation.
    }

    wrenSetSlotString((WrenVM*)caller, 1, (const char*)method);
    wrenInsertInList((WrenVM*)caller, 0, -1, 1);
}

static void vm_WrenVM_getMethodsInClass(WrenVM* vm)
{
    vm_WrenVM* self = (vm_WrenVM*)wrenGetSlotForeign(vm, 0);
    WRENCH_CHECK_MAGIC_TAG(self, vm, WrenVM);

    wrenSetSlotNewList(vm, 0);

    const char* moduleName = wrenGetSlotString(vm, 1);
    const char* className = wrenGetSlotString(vm, 2);

    wrenForEachMethodInClass(self->vm, moduleName, className, vm_WrenVM_methodVisit, vm);
}

/*
================================================================================
 * ~~ [ (un)hook ] ~~ *
--------------------------------------------------------------------------------
*/

#if WRENCH_VM_EXTENDED
    /*
     * Enable user extension of stdlib modules.
     */
    #ifndef __VM_EX_INL__
    #include <vm_ex.inl>
    #endif
#else
    static bool vmWrenInitEx(WrenVM* vm)
    {
        return true;
    }

    static void vmWrenQuitEx(void)
    {
        //
    }

    static bool vmWrenConfigurationWrenInitEx(WrenVM* vm)
    {
        return true;
    }

    static bool vmWrenVMWrenInitEx(WrenVM* vm)
    {
        return true;
    }
#endif /* WRENCH_VM_EXTENDED */

WRENCH_EXPORT bool vmWrenInit(WrenVM* vm)
{
    if (!wrenBeginModule(vm, "vm")) { return false; } else
    {
        WREN_CODE("import \"file\" for File");

        WREN_BEGIN_CLASS(vm, WrenConfiguration);
        {
            WREN_METHOD(vm, WrenConfiguration, true, primary, "", "");

            // TODO: ==
            // TODO: !=

            // TODO: reallocateFn
            // TODO: resolveModuleFn
            // TODO: loadModuleFn
            // TODO: bindForeignMethodFn
            // TODO: bindForeignClassFn
            // TODO: writeFn
            // TODO: errorFn

            // TODO: initialHeapSize
            // TODO: minHeapSize
            // TODO: heapGrowthPercent
            // TODO: userData

            if (!vmWrenConfigurationWrenInitEx(vm))
            {
                return false;
            }
        }
        WREN_END_CLASS();

        WREN_BEGIN_CLASS(vm, WrenVM);
        {
            WREN_CODE("construct new(arg) {}");

            WREN_METHOD(vm, WrenVM, true, self, "(extended, collect)", "(_,_)");
            WREN_CODE("static self(extended) { self(extended, false) }");
            WREN_CODE("static self { self(true, false) }");

            WREN_PROPERTY(vm, WrenVM, false, collect);
            WREN_PROPERTY(vm, WrenVM, false, extended);

            // TODO: ==
            // TODO: !=

            // ===== [ Wren API ] ==============================================

            // TODO: getVersionNumber
            // TODO: collectGarbage
            // TODO: interpret
            // TODO: makeCallHandle
            // TODO: call
            // TODO: releaseHandle
            // TODO: getSlotCount
            // TODO: ensureSlots
            // TODO: getSlotType
            // TODO: getSlotBool
            // TODO: getSlotBytes
            // TODO: getSlotDouble
            // TODO: getSlotForeign
            // TODO: getSlotString
            // TODO: getSlotHandle
            // TODO: setSlotBool
            // TODO: setSlotBytes
            // TODO: setSlotDouble
            // TODO: setSlotNewForeign
            // TODO: setSlotNewList
            // TODO: setSlotNewMap
            // TODO: setSlotNull
            // TODO: setSlotString
            // TODO: setSlotHandle
            // TODO: getListCount
            // TODO: getListElement
            // TODO: setListElement
            // TODO: insertInList
            // TODO: getMapCount
            // TODO: getMapContainsKey
            // TODO: getMapValue
            // TODO: setMapValue
            // TODO: removeMapValue
            // TODO: getVariable
            // TODO: hasVariable
            // TODO: hasModule
            // TODO: abortFiber
            // TODO: getUserData
            // TODO: setUserData

            // ===== [ Wrench API ] ============================================

            #if WRENCH_DEBUG
            {
                WREN_CODE("static debug { true }");
            }
            #else
            {
                WREN_CODE("static debug { false }");
            }
            #endif /* WRENCH_DEBUG */

            // TODO: registerGlobalInitFunction
            // TODO: registerGlobalQuitFunction

            // NOTE: We don't expose a `foreignLibraryLoadEnabled` property here,
            // to prevent malicious scripts from escaping sandboxed environments.

            WREN_PROPERTY(vm, WrenVM, false, outputFile);
            WREN_PROPERTY(vm, WrenVM, false, errorString);

            // TODO: getUserDataEx
            // TODO: setUserDataEx

            /* HACK: Can't call wrenEnsureSlots due to potential stack corruption.
             * Broker getter through a private call so we can build our Wren list.
             */
            WREN_METHOD(vm, WrenVM, false, getCommandLine_, "(unused)", "(_)");
            WREN_CODE("commandLine { getCommandLine_(null) }");

            // TODO: commandLine=

            WREN_METHOD(vm, WrenVM, false, getModuleSource, "(name)", "(_)");

            WREN_METHOD(vm, WrenVM, false, printModuleSource, "(name, stream, indent, stripComments)", "(_,_,_,_)");
            WREN_CODE("printModuleSource(name) { printModuleSource(name, File.stdout, true, false) }");

            WREN_PROPERTY(vm, WrenVM, false, basePath);

            // TODO: getFileReadCallback
            // TODO: setFileReadCallback
            // TODO: getFileFreeCallback
            // TODO: setFileFreeCallback

            // TODO: defaultFileRead
            // TODO: defaultFileFree

            // TODO: loadSourceFile

            WREN_METHOD(vm, WrenVM, false, beginModule, "(name)", "(_)");
            // TODO: codeEx
            WREN_METHOD(vm, WrenVM, false, code, "(source)", "(_)");
            WREN_METHOD(vm, WrenVM, false, endModule, "()", "()");

            // TODO: registerModuleEx
            WREN_METHOD(vm, WrenVM, false, registerModule, "(name, source)", "(_,_)");
            // TODO: registerClass
            // TODO: registerMethod

            WREN_PROPERTY_EX(vm, WrenVM, true, primary, vm_WrenVM_static_primary_get, vm_WrenVM_static_primary_set);
            WREN_PROPERTY(vm, WrenVM, false, primary);

            // TODO: forEachVM

            WREN_METHOD(vm, WrenVM, false, getModules_, "(unused)", "(_)");
            WREN_CODE("modules { getModules_(null) }");

            WREN_METHOD(vm, WrenVM, false, getClassesInModule, "(moduleName)", "(_)");
            WREN_METHOD(vm, WrenVM, false, getMethodsInClass, "(moduleName, className)", "(_,_)");

            // TODO: getSlotFloat
            // TODO: setSlotFloat
            // TODO: getSlotInt
            // TODO: setSlotInt
            // TODO: getSlotByte
            // TODO: setSlotByte

            // TODO: defaultReallocate
            // TODO: defaultResolveModule
            // TODO: defaultLoadModule
            // TODO: defaultBindForeignMethod
            // TODO: defaultBindForeignClass
            // TODO: defaultWrite
            // TODO: defaultError

            if (!vmWrenVMWrenInitEx(vm))
            {
                return false;
            }
        }
        WREN_END_CLASS();
    }

    if (!vmWrenInitEx(vm))
    {
        return false;
    }

    return wrenEndModule(vm);
}

WRENCH_EXPORT void vmWrenQuit(void)
{
    vmWrenQuitEx();
}
