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

    // TODO: path
    // TODO: mode
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

    switch (wrenGetSlotType(vm, 1))
    {
        case WREN_TYPE_STRING:
        {
            if (!wrenSetBasePath(self->vm, wrenGetSlotString(vm, 1)))
            {
                wrenSetSlotString(vm, 0, wrenGetErrorString(self->vm));
                wrenAbortFiber(vm, 0);
            }
        }
        break;

        case WREN_TYPE_NULL:
        {
            if (!wrenSetBasePath(self->vm, NULL))
            {
                wrench_assert(0, "unreachable");
            }
        }
        break;

        default:
        {
            wrench_assert(0, "");
        }
        break;
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

static void vm_WrenVM_objectHasMethod(WrenVM* vm)
{
    vm_WrenVM* self = (vm_WrenVM*)wrenGetSlotForeign(vm, 0);
    WRENCH_CHECK_MAGIC_TAG(self, vm, WrenVM);

    const char* signature = wrenGetSlotString(vm, 2);
    const bool result = wrenObjectHasMethod(self->vm, 1, signature);

    wrenSetSlotBool(vm, 0, result);
}

static void vm_WrenVM_objectCountMethods(WrenVM* vm)
{
    vm_WrenVM* self = (vm_WrenVM*)wrenGetSlotForeign(vm, 0);
    WRENCH_CHECK_MAGIC_TAG(self, vm, WrenVM);

    const size_t result = wrenObjectCountMethods(self->vm, 1);
    wrench_assert(result <= UINT32_MAX, "%" PRIu64, (uint64_t)result);

    wrenSetSlotUnsignedInt(vm, 0, (unsigned int)result);
}

static void vm_WrenVM_objectListMethods(WrenVM* vm)
{
    #if WRENCH_DEBUG
    {
        vm_WrenVM* self = (vm_WrenVM*)wrenGetSlotForeign(vm, 0);
        WRENCH_CHECK_MAGIC_TAG(self, vm, WrenVM);

        wrench_assert(self->vm == vm, "TODO");
    }
    #endif /* WRENCH_DEBUG */

    /* HACK: We can safely overwrite the object slot with our temp values.
     */
    wrenSetSlotNewList(vm, 0);
    wrenObjectListMethods(vm, 1, 0, 1);
}

/* TODO: Push this into `wrench.h`.
 */
#ifndef stderr
#define stderr wrench_stderr
#endif

//#include <wren/src/vm/wren_common.h>
//#include <wren/src/vm/wren_compiler.h>
//#include <wren/src/vm/wren_core.h>
//#include <wren/src/vm/wren_debug.h>
//#include <wren/src/vm/wren_math.h>
//#include <wren/src/vm/wren_primitive.h>
#include <wren/src/vm/wren_utils.h>
#include <wren/src/vm/wren_value.h>
#include <wren/src/vm/wren_vm.h>

#ifdef stderr
#undef stderr
#endif

static void vm_WrenVM_getModuleVariable(WrenVM* vm)
{
    #if WRENCH_DEBUG
    {
        vm_WrenVM* self = (vm_WrenVM*)wrenGetSlotForeign(vm, 0);
        WRENCH_CHECK_MAGIC_TAG(self, vm, WrenVM);

        wrench_assert(self->vm == vm, "TODO");
    }
    #endif /* WRENCH_DEBUG */

    const char* moduleName = wrenGetSlotString(vm, 1);
    const char* variableName = wrenGetSlotString(vm, 2);

    /* Allocate an ObjString to act as the map key. Garbage collection is safe
     * here, because the module & var strings are kept "live" in slots 1 and 2.
     */
    Value moduleNameValue = wrenNewStringLength(vm, moduleName, wrench_strlen(moduleName));

    /* Attempt to fetch the module from the Wren VM's internal module registry.
     */
    Value moduleValue = wrenMapGet(vm->modules, moduleNameValue);

    if (IS_UNDEFINED(moduleValue))
    {
        wrenSetSlotNull(vm, 0);
        return;
    }

    ObjModule* module = AS_MODULE(moduleValue);

    /* Search the module's symbol table for the variable name.
     */
    int symbol = wrenSymbolTableFind(&module->variableNames, variableName, wrench_strlen(variableName));

    if (symbol == -1)
    {
        wrenSetSlotNull(vm, 0);
        return;
    }

    vm->apiStack[0] = module->variables.data[symbol];
}

static void vm_WrenVM_currentModuleName(WrenVM* vm)
{
    vm_WrenVM* self = (vm_WrenVM*)wrenGetSlotForeign(vm, 0);
    WRENCH_CHECK_MAGIC_TAG(self, vm, WrenVM);

    const char* name = wrenCurrentModuleName(self->vm);

    if (name != NULL)
    {
        wrenSetSlotString(vm, 0, name);
    }
    else
    {
        wrenSetSlotNull(vm, 0);
    }
}

/* TODO: Push this into `wrench.h`. Create the list here, append in wrench func.
 */
static void vm_WrenVM_callStack_(WrenVM* vm)
{
    #if WRENCH_DEBUG
    {
        vm_WrenVM* self = (vm_WrenVM*)wrenGetSlotForeign(vm, 0);
        WRENCH_CHECK_MAGIC_TAG(self, vm, WrenVM);

        wrench_assert(self->vm == vm, "TODO");
    }
    #endif /* WRENCH_DEBUG */

    ObjFiber* fiber = vm->fiber;
    wrenSetSlotNewList(vm, 0);

    wrench_assert(fiber != NULL, "");
    wrench_assert(fiber->numFrames != 0, "");

    for (int i = fiber->numFrames - 1; i >= 0; i--)
    {
        CallFrame* frame = &fiber->frames[i];
        ObjFn* fn = frame->closure->fn;

        int instruction = (int)(frame->ip - fn->code.data) - 1;

        if (instruction < 0 || instruction >= fn->code.count)
        {
            continue;
        }

        int line = fn->debug->sourceLines.data[instruction];

        const char* moduleName = (fn->module != NULL && fn->module->name != NULL) ? fn->module->name->value : "<unknown module>";
        const char* fnName = (fn->debug != NULL && fn->debug->name) ? fn->debug->name : "<unknown or foreign function>";

        char buffer[1024];
        wrench_snprintf(buffer, sizeof(buffer), "%s (file \"%s\" line %i)", fnName, moduleName, line);

        wrenSetSlotString(vm, 1, (const char*)buffer);
        wrenInsertInList(vm, 0, -1, 1);
    }
}

static void vm_WrenVM_signatureArity(WrenVM* vm)
{
    wrenSetSlotInt(vm, 0, wrenSignatureArity(wrenGetSlotString(vm, 1)));
}

static void vm_WrenVM_callGetter(WrenVM* vm)
{
    #if WRENCH_DEBUG
    {
        vm_WrenVM* self = (vm_WrenVM*)wrenGetSlotForeign(vm, 0);
        WRENCH_CHECK_MAGIC_TAG(self, vm, WrenVM);

        wrench_assert(self->vm == vm, "TODO");
    }
    #endif /* WRENCH_DEBUG */

    const char* name = wrenGetSlotString(vm, 2);
    WrenHandle* method = wrenMakeCallHandle(vm, name);

    if (method == NULL)
    {
        char error[1024];
        wrench_snprintf(error, sizeof(error), "Failed to create call handle for \"%s\" getter!", name);

        wrenSetSlotString(vm, 0, (const char*)error);
        wrenAbortFiber(vm, 0);

        return;
    }

    /* `wrenCall` expects the receiver in slot 0.
     */
    WrenHandle* object = wrenGetSlotHandle(vm, 1);
    wrenSetSlotHandle(vm, 0, object);

    switch (wrenCall(vm, method))
    {
        // TODO

        default: break;
    }

    wrenReleaseHandle(vm, method);
    wrenReleaseHandle(vm, object);
}

static void vm_WrenVM_callSetter(WrenVM* vm)
{
    #if WRENCH_DEBUG
    {
        vm_WrenVM* self = (vm_WrenVM*)wrenGetSlotForeign(vm, 0);
        WRENCH_CHECK_MAGIC_TAG(self, vm, WrenVM);

        wrench_assert(self->vm == vm, "TODO");
    }
    #endif /* WRENCH_DEBUG */

    char signature[1024];
    const char* name = wrenGetSlotString(vm, 2);

    if (wrench_snprintf(signature, sizeof(signature), "%s=(_)", name) < 0)
    {
        wrenSetSlotString(vm, 0, "Method signature exceeded string buffer size.");
        wrenAbortFiber(vm, 0);

        return;
    }

    WrenHandle* method = wrenMakeCallHandle(vm, (const char*)signature);

    if (method == NULL)
    {
        char error[1024];
        wrench_snprintf(error, sizeof(error), "Failed to create call handle for \"%s\" setter!", name);

        wrenSetSlotString(vm, 0, (const char*)error);
        wrenAbortFiber(vm, 0);

        return;
    }

    WrenHandle* object = wrenGetSlotHandle(vm, 1);
    WrenHandle* value = wrenGetSlotHandle(vm, 3);

    wrenSetSlotHandle(vm, 0, object);
    wrenSetSlotHandle(vm, 1, value);

    switch (wrenCall(vm, method))
    {
        case WREN_RESULT_SUCCESS: break;

        case WREN_RESULT_COMPILE_ERROR:
        case WREN_RESULT_RUNTIME_ERROR:
        {
            wrenSetSlotString(vm, 0, wrenGetErrorString(vm));
            wrenAbortFiber(vm, 0);

            // Let cleanup happen.
            // return;
        }
        break;

        default:
        {
            wrench_assert(0, "");
        }
        break;
    }

    wrenReleaseHandle(vm, method);
    wrenReleaseHandle(vm, object);
    wrenReleaseHandle(vm, value);
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

            // TODO: registerLocalQuitFunction

            // NOTE: We don't expose a `foreignLibraryLoadEnabled` property here,
            // to prevent malicious scripts from escaping sandboxed environments.

            // NOTE: Same with `wrenLibraryLoadEnabled`.

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

            // TODO: setSlotNewForeignFast
            // TODO: getMapEntry

            WREN_METHOD(vm, WrenVM, false, objectHasMethod, "(object, signature)", "(_,_)");
            WREN_METHOD(vm, WrenVM, false, objectCountMethods, "(object)", "(_)");
            WREN_METHOD(vm, WrenVM, false, objectListMethods, "(object)", "(_)");
            WREN_METHOD(vm, WrenVM, false, getModuleVariable, "(moduleName, variableName)", "(_,_)");
            WREN_METHOD(vm, WrenVM, false, currentModuleName, "", "");

            WREN_METHOD(vm, WrenVM, false, callStack_, "(unused)", "(_)");
            WREN_CODE("callStack { callStack_(null) }");

            WREN_METHOD(vm, WrenVM, true, signatureArity, "(signature)", "(_)");

            WREN_METHOD(vm, WrenVM, false, callGetter, "(object, name)", "(_,_)");
            WREN_METHOD(vm, WrenVM, false, callSetter, "(object, name, value)", "(_,_,_)");

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

        // TODO: Preprocessor

        // TODO: Gasket
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
