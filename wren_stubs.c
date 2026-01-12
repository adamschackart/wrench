/* -----------------------------------------------------------------------------
--- Copyright (c) 2012-2026 Adam Schackart / "AJ Hackman", all rights reserved.
--- Distributed under the BSD license v2 (opensource.org/licenses/BSD-3-Clause)
----------------------------------------------------------------------------- */

#if !defined(WREN_STUB)
    #if defined(WRENCH_STUB)
        #define WREN_STUB WRENCH_STUB
    #elif 1
        #define WREN_STUB() fprintf(stderr, "TODO %s (file \"%s\", line %i)\n", __FUNCTION__, __FILE__, __LINE__)
    #else
        #define WREN_STUB() ASSERT(0, "TODO")
    #endif
#endif /* !WREN_STUB */

#include <wren.h>

int wrenGetVersionNumber()
{
    WREN_STUB(); return 0;
}

void wrenInitConfiguration(WrenConfiguration* configuration)
{
    WREN_STUB();
}

WrenVM* wrenNewVM(WrenConfiguration* configuration)
{
    WREN_STUB(); return NULL;
}

void wrenFreeVM(WrenVM* vm)
{
    WREN_STUB();
}

void wrenCollectGarbage(WrenVM* vm)
{
    WREN_STUB();
}

WrenInterpretResult wrenInterpret(WrenVM* vm, const char* module, const char* source)
{
    WREN_STUB(); return WREN_RESULT_SUCCESS;
}

WrenHandle* wrenMakeCallHandle(WrenVM* vm, const char* signature)
{
    WREN_STUB(); return NULL;
}

WrenInterpretResult wrenCall(WrenVM* vm, WrenHandle* method)
{
    WREN_STUB(); return WREN_RESULT_SUCCESS;
}

void wrenReleaseHandle(WrenVM* vm, WrenHandle* handle)
{
    WREN_STUB();
}

int wrenGetSlotCount(WrenVM* vm)
{
    WREN_STUB(); return 0;
}

void wrenEnsureSlots(WrenVM* vm, int numSlots)
{
    WREN_STUB();
}

WrenType wrenGetSlotType(WrenVM* vm, int slot)
{
    WREN_STUB(); return WREN_TYPE_UNKNOWN;
}

bool wrenGetSlotBool(WrenVM* vm, int slot)
{
    WREN_STUB(); return false;
}

const char* wrenGetSlotBytes(WrenVM* vm, int slot, int* length)
{
    WREN_STUB();

    if (length != NULL) *length = 0;
    return "";
}

double wrenGetSlotDouble(WrenVM* vm, int slot)
{
    WREN_STUB(); return 0.0;
}

void* wrenGetSlotForeign(WrenVM* vm, int slot)
{
    WREN_STUB(); return NULL;
}

const char* wrenGetSlotString(WrenVM* vm, int slot)
{
    WREN_STUB(); return "";
}

WrenHandle* wrenGetSlotHandle(WrenVM* vm, int slot)
{
    WREN_STUB(); return NULL;
}

void wrenSetSlotBool(WrenVM* vm, int slot, bool value)
{
    WREN_STUB();
}

void wrenSetSlotBytes(WrenVM* vm, int slot, const char* bytes, size_t length)
{
    WREN_STUB();
}

void wrenSetSlotDouble(WrenVM* vm, int slot, double value)
{
    WREN_STUB();
}

void* wrenSetSlotNewForeign(WrenVM* vm, int slot, int classSlot, size_t size)
{
    WREN_STUB(); return NULL;
}

void wrenSetSlotNewList(WrenVM* vm, int slot)
{
    WREN_STUB();
}

void wrenSetSlotNewMap(WrenVM* vm, int slot)
{
    WREN_STUB();
}

void wrenSetSlotNull(WrenVM* vm, int slot)
{
    WREN_STUB();
}

void wrenSetSlotString(WrenVM* vm, int slot, const char* text)
{
    WREN_STUB();
}

void wrenSetSlotHandle(WrenVM* vm, int slot, WrenHandle* handle)
{
    WREN_STUB();
}

int wrenGetListCount(WrenVM* vm, int slot)
{
    WREN_STUB(); return 0;
}

void wrenGetListElement(WrenVM* vm, int listSlot, int index, int elementSlot)
{
    WREN_STUB();
}

void wrenSetListElement(WrenVM* vm, int listSlot, int index, int elementSlot)
{
    WREN_STUB();
}

void wrenInsertInList(WrenVM* vm, int listSlot, int index, int elementSlot)
{
    WREN_STUB();
}

int wrenGetMapCount(WrenVM* vm, int slot)
{
    WREN_STUB(); return 0;
}

bool wrenGetMapContainsKey(WrenVM* vm, int mapSlot, int keySlot)
{
    WREN_STUB(); return false;
}

void wrenGetMapValue(WrenVM* vm, int mapSlot, int keySlot, int valueSlot)
{
    WREN_STUB();
}

void wrenSetMapValue(WrenVM* vm, int mapSlot, int keySlot, int valueSlot)
{
    WREN_STUB();
}

void wrenRemoveMapValue(WrenVM* vm, int mapSlot, int keySlot, int removedValueSlot)
{
    WREN_STUB();
}

void wrenGetVariable(WrenVM* vm, const char* module, const char* name, int slot)
{
    WREN_STUB();
}

bool wrenHasVariable(WrenVM* vm, const char* module, const char* name)
{
    WREN_STUB(); return false;
}

bool wrenHasModule(WrenVM* vm, const char* module)
{
    WREN_STUB(); return false;
}

void wrenAbortFiber(WrenVM* vm, int slot)
{
    WREN_STUB();
}

void* wrenGetUserData(WrenVM* vm)
{
    WREN_STUB(); return NULL;
}

void wrenSetUserData(WrenVM* vm, void* userData)
{
    WREN_STUB();
}
