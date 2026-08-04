/* -----------------------------------------------------------------------------
--- Copyright (c) 2012-2026 Adam Schackart / "AJ Hackman", all rights reserved.
--- Distributed under the BSD license v2 (opensource.org/licenses/BSD-3-Clause)
--------------------------------------------------------------------------------
--- XXX: Not ready, use at your own risk - may have side effects/import issues.
----------------------------------------------------------------------------- */
#ifndef __WRENCH_BYTECODE_H__
#define __WRENCH_BYTECODE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <wren.h>

#ifdef __cplusplus
} /* extern "C" */
#endif

#if !WRENCH_NO_CSTDLIB
    /*
     * For uint8_t.
     */
    #include <stdint.h>
#endif

/*
================================================================================
 * ~~ [ macros ] ~~ *
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

/* Helper for foreign library init and quit functions.
 */
#if !defined(WRENCH_EXPORT)
    #if defined(__cplusplus)
        #if _MSC_VER
            #define WRENCH_EXPORT __declspec(dllexport) extern "C"
        #else
            #define WRENCH_EXPORT extern "C"
        #endif
    #else
        #if _MSC_VER
            #define WRENCH_EXPORT __declspec(dllexport)
        #else
            #define WRENCH_EXPORT extern
        #endif
    #endif
#endif /* !WRENCH_EXPORT */

/* Enables safety checks at a performance cost.
 */
#if !defined(WRENCH_DEBUG)
    #if defined(_DEBUG) || defined(DEBUG) || (defined(__GNUC__) && !defined(__OPTIMIZE__)) || !NDEBUG
        #define WRENCH_DEBUG 1
    #else
        #define WRENCH_DEBUG 0
    #endif
#endif /* !WRENCH_DEBUG */

/*
================================================================================
 * ~~ [ public API ] ~~ *
--------------------------------------------------------------------------------
*/

/* Compiles Wren source code as a single-file module and writes it to a serialized (shelf-stable) bytecode buffer.
 */
WRENCH_DECL(WrenInterpretResult, SerializeBytecodeModule, (WrenVM* vm, const char* moduleName, const char* source, uint8_t** bytes, size_t* length));

/* Loads a serialized bytecode buffer into the VM and executes it.
 */
WRENCH_DECL(WrenInterpretResult, InterpretBytecodeModule, (WrenVM* vm, const char* moduleName, const uint8_t* bytes, size_t length));

#endif /* __WRENCH_BYTECODE_H__ */

/*
================================================================================
 * ~~ [ implementation ] ~~ *
--------------------------------------------------------------------------------
*/

#ifdef WRENCH_BYTECODE_IMPLEMENTATION
/*
 * Enable multiple file inclusions with `WRENCH_BYTECODE_IMPLEMENTATION` for ease of use.
 */
#ifndef __WRENCH_BYTECODE_C__
#define __WRENCH_BYTECODE_C__

/* ===== [ standard library ] =============================================== */

/* Disable MSVC warnings about fopen() etc.
 */
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS 1
#endif

#if !WRENCH_NO_CSTDLIB
    #include <stdio.h>
#endif

#if !_WIN32 && !WRENCH_NO_POSIX_HEADERS
    #include <signal.h>
#endif

#ifndef wrench_abort
#define wrench_abort abort
#endif
#ifndef wrench_malloc
#define wrench_malloc malloc
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
#define wrench_free(x) free((void*)(x))
#endif
#ifndef wrench_fseek
#define wrench_fseek fseek
#endif
#ifndef wrench_ftell
#define wrench_ftell ftell
#endif
#ifndef wrench_fwrite
#define wrench_fwrite fwrite
#endif
#ifndef wrench_memcmp
#define wrench_memcmp memcmp
#endif
#ifndef wrench_memcpy
#define wrench_memcpy memcpy
#endif
#ifndef wrench_realloc
#define wrench_realloc realloc
#endif
/*
 * XXX TODO FIXME: We #undef std(in/err/out) in wrench.h - do the same thing here.
 */
#ifndef wrench_stderr
#define wrench_stderr stderr
#endif
#ifndef wrench_stdout
#define wrench_stdout stdout
#endif
#ifndef wrench_strlen
#define wrench_strlen strlen
#endif

/* ===== [ wren internals ] ================================================= */

#ifndef stderr
#define stderr wrench_stderr
#endif

#include <wren/src/vm/wren_common.h>
#include <wren/src/vm/wren_compiler.h>
#include <wren/src/vm/wren_core.h>
#include <wren/src/vm/wren_debug.h>
#include <wren/src/vm/wren_math.h>
#include <wren/src/vm/wren_primitive.h>
#include <wren/src/vm/wren_utils.h>
#include <wren/src/vm/wren_value.h>
#include <wren/src/vm/wren_vm.h>

#ifdef stderr
#undef stderr
#endif

/* ===== [ structures ] ===================================================== */

typedef struct WrenchByteBuffer
{
    uint8_t* data;
    size_t capacity;
    size_t count;
}
WrenchByteBuffer;

/* ===== [ constants ] ====================================================== */

typedef enum WrenchConstantType
{
    WRENCH_CONSTANT_NULL = 0,
    WRENCH_CONSTANT_FALSE,
    WRENCH_CONSTANT_TRUE,
    WRENCH_CONSTANT_NUM,
    WRENCH_CONSTANT_STRING,
    WRENCH_CONSTANT_FN
}
WrenchConstantType;

/* NOTE: This was written against Wren 0.4.0.
 */
#ifndef WRENCH_BYTECODE_MAGIC
#define WRENCH_BYTECODE_MAGIC "WREN"
#endif
#ifndef WRENCH_BYTECODE_VERSION
#define WRENCH_BYTECODE_VERSION 1
#endif

/* ===== [ utilities ] ====================================================== */

/* Indicates that a function is not implemented.
 */
#if !defined(WRENCH_STUB)
    #if 1
        #define WRENCH_STUB() wrench_fprintf(wrench_stderr, "TODO %s (file \"%s\", line %i)\n", __FUNCTION__, __FILE__, __LINE__)
    #else
        #define WRENCH_STUB() wrench_assert(0, "TODO")
    #endif
#endif /* !WRENCH_STUB */

#if !defined(WRENCH_STRINGIFY)
    #define WRENCH_STRINGIFY(s) _WRENCH_STR_IMPL(s)
    #define _WRENCH_STR_IMPL(s) #s
#endif

#if !defined(wrench_breakpoint)
    #if _WIN32
        extern void __cdecl __debugbreak(void);

        #ifndef wrench_breakpoint
        #define wrench_breakpoint() __debugbreak()
        #endif
    #else
        #ifndef wrench_breakpoint
        #define wrench_breakpoint() raise(SIGTRAP)
        #endif
    #endif
#endif /* !wrench_breakpoint */

#if !defined(wrench_assert)
    #if WRENCH_DEBUG
        #define wrench_assert(cnd, ...) if ((cnd) == 0)                                                     \
        {                                                                                                   \
            wrench_fprintf(wrench_stderr, "assert \"%s\" failed in func \"%s\" (file \"%s\", line %i): ",   \
                                                WRENCH_STRINGIFY(cnd), __FUNCTION__, __FILE__, __LINE__);   \
                                                                                                            \
            wrench_fprintf(wrench_stderr, __VA_ARGS__);                                                     \
            wrench_fprintf(wrench_stderr, "\n");                                                            \
                                                                                                            \
            wrench_breakpoint();                                                                            \
        }
    #else
        #define wrench_assert(cnd, ...) ((void)sizeof(cnd))
    #endif
#endif /* !wrench_assert */

/* TODO: Remove this and write to an error string in the context that could be retrieved later.
 */
#ifndef wrench_error
#define wrench_error(...) do                    \
{                                               \
    wrench_fprintf(wrench_stderr, __VA_ARGS__); \
    wrench_fprintf(wrench_stderr, "\n");        \
                                                \
    wrench_abort();                             \
}                                               \
while (0)

#endif /* wrench_error */

static void wrenchByteBufferInit(WrenchByteBuffer* buffer)
{
    buffer->data = NULL;
    buffer->capacity = 0;
    buffer->count = 0;
}

static void wrenchByteBufferClear(WrenchByteBuffer* buffer)
{
    wrench_free(buffer->data);
    wrenchByteBufferInit(buffer);
}

/* TODO: Call `wrenchByteBufferWriteData`, and do the resizing there.
 */
static bool wrenchByteBufferWriteByte(WrenchByteBuffer* buffer, uint8_t byte)
{
    if (buffer->capacity < buffer->count + 1)
    {
        if (1)
        {
            buffer->capacity = buffer->capacity < 16 ? 16 : ((buffer->capacity * 3) / 2);
        }
        else
        {
            buffer->capacity = buffer->capacity < 16 ? 16 : buffer->capacity * 2;
        }

        buffer->data = (uint8_t*)wrench_realloc(buffer->data, buffer->capacity);

        if (buffer->data == NULL)
        {
            return false;
        }
    }

    buffer->data[buffer->count++] = byte;
    return true;
}

/* TODO: Optimization.
 */
static bool wrenchByteBufferWriteData(WrenchByteBuffer* buffer, const uint8_t* data, size_t length)
{
    for (size_t i = 0; i < length; i++)
    {
        if (!wrenchByteBufferWriteByte(buffer, data[i]))
        {
            return false;
        }
    }

    return true;
}

/* TODO: Optimization.
 */
static bool wrenchByteBufferWriteUnsignedInt32(WrenchByteBuffer* buffer, uint32_t value)
{
    if (!wrenchByteBufferWriteByte(buffer, (uint8_t)(value >> 24))) return false;
    if (!wrenchByteBufferWriteByte(buffer, (uint8_t)(value >> 16))) return false;
    if (!wrenchByteBufferWriteByte(buffer, (uint8_t)(value >>  8))) return false;
    return wrenchByteBufferWriteByte(buffer, (uint8_t)(value));
}

static uint32_t wrenchReadUnsignedInt32(const uint8_t* bytes, size_t* offset)
{
    const uint32_t value = (((uint32_t)bytes[*offset + 0] << 24) |
                            ((uint32_t)bytes[*offset + 1] << 16) |
                            ((uint32_t)bytes[*offset + 2] <<  8) |
                            ((uint32_t)bytes[*offset + 3] <<  0));
    *offset += 4;
    return value;
}

/* TODO: Optimization.
 */
static bool wrenchByteBufferWriteDouble(WrenchByteBuffer* buffer, double value)
{
    const uint64_t bits = wrenDoubleToBits(value);
    if (!wrenchByteBufferWriteByte(buffer, (uint8_t)(bits >> 56))) return false;
    if (!wrenchByteBufferWriteByte(buffer, (uint8_t)(bits >> 48))) return false;
    if (!wrenchByteBufferWriteByte(buffer, (uint8_t)(bits >> 40))) return false;
    if (!wrenchByteBufferWriteByte(buffer, (uint8_t)(bits >> 32))) return false;
    if (!wrenchByteBufferWriteByte(buffer, (uint8_t)(bits >> 24))) return false;
    if (!wrenchByteBufferWriteByte(buffer, (uint8_t)(bits >> 16))) return false;
    if (!wrenchByteBufferWriteByte(buffer, (uint8_t)(bits >>  8))) return false;
    return wrenchByteBufferWriteByte(buffer, (uint8_t)(bits));
}

static double wrenchReadDouble(const uint8_t* bytes, size_t* offset)
{
    uint64_t bits = ((uint64_t)bytes[*offset + 0] << 56) |
                    ((uint64_t)bytes[*offset + 1] << 48) |
                    ((uint64_t)bytes[*offset + 2] << 40) |
                    ((uint64_t)bytes[*offset + 3] << 32) |
                    ((uint64_t)bytes[*offset + 4] << 24) |
                    ((uint64_t)bytes[*offset + 5] << 16) |
                    ((uint64_t)bytes[*offset + 6] <<  8) |
                    ((uint64_t)bytes[*offset + 7] <<  0);

    *offset += 8;
    return wrenDoubleFromBits(bits);
}

/*static uint8_t wrenchIsBigEndian(void)
{
    uint32_t i = 1;
    return (*((uint8_t*)&i) == 1) ? 0 : 1;
}*/

static bool wrenchSerializeFn(WrenchByteBuffer* buffer, ObjFn* fn)
{
    /* Function metadata.
     */
    if (!wrenchByteBufferWriteUnsignedInt32(buffer, (uint32_t)fn->arity))
    {
        return false;
    }

    if (!wrenchByteBufferWriteUnsignedInt32(buffer, (uint32_t)fn->numUpvalues))
    {
        return false;
    }

    if (!wrenchByteBufferWriteUnsignedInt32(buffer, (uint32_t)fn->maxSlots))
    {
        return false;
    }

    /* Instruction stream (code).
     */
    if (!wrenchByteBufferWriteUnsignedInt32(buffer, (uint32_t)fn->code.count))
    {
        return false;
    }

    if (!wrenchByteBufferWriteData(buffer, fn->code.data, fn->code.count))
    {
        return false;
    }

    /* Debug information.
     */
    if (fn->debug != NULL)
    {
        /* Debug name.
         */
        if (fn->debug->name != NULL)
        {
            const uint32_t nameLength = (uint32_t)wrench_strlen(fn->debug->name);

            if (!wrenchByteBufferWriteUnsignedInt32(buffer, nameLength))
            {
                return false;
            }

            if (!wrenchByteBufferWriteData(buffer, (const uint8_t*)fn->debug->name, nameLength))
            {
                return false;
            }
        }
        else
        {
            if (!wrenchByteBufferWriteUnsignedInt32(buffer, 0))
            {
                return false;
            }
        }

        /* Source lines (IntBuffer).
         */
        if (!wrenchByteBufferWriteUnsignedInt32(buffer, (uint32_t)fn->debug->sourceLines.count))
        {
            return false;
        }

        if (fn->debug->sourceLines.count > 0)
        {
            /* Direct copy is safe due to header endianness validation.
             */
            if (!wrenchByteBufferWriteData(buffer, (const uint8_t*)fn->debug->sourceLines.data, fn->debug->sourceLines.count * sizeof(int)))
            {
                return false;
            }
        }
    }
    else
    {
        /* 0-length name.
         */
        if (!wrenchByteBufferWriteUnsignedInt32(buffer, 0))
        {
            return false;
        }

        /* 0 source lines.
         */
        if (!wrenchByteBufferWriteUnsignedInt32(buffer, 0))
        {
            return false;
        }
    }

    /* Constant pool.
     */
    if (!wrenchByteBufferWriteUnsignedInt32(buffer, (uint32_t)fn->constants.count))
    {
        return false;
    }

    for (int i = 0; i < fn->constants.count; i++)
    {
        Value constant = fn->constants.data[i];

        #if !defined(IS_TRUE)
            #if WREN_NAN_TAGGING
                #define IS_TRUE(value) ((value) == TRUE_VAL)
            #else
                #define IS_TRUE(value) ((value).type == VAL_TRUE)
            #endif
        #endif /* IS_TRUE */

        if (IS_NUM(constant))
        {
            if (!wrenchByteBufferWriteByte(buffer, WRENCH_CONSTANT_NUM))
            {
                return false;
            }

            if (!wrenchByteBufferWriteDouble(buffer, AS_NUM(constant)))
            {
                return false;
            }
        }
        else if (IS_STRING(constant))
        {
            if (!wrenchByteBufferWriteByte(buffer, WRENCH_CONSTANT_STRING))
            {
                return false;
            }

            ObjString* string = AS_STRING(constant);
            const uint32_t length = (uint32_t)string->length;

            if (!wrenchByteBufferWriteUnsignedInt32(buffer, length))
            {
                return false;
            }

            if (!wrenchByteBufferWriteData(buffer, (const uint8_t*)string->value, length))
            {
                return false;
            }
        }
        else if (IS_FN(constant))
        {
            if (!wrenchByteBufferWriteByte(buffer, WRENCH_CONSTANT_FN))
            {
                return false;
            }

            /* Recurse to serialize nested closures.
             */
            if (!wrenchSerializeFn(buffer, AS_FN(constant)))
            {
                return false;
            }
        }
        else if (IS_NULL(constant))
        {
            if (!wrenchByteBufferWriteByte(buffer, WRENCH_CONSTANT_NULL))
            {
                return false;
            }
        }
        else if (IS_FALSE(constant))
        {
            if (!wrenchByteBufferWriteByte(buffer, WRENCH_CONSTANT_FALSE))
            {
                return false;
            }
        }
        else if (IS_TRUE(constant))
        {
            if (!wrenchByteBufferWriteByte(buffer, WRENCH_CONSTANT_TRUE))
            {
                return false;
            }
        }
        else
        {
            /* Wren's compiler only places numbers, strings, and functions into the constant pool.
             * If we've hit this, the VM state is corrupted or the compiler behavior has changed.
             */
            wrench_assert(0, "Unsupported constant type encountered in constant pool.");
        }
    }

    return true;
}

static ObjFn* wrenchDeserializeFn(WrenVM* vm, ObjModule* module, const uint8_t* bytes, size_t length, size_t* offset)
{
    const uint32_t arity = wrenchReadUnsignedInt32(bytes, offset);
    const uint32_t numUpvalues = wrenchReadUnsignedInt32(bytes, offset);
    const uint32_t maxSlots = wrenchReadUnsignedInt32(bytes, offset);

    ObjFn* fn = wrenNewFunction(vm, module, maxSlots);

    if (fn == NULL)
    {
        // TODO
    }

    wrenPushRoot(vm, (Obj*)fn);

    fn->arity = arity;
    fn->numUpvalues = numUpvalues;

    /* Instruction stream (code).
     */
    const uint32_t codeCount = wrenchReadUnsignedInt32(bytes, offset);

    if (codeCount > 0)
    {
        fn->code.data = (uint8_t*)wrenReallocate(vm, NULL, 0, codeCount);

        if (fn->code.data == NULL)
        {
            // TODO
        }

        fn->code.capacity = codeCount;
        fn->code.count = codeCount;
        wrench_memcpy(fn->code.data, bytes + *offset, codeCount);
        *offset += codeCount;
    }

    /* Debug information.
     */
    const uint32_t nameLength = wrenchReadUnsignedInt32(bytes, offset);
    char* debugName = NULL;

    if (nameLength > 0)
    {
        debugName = (char*)wrenReallocate(vm, NULL, 0, nameLength + 1);

        if (debugName == NULL)
        {
            // TODO
        }

        wrench_memcpy(debugName, bytes + *offset, nameLength);
        debugName[nameLength] = '\0';
        *offset += nameLength;
    }

    const uint32_t sourceLinesCount = wrenchReadUnsignedInt32(bytes, offset);
    int* sourceLines = NULL;

    if (sourceLinesCount > 0)
    {
        const size_t linesSize = sourceLinesCount * sizeof(int);
        sourceLines = (int*)wrenReallocate(vm, NULL, 0, linesSize);

        if (sourceLines == NULL)
        {
            // TODO
        }

        wrench_memcpy(sourceLines, bytes + *offset, linesSize);
        *offset += linesSize;
    }

    if (debugName != NULL || sourceLinesCount > 0)
    {
        fn->debug = (FnDebug*)wrenReallocate(vm, NULL, 0, sizeof(FnDebug));

        if (fn->debug == NULL)
        {
            // TODO
        }

        fn->debug->name = debugName;
        fn->debug->sourceLines.data = sourceLines;
        fn->debug->sourceLines.capacity = sourceLinesCount;
        fn->debug->sourceLines.count = sourceLinesCount;
    }

    /* Constant pool.
     */
    const uint32_t constantsCount = wrenchReadUnsignedInt32(bytes, offset);

    if (constantsCount > 0)
    {
        fn->constants.data = (Value*)wrenReallocate(vm, NULL, 0, constantsCount * sizeof(Value));

        if (fn->constants.data == NULL)
        {
            // TODO
        }

        fn->constants.capacity = constantsCount;
        fn->constants.count = constantsCount;

        /* Initialize elements to NULL_VAL. If any of the string or function allocations
         * below triggers a GC pass, wrenBlackenFunction will safely skip these slots.
         */
        for (uint32_t i = 0; i < constantsCount; i++)
        {
            fn->constants.data[i] = NULL_VAL;
        }

        for (uint32_t i = 0; i < constantsCount; i++)
        {
            const uint8_t type = bytes[(*offset)++];

            switch ((WrenchConstantType)type)
            {
                case WRENCH_CONSTANT_NULL:
                {
                    fn->constants.data[i] = NULL_VAL;
                }
                break;

                case WRENCH_CONSTANT_FALSE:
                {
                    fn->constants.data[i] = FALSE_VAL;
                }
                break;

                case WRENCH_CONSTANT_TRUE:
                {
                    fn->constants.data[i] = TRUE_VAL;
                }
                break;

                case WRENCH_CONSTANT_NUM:
                {
                    fn->constants.data[i] = NUM_VAL(wrenchReadDouble(bytes, offset));
                }
                break;

                case WRENCH_CONSTANT_STRING:
                {
                    uint32_t strLen = wrenchReadUnsignedInt32(bytes, offset);
                    Value strVal = wrenNewStringLength(vm, (const char*)(bytes + *offset), strLen);

                    // TODO: Check strVal allocation.

                    *offset += strLen;
                    fn->constants.data[i] = strVal;
                }
                break;

                case WRENCH_CONSTANT_FN:
                {
                    ObjFn* nestedFn = wrenchDeserializeFn(vm, module, bytes, length, offset);

                    if (nestedFn == NULL)
                    {
                        // TODO
                    }

                    fn->constants.data[i] = OBJ_VAL(nestedFn);
                }
                break;

                default:
                {
                    wrench_assert(0, "Invalid constant type in bytecode payload.");
                }
                break;
            }
        }
    }

    wrenPopRoot(vm);
    return fn;
}

/* ===== [ public API ] ===================================================== */

WRENCH_IMPL(WrenInterpretResult, SerializeBytecodeModule, (WrenVM* vm, const char* moduleName, const char* source, uint8_t** bytes, size_t* length))
{
    /* Create the module name string object.
     */
    Value nameValue = wrenNewStringLength(vm, moduleName, wrench_strlen(moduleName));

    // TODO: Check nameValue allocation.

    ObjString* nameString = AS_STRING(nameValue);
    wrenPushRoot(vm, (Obj*)nameString);

    /* Fetch or create the module object.
     */
    Value moduleValue = wrenMapGet(vm->modules, nameValue);
    ObjModule* module = NULL;

    if (IS_UNDEFINED(moduleValue))
    {
        module = wrenNewModule(vm, nameString);

        if (module == NULL)
        {
            return WREN_RESULT_COMPILE_ERROR;
        }

        wrenPushRoot(vm, (Obj*)module);
        wrenMapSet(vm, vm->modules, nameValue, OBJ_VAL(module));

        // Implicitly import the core module variables into the new module.
        Value coreModuleValue = wrenMapGet(vm->modules, NULL_VAL);
        ObjModule* coreModule = AS_MODULE(coreModuleValue);

        for (int i = 0; i < coreModule->variables.count; i++)
        {
            wrenDefineVariable( vm, module,
                                coreModule->variableNames.data[i]->value,
                                coreModule->variableNames.data[i]->length,
                                coreModule->variables.data[i], NULL);
        }

        wrenPopRoot(vm);
    }
    else
    {
        module = AS_MODULE(moduleValue);
    }

    /* Compile the source into a top-level function.
     */
    ObjFn* topLevelFn = wrenCompile(vm, module, source, false, true);
    wrenPopRoot(vm);

    if (topLevelFn == NULL)
    {
        return WREN_RESULT_COMPILE_ERROR;
    }

    /* Set up the serialization context.
     */
    WrenchByteBuffer buffer;
    wrenchByteBufferInit(&buffer);

    /* Write binary header.
     */
    if (!wrenchByteBufferWriteData(&buffer, (const uint8_t*)WRENCH_BYTECODE_MAGIC, 4))
    {
        return WREN_RESULT_COMPILE_ERROR;
    }

    if (!wrenchByteBufferWriteUnsignedInt32(&buffer, WRENCH_BYTECODE_VERSION))
    {
        return WREN_RESULT_COMPILE_ERROR;
    }

    /*if (!wrenchByteBufferWriteByte(&buffer, wrenchIsBigEndian()))
    {
        return WREN_RESULT_COMPILE_ERROR;
    }*/

    if (!wrenchByteBufferWriteByte(&buffer, (uint8_t)sizeof(void*)))
    {
        return WREN_RESULT_COMPILE_ERROR;
    }

    /* Write module name (length + string data).
     */
    const uint32_t nameLength = (uint32_t)nameString->length;

    if (!wrenchByteBufferWriteUnsignedInt32(&buffer, nameLength))
    {
        return WREN_RESULT_COMPILE_ERROR;
    }

    if (!wrenchByteBufferWriteData(&buffer, (const uint8_t*)nameString->value, nameLength))
    {
        return WREN_RESULT_COMPILE_ERROR;
    }

    /* Serialize module variable layout.
     */
    if (!wrenchByteBufferWriteUnsignedInt32(&buffer, (uint32_t)module->variableNames.count))
    {
        return WREN_RESULT_COMPILE_ERROR;
    }

    for (int i = 0; i < module->variableNames.count; i++)
    {
        ObjString* varName = module->variableNames.data[i];
        wrench_assert(varName != NULL, "");

        const uint32_t varLen = (uint32_t)varName->length;

        if (!wrenchByteBufferWriteUnsignedInt32(&buffer, varLen))
        {
            return WREN_RESULT_COMPILE_ERROR;
        }

        if (!wrenchByteBufferWriteData(&buffer, (const uint8_t*)varName->value, varLen))
        {
            return WREN_RESULT_COMPILE_ERROR;
        }
    }

    /* Serialize the function graph (instructions, debug info, and constants).
     */
    if (!wrenchSerializeFn(&buffer, topLevelFn))
    {
        return WREN_RESULT_COMPILE_ERROR;
    }

    /* Transfer ownership of the buffer to the caller.
     */
    *bytes = buffer.data;
    *length = buffer.count;

    return WREN_RESULT_SUCCESS;
}

WRENCH_IMPL(WrenInterpretResult, InterpretBytecodeModule, (WrenVM* vm, const char* moduleName, const uint8_t* bytes, size_t length))
{
    if (bytes == NULL || length < 10)
    {
        return WREN_RESULT_COMPILE_ERROR;
    }

    size_t offset = 0;

    /* Check magic number.
     */
    if (wrench_memcmp(bytes, WRENCH_BYTECODE_MAGIC, 4) != 0)
    {
        return WREN_RESULT_COMPILE_ERROR;
    }

    offset += 4;

    /* Check version.
     */
    const uint32_t version = wrenchReadUnsignedInt32(bytes, &offset);

    if (version != WRENCH_BYTECODE_VERSION)
    {
        return WREN_RESULT_COMPILE_ERROR;
    }

    /* Check endianness.
     */
    /*const uint8_t endianness = bytes[offset++];

    if (endianness != wrenchIsBigEndian())
    {
        return WREN_RESULT_COMPILE_ERROR;
    }*/

    /* Check pointer size.
     */
    const uint8_t pointerSize = bytes[offset++];

    if (pointerSize != (uint8_t)sizeof(void*))
    {
        return WREN_RESULT_COMPILE_ERROR;
    }

    /* Read module name.
     */
    const uint32_t nameLength = wrenchReadUnsignedInt32(bytes, &offset);
    Value nameValue = wrenNewStringLength(vm, (const char*)(bytes + offset), nameLength);

    // TODO: Check nameValue allocation.

    offset += nameLength;

    ObjString* nameString = AS_STRING(nameValue);
    wrenPushRoot(vm, (Obj*)nameString);

    /* Fetch or create module.
     */
    Value moduleValue = wrenMapGet(vm->modules, nameValue);
    ObjModule* module = NULL;

    if (IS_UNDEFINED(moduleValue))
    {
        module = wrenNewModule(vm, nameString);

        if (module == NULL)
        {
            return WREN_RESULT_COMPILE_ERROR;
        }

        wrenPushRoot(vm, (Obj*)module);
        wrenMapSet(vm, vm->modules, nameValue, OBJ_VAL(module));

        /* Implicitly import the core module variables into the new module.
         */
        Value coreModuleValue = wrenMapGet(vm->modules, NULL_VAL);
        ObjModule* coreModule = AS_MODULE(coreModuleValue);

        for (int i = 0; i < coreModule->variables.count; i++)
        {
            wrenDefineVariable( vm, module,
                                coreModule->variableNames.data[i]->value,
                                coreModule->variableNames.data[i]->length,
                                coreModule->variables.data[i], NULL);
        }

        wrenPopRoot(vm);
    }
    else
    {
        module = AS_MODULE(moduleValue);
    }

    // Reconstruct module variable layout to match hardcoded bytecode indices.
    const uint32_t varCount = wrenchReadUnsignedInt32(bytes, &offset);

    for (uint32_t i = 0; i < varCount; i++)
    {
        const uint32_t varLen = wrenchReadUnsignedInt32(bytes, &offset);

        /* Allocate a safely null-terminated string. Native linkers and
         * VM hash functions could fail or read garbage without this.
         */
        char* varNameStr = (char*)wrench_malloc(varLen + 1);

        if (varNameStr == NULL)
        {
            return WREN_RESULT_COMPILE_ERROR;
        }

        wrench_memcpy(varNameStr, bytes + offset, varLen);
        varNameStr[varLen] = '\0';

        /* Only define the slot if the core module import didn't already provide it.
         */
        const int symbol = wrenSymbolTableFind(&module->variableNames, varNameStr, varLen);

        if (symbol == -1)
        {
            wrenDefineVariable(vm, module, varNameStr, varLen, NULL_VAL, NULL);
        }

        wrench_free(varNameStr);
        offset += varLen;
    }

    /* Deserialize top-level function.
     */
    ObjFn* topLevelFn = wrenchDeserializeFn(vm, module, bytes, length, &offset);

    if (topLevelFn == NULL)
    {
        return WREN_RESULT_COMPILE_ERROR;
    }

    /* Execute.
     */
    ObjClosure* closure = wrenNewClosure(vm, topLevelFn);

    if (closure == NULL)
    {
        return WREN_RESULT_COMPILE_ERROR;
    }

    wrenPopRoot(vm);

    /* Root the closure to protect it from the GC while we allocate the handle.
     */
    wrenPushRoot(vm, (Obj*)closure);
    WrenHandle* callHandle = wrenMakeCallHandle(vm, "call()");

    if (callHandle == NULL)
    {
        return WREN_RESULT_COMPILE_ERROR;
    }

    /* Set up the API stack. This creates a base fiber if one doesn't exist.
     */
    wrenEnsureSlots(vm, 1);

    /* Inject the top-level closure into slot 0 (the receiver).
     */
    vm->apiStack[0] = OBJ_VAL(closure);
    wrenPopRoot(vm);

    /* Invoking call() on the closure runs it through the standard interpreter loop.
     */
    WrenInterpretResult result = wrenCall(vm, callHandle);
    wrenReleaseHandle(vm, callHandle);

    return result;
}

#if defined(WRENCH_BYTECODE_MAIN)

/* FIXME
 */
#undef wrench_stderr
#undef wrench_stdin
#undef wrench_stdout

#ifndef WRENCH_IMPLEMENTATION
#define WRENCH_IMPLEMENTATION 1

#define WRENCH_HAVE_UTIL 1
#include <wrench.h>
#endif

static uint8_t* wrench_bytecode_read_binary_file(const char* filename, size_t* out_length)
{
    wrench_assert(filename != NULL && filename[0] != '\0', "");

    FILE* file = wrench_fopen(filename, "rb");

    if (file == NULL)
    {
        wrench_error("Could not open \"%s\" for reading.", filename);
        return NULL;
    }

    if (wrench_fseek(file, 0, SEEK_END) != 0)
    {
        wrench_error("Failed to seek to end of file \"%s\".", filename);

        wrench_fclose(file);
        return NULL;
    }

    const long length = wrench_ftell(file);

    if (length < 0)
    {
        wrench_error("Failed to determine file size for \"%s\".", filename);

        wrench_fclose(file);
        return NULL;
    }

    if (wrench_fseek(file, 0, SEEK_SET) != 0)
    {
        wrench_error("Failed to seek to beginning of file \"%s\".", filename);

        wrench_fclose(file);
        return NULL;
    }

    uint8_t* buffer = (uint8_t*)wrench_malloc(length);

    if (buffer == NULL)
    {
        wrench_error("Out of memory allocating %ld bytes.", length);

        wrench_fclose(file);
        return NULL;
    }

    const size_t read_len = wrench_fread(buffer, 1, length, file);

    if (read_len != (size_t)length)
    {
        wrench_error("Failed to read entire file \"%s\". Expected %ld bytes, got %zu.", filename, length, read_len);

        wrench_free(buffer);
        wrench_fclose(file);

        return NULL;
    }

    if (wrench_fclose(file) != 0)
    {
        wrench_error("Failed to close file \"%s\" cleanly.", filename);
    }

    *out_length = (size_t)length;
    return buffer;
}

static bool wrench_bytecode_write_binary_file(const char* filename, const uint8_t* data, size_t length)
{
    wrench_assert(filename != NULL && filename[0] != '\0', "");
    wrench_assert(data != NULL, "");

    FILE* file = wrench_fopen(filename, "wb");

    if (file == NULL)
    {
        wrench_error("Could not open file \"%s\" for writing.", filename);
        return false;
    }

    const size_t written = wrench_fwrite(data, 1, length, file);

    if (written != length)
    {
        // TODO
    }

    if (wrench_fclose(file) != 0)
    {
        // TODO
    }

    return written == length;
}

int WRENCH_BYTECODE_MAIN(int argc, char** argv)
{
    if (argc != 2)
    {
        wrench_fprintf(wrench_stderr, "Usage: %s <file.wren | file.egg>\n", argv[0]);
        return 1;
    }

    const char* filename = argv[1];
    size_t filename_len = wrench_strlen(filename);

    WrenVM* vm = wrenNewExtendedVM(argc, argv, true);
    int exitCode = 0;

    if (filename_len > 5 && strcmp(filename + filename_len - 5, ".wren") == 0)
    {
        /* Read source.
         */
        size_t source_len = 0;
        char* source = (char*)wrench_bytecode_read_binary_file(filename, &source_len);

        if (!source)
        {
            exitCode = 1;
            goto cleanup;
        }

        /* Null-terminate the source string for the compiler.
         */
        source = wrench_realloc(source, source_len + 1);
        source[source_len] = '\0';

        /* Serialize.
         */
        uint8_t* bytecode = NULL;
        size_t bytecode_len = 0;

        /* Pass "main" as the module name for top-level scripts.
         */
        WrenInterpretResult result = wrenSerializeBytecodeModule(vm, "main", source, &bytecode, &bytecode_len);
        wrench_free(source);

        if (result != WREN_RESULT_SUCCESS)
        {
            exitCode = 1;
            goto cleanup;
        }

        /* Write to .egg.
         */
        char out_filename[256];
        snprintf(out_filename, sizeof(out_filename), "%.*s.egg", (int)(filename_len - 5), filename);

        if (!wrench_bytecode_write_binary_file(out_filename, bytecode, bytecode_len))
        {
            exitCode = 1;
        }
        else
        {
            wrench_fprintf(wrench_stdout, "Successfully compiled to %s\n", out_filename);
        }

        wrench_free(bytecode);
    }
    else if (filename_len > 4 && strcmp(filename + filename_len - 4, ".egg") == 0)
    {
        /* Read bytecode.
         */
        size_t bytecode_len = 0;
        uint8_t* bytecode = wrench_bytecode_read_binary_file(filename, &bytecode_len);

        if (!bytecode)
        {
            exitCode = 1;
            goto cleanup;
        }

        /* Interpret.
         */
        WrenInterpretResult result = wrenInterpretBytecodeModule(vm, "main", bytecode, bytecode_len);
        wrench_free(bytecode);

        if (result != WREN_RESULT_SUCCESS)
        {
            exitCode = 1;
        }
    }
    else
    {
        wrench_fprintf(wrench_stderr, "Unrecognized extension. Use .wren to compile, or .egg to execute.\n");
        exitCode = 1;
    }

    cleanup:
    {
        wrenFreeExtendedVM(vm, true);
        return exitCode;
    }
}

#endif /* WRENCH_BYTECODE_MAIN */
#endif /* __WRENCH_BYTECODE_C__ */
#endif /* WRENCH_BYTECODE_IMPLEMENTATION */
