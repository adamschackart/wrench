/* -----------------------------------------------------------------------------
--- Copyright (c) 2012-2026 Adam Schackart / "AJ Hackman", all rights reserved.
--- Distributed under the BSD license v2 (opensource.org/licenses/BSD-3-Clause)
--------------------------------------------------------------------------------
--- TODO: If RAM <= 4GB, disable async compilation and pass -no-integrated-cpp.
--- Could possibly pass -fno-inline as well (but that affects code generation).
----------------------------------------------------------------------------- */
import "config" for Config
import "file" for File, Path
import "platform" for Platform
import "process" for Process
import "time" for Timer
import "util" for NumUtil, StringUtil
import "vm" for WrenVM

/*
================================================================================
 * ~~ [ utilities ] ~~ *
--------------------------------------------------------------------------------
*/

/* NOTE: We need to wrap these up in a class, or calls in methods can't find them.
 */
class Util {
    /*
     * Enables us to safely embed code (or any other type of text) within a string.
     * XXX: This should go in StringUtil, but we've got enough escape chars here...
     */
    static escapeString(s) {
        return s.replace("\\", "\\\\")
                .replace("\"", "\\\"")
                .replace("\n", "\\n")
                .replace("\r", "\\r")
                .replace("\t", "\\t")
    }

    /* Modifies wren.c with performance improvements and some extra functionality.
     */
    static patchWrenAmalgamation(filename, data) {
        /*
         * Replace Windows-style line separators.
         */
        filename = filename.replace("\\", "/")

        if (filename == "wren/src/vm/wren_compiler.c") {
            if (false) {
                data = data.split("\n").map { |line| line.trimEnd() }.join("\n")
            }

            /* Replace `(v)sprintf`.
             */
            data = [
                "#if WRENCH_USE_STB_SPRINTF && !defined(STB_SPRINTF_IMPLEMENTATION)",
                "    #define STB_SPRINTF_IMPLEMENTATION",
                "    #define STB_SPRINTF_STATIC",
                "    #include <stb/stb_sprintf.h>",
                "#endif",
                "",
                "#if !defined(wrench_snprintf)",
                "    #if defined(STB_SPRINTF_H_INCLUDE)",
                "        #define wrench_snprintf stbsp_snprintf",
                "    #else",
                "        #define wrench_snprintf snprintf",
                "    #endif",
                "#endif",
                "",
                "#if !defined(wrench_vsnprintf)",
                "    #if defined(STB_SPRINTF_H_INCLUDE)",
                "        #define wrench_vsnprintf stbsp_vsnprintf",
                "    #else",
                "        #define wrench_vsnprintf vsnprintf",
                "    #endif",
                "#endif",
                "\n",
            ].join("\n") + data

            data = data.replace("sprintf(message,", "wrench_snprintf(message, sizeof(message),")
            data = data.replace("vsprintf(message + length,", "wrench_vsnprintf(message + length, sizeof(message) - length,")
            data = data.replace("sprintf(label,", "wrench_snprintf(label, sizeof(label),")
            data = data.replace("sprintf(fullSignatureWithPrefix,", "wrench_snprintf(fullSignatureWithPrefix, sizeof(fullSignatureWithPrefix),")

            /* Fix collisions with win32 definitions.
             */
            data = data.replace("TokenType", "WrenTokenType")
            data = data.replace("boolean", "emitBoolean")
        } else if (filename == "wren/src/vm/wren_core.c") {
            if (false) {
                data = data.split("\n").map { |line| line.trimEnd() }.join("\n")
            }

            /* Check RHS for 0 before Num division.
             */
            data = data.replace("DEF_NUM_INFIX(divide,   /,  NUM)",
            [
                "static bool prim_num_divide(WrenVM* vm, Value* args)",
                "{",
                "  if (!validateNum(vm, args[1], \"Right operand\")) return false;",
                "  if (fabs(AS_NUM(args[1])) < 0.000001)",
                "  {",
                "    vm->fiber->error = wrenNewString(vm, \"Division by zero!\");",
                "    return false;",
                "  }",
                "  RETURN_NUM(AS_NUM(args[0]) / AS_NUM(args[1]));",
                "}",
            ].join("\n"))

            var index
            var lines = File.readLines("wren/src/vm/wren_core.wren")

            /* List aliases for those more familiar with Python.
             */
            index = lines.indexOf("class List is Sequence {")

            if (index < 0) {
                Fiber.abort("failed to patch wren core for List.append and List.extend")
            } else {
                [
                    "  append(item) { add(item) }",
                    "  extend(list) { addAll(list) }",
                    "",
                    "  pop(x) { removeAt(x) }",
                    "  pop() { removeAt(-1) }",
                    "",
                ][-1..0].each { |line| lines.insert(index + 1, line) }
            }

            /* Sequence.find, thanks to Michael Hermier. Don't call inside Wrench,
             * as we must be able to rely on vanilla Wren to bootstrap the build.
             * If we really need it for whatever reason, create SequenceUtil.find.
             *
             * "Returns an iterator on the sequence that pass the function `predicate`,
             * starting from the begining of the sequence or `it` if provided.
             *
             * It is a runtime error if `it` is not a valid iterator value on the sequence."
             */
            index = lines.indexOf("  isEmpty { iterate(null) ? false : true }")

            if (index < 0) {
                Fiber.abort("failed to patch wren core for Sequence.find")
            } else {
                [
                    "  find(predicate) { find(iterate(null), predicate) }",
                    "",
                    "  find(it, predicate) {",
                    "    while(it) {",
                    "      if (predicate.call(iteratorValue(it))) break",
                    "      it = iterate(it)",
                    "    }",
                    "    return it",
                    "}",
                    "",
                ][-1..0].each { |line| lines.insert(index, line) }
            }

            // TODO: findIndex

            /* Faster Sequence.where (removes redundant call to iteratorValue), thanks to Thorben Krüger.
             */
            index = lines.indexOf("class WhereSequence is Sequence {")

            if (index < 0) {
                Fiber.abort("failed to patch wren core for faster Sequence.where")
            } else {
                [
                    "    _cache_val = null",
                    "    _cache_iter = null"
                ][-1..0].each { |line| lines.insert(index + 4, line) }

                lines[index + 10] = "      var val = _sequence.iteratorValue(iterator)"

                [
                    "      if (_fn.call(val)) {",
                    "        _cache_val = val",
                    "        _cache_iter = iterator",
                    "        break",
                    "      }",
                ][-1..0].each { |line| lines.insert(index + 11, line) }

                lines[index + 20] = "  iteratorValue(iterator) {"

                [
                    "    if (iterator == _cache_iter) return _cache_val else return _sequence.iteratorValue(iterator)",
                    "  }",
                ][-1..0].each { |line| lines.insert(index + 21, line) }
            }

            /* Faster String.isEmpty.
             */
            index = lines.indexOf("class String is Sequence {")

            if (index < 0) {
                Fiber.abort("failed to patch wren core for faster String.isEmpty")
            } else {
                [
                    "  isEmpty { byteCount_ == 0 }",
                    "",
                ][-1..0].each { |line| lines.insert(index + 1, line) }
            }

            /* Sequence.none and Python-like aliases.
             */
            index = lines.indexOf("class Sequence {")

            if (index < 0) {
                Fiber.abort("failed to patch wren core for Sequence.none")
            } else {
                [
                    "  filter(f) { where(f) }",
                    "  apply(f) { each(f) }",
                    "",
                    "  none(f) {",
                    "    for (element in this) {",
                    "      if (f.call(element)) return false",
                    "    }",
                    "",
                    "    return true",
                    "  }",
                    "",
                ][-1..0].each { |line| lines.insert(index + 1, line) }
            }

            /* Headerize core module source.
             */
            data = data.replace("#include \"wren_core.wren.inc\"",

            "static const char* coreModuleSource =\n" +
            lines.map { |line| "\"" + Util.escapeString(line) + "\\n\"" }.join("\n") +
            ";")
        } else if (filename == "wren/src/vm/wren_primitive.h") {
            if (false) {
                data = data.split("\n").map { |line| line.trimEnd() }.join("\n")
            }

            /* Fix collisions with Objective-C definitions.
             */
            data = data.replace(" Method", " WrenMethod")
        } else if (filename == "wren/src/vm/wren_value.h") {
            if (false) {
                data = data.split("\n").map { |line| line.trimEnd() }.join("\n")
            }

            /* Fix collisions with Objective-C definitions.
             */
            data = data.replace(" Method", " WrenMethod")
            data = data.replace("(Method", "(WrenMethod")
        } else if (filename == "wren/src/vm/wren_value.c") {
            if (false) {
                data = data.split("\n").map { |line| line.trimEnd() }.join("\n")
            }

            /* Lazy + faster string hashing.
             */
            data = data.replace([
                "// Calculates and stores the hash code for [string].",
                "static void hashString(ObjString* string)",
                "{",
                "  // FNV-1a hash. See: http://www.isthe.com/chongo/tech/comp/fnv/",
                "  uint32_t hash = 2166136261u;",
                "",
                "  // This is O(n) on the length of the string, but we only call this when a new",
                "  // string is created. Since the creation is also O(n) (to copy/initialize all",
                "  // the bytes), we allow this here.",
                "  for (uint32_t i = 0; i < string->length; i++)",
                "  {",
                "    hash ^= string->value[i];",
                "    hash *= 16777619;",
                "  }",
                "",
                "  string->hash = hash;",
                "}"].join("\n"),

                "static void hashString(ObjString* string)\n{\n  string->hash = 0;\n}")

            data = data.replace("aString->hash == bString->hash", "true")

            data = data.replace("      return ((ObjString*)object)->hash;",
            [
                "{",
                "  ObjString* string = (ObjString*)object;",
                "",
                "  if (string->hash == 0)",
                "  {",
                "    uint32_t hash = 0;",
                "",
                "    for (uint32_t i = 0; i < string->length; i++)",
                "    {",
                "      #if __TINYC__",
                "      {",
                "        hash = ((hash << 7) | (hash >> (32 - 7))) + string->value[i];",
                "      }",
                "      #elif _MSC_VER",
                "      {",
                "        hash = _rotl(hash, 7) + string->value[i];",
                "      }",
                "      #elif __GNUC__ && defined(__has_builtin) && __has_builtin(__builtin_rotateleft32)",
                "      {",
                "        hash = __builtin_rotateleft32(hash, 7) + string->value[i];",
                "      }",
                "      #else",
                "      {",
                "        hash = ((hash << 7) | (hash >> (32 - 7))) + string->value[i];",
                "      }",
                "      #endif",
                "    }",
                "",
                "    // Final mixing step.",
                "    hash += hash >> 16;",
                "",
                "    string->hash = hash;",
                "  }",
                "",
                "  return string->hash;",
                "}",
            ].map { |line| "  " * 2 + line }.join("\n"))

            /* Better hash combination.
             */
            data = data.replace("// Generates a hash code for [object].",
            [
                "static inline uint32_t hashTwoNumbers(double a, double b)",
                "{",
                "  uint32_t lhs = hashNumber(a);",
                "  uint32_t rhs = hashNumber(b);",
                "",
                "  lhs ^= rhs + 0x9e3779b9 + (lhs << 6) + (lhs >> 2);",
                "  return lhs;",
                "}",
                "",
                "// Generates a hash code for [object].",
            ].join("\n"))

            data = data.replace("hashNumber(fn->arity) ^ hashNumber(fn->code.count)", "hashTwoNumbers(fn->arity, fn->code.count)")
            data = data.replace("hashNumber(range->from) ^ hashNumber(range->to)", "hashTwoNumbers(range->from, range->to)")

            /* Replace an `sprintf`.
             */
            data = data.replace("sprintf(buffer,", "wrench_snprintf(buffer, sizeof(buffer),")

            /* Fix collisions with Objective-C definitions.
             */
            data = data.replace(" Method", " WrenMethod")
            data = data.replace("(Method", "(WrenMethod")

            data = data.replace("wrenMethod", "wrenWrenMethod")
        } else if (filename == "wren/src/vm/wren_vm.c") {
            if (false) {
                data = data.split("\n").map { |line| line.trimEnd() }.join("\n")
            }

            /* Stack overflow protection.
             */
            data = data.replace("      // If the class's method table doesn't include the symbol, bail.",
            [
                "      if (fiber->numFrames >= 128) {",
                "        fiber->error = wrenNewString(vm, \"Stack overflow!\");",
                "        RUNTIME_ERROR();",
                "      }",
                "",
                "      // If the class's method table doesn't include the symbol, bail."
            ].join("\n"))

            /* Fix collisions with Objective-C definitions.
             */
            data = data.replace(" Method", " WrenMethod")
        }

        /* Patch to enable any source file to be overridden.
         */
        if (filename.endsWith(".c")) {
            var s = StringUtil.toUpper(Path.split(filename)[-1][0...-".c".count]) + "_C_DEFINED"
            data = "#ifndef %(s)\n\n%(data)\n#define %(s) 1\n#endif\n\n"
        }

        return patchWrenAmalgamationForSymbolTableReplacement_(filename, data)
    }

    /* Enable replacement of O(n) SymbolTable. Bring your own hashtable or cache.
     *
     * XXX: ObjModule embeds a SymbolTable inline, and at 64 bytes ObjModule
     * fits exactly in one x86 cache line. Adding additional fields here pushes
     * ObjModule past 64 bytes, and causes measurable performance regressions
     * (~5-14%) due to extra cache line fetches on every module access.
     */
    static patchWrenAmalgamationForSymbolTableReplacement_(filename, data) {
        if (filename == "wren/src/vm/wren_utils.h") {
            if (false) {
                data = data.split("\n").map { |line| line.trimEnd() }.join("\n")
            }

            data = data.replace([
                "// TODO: Change this to use a map.",
                "typedef StringBuffer SymbolTable;",
            ].join("\n"),
            [
                "#ifndef WREN_SYMBOL_TABLE_DEFINED",
                "typedef StringBuffer SymbolTable;",
                "#endif",
                "",
                "// Get the total number of symbols in the table.",
                "int wrenSymbolTableCount(SymbolTable* symbols);",
                "",
                "// Get symbol by index.",
                "ObjString* wrenSymbolTableGet(SymbolTable* symbols, int index);",
            ].join("\n"))
        } else if (filename == "wren/src/vm/wren_utils.c") {
            if (false) {
                data = data.split("\n").map { |line| line.trimEnd() }.join("\n")
            }

            data = data.replace(
                "DEFINE_BUFFER(String, ObjString*);",
            [
                "DEFINE_BUFFER(String, ObjString*);",
                "",
                "#ifndef WREN_SYMBOL_TABLE_DEFINED",
            ].join("\n"))

            data = data.replace(
                "int wrenUtf8EncodeNumBytes(int value)",
            [
                "int wrenSymbolTableCount(SymbolTable* symbols)",
                "{",
                "  return symbols->count;",
                "}",
                "",
                "ObjString* wrenSymbolTableGet(SymbolTable* symbols, int index)",
                "{",
                "  ASSERT(index >= 0, \"Negative symbol index.\");",
                "  ASSERT(index < symbols->count, \"Symbol index too high.\");",
                "",
                "  return symbols->data[index];",
                "}",
                "",
                "#endif // !WREN_SYMBOL_TABLE_DEFINED",
                "",
                "int wrenUtf8EncodeNumBytes(int value)",
            ].join("\n"))
        } else if (filename == "wren/src/vm/wren_compiler.c") {
            if (false) {
                data = data.split("\n").map { |line| line.trimEnd() }.join("\n")
            }

            data = data.replace(
                "classInfo.fields.count",
                "wrenSymbolTableCount(&classInfo.fields)")

            data = data.replace(
                "parser.module->variableNames.data[i]",
                "wrenSymbolTableGet(&parser.module->variableNames, i)")
        } else if (filename == "wren/src/vm/wren_vm.c") {
            if (false) {
                data = data.split("\n").map { |line| line.trimEnd() }.join("\n")
            }

            data = data.replace(
                "vm->methodNames.count",
                "wrenSymbolTableCount(&vm->methodNames)")

            data = data.replace(
                "vm->methodNames.data[symbol]",
                "wrenSymbolTableGet(&vm->methodNames, symbol)")

            data = data.replace(
                "coreModule->variableNames.data[i]",
                "wrenSymbolTableGet(&coreModule->variableNames, i)")
        } else if (filename == "wren/src/vm/wren_debug.c") {
            if (false) {
                data = data.split("\n").map { |line| line.trimEnd() }.join("\n")
            }

            data = data.replace(
                "fn->module->variableNames.data[slot]",
                "wrenSymbolTableGet(&fn->module->variableNames, slot)")

            data = data.replace(
                "vm->methodNames.data[symbol]",
                "wrenSymbolTableGet(&vm->methodNames, symbol)")
        } else if (filename == "wren/src/optional/wren_opt_meta.c") {
            if (false) {
                data = data.split("\n").map { |line| line.trimEnd() }.join("\n")
            }

            data = data.replace(
                "module->variableNames.count",
                "wrenSymbolTableCount(&module->variableNames)")

            data = data.replace(
                "module->variableNames.data[i]",
                "wrenSymbolTableGet(&module->variableNames, i)")
        }

        return patchWrenAmalgamationForProfileMarkers_(filename, data)
    }

    static patchWrenAmalgamationForProfileMarkers_(filename, data) {
        if (filename == "wren/src/vm/wren_core.c") {
            if (false) {
                data = data.split("\n").map { |line| line.trimEnd() }.join("\n")
            }

            // TODO
        } else if (filename == "wren/src/vm/wren_primitive.c") {
            if (false) {
                data = data.split("\n").map { |line| line.trimEnd() }.join("\n")
            }

            // TODO
        } else if (filename == "wren/src/vm/wren_value.c") {
            if (false) {
                data = data.split("\n").map { |line| line.trimEnd() }.join("\n")
            }

            // TODO
        } else if (filename == "wren/src/vm/wren_vm.c") {
            if (false) {
                data = data.split("\n").map { |line| line.trimEnd() }.join("\n")
            }

            data = data.replace([
                "static ObjUpvalue* captureUpvalue(WrenVM* vm, ObjFiber* fiber, Value* local)",
                "{",
            ].join("\n"),
            [
                "#ifndef WRENCH_PROFILE_SCOPE",
                "#define WRENCH_PROFILE_SCOPE() ((void)0)",
                "#endif",
                "static ObjUpvalue* captureUpvalue(WrenVM* vm, ObjFiber* fiber, Value* local)",
                "{",
                "  WRENCH_PROFILE_SCOPE();",
                "",
            ].join("\n"))

            data = data.replace([
                "static void closeUpvalues(ObjFiber* fiber, Value* last)",
                "{",
            ].join("\n"),
            [
                "static void closeUpvalues(ObjFiber* fiber, Value* last)",
                "{",
                "  WRENCH_PROFILE_SCOPE();",
                "",
            ].join("\n"))

            data = data.replace([
                "static WrenForeignMethodFn findForeignMethod(WrenVM* vm,",
                "                                             const char* moduleName,",
                "                                             const char* className,",
                "                                             bool isStatic,",
                "                                             const char* signature)",
                "{",
            ].join("\n"),
            [
                "static WrenForeignMethodFn findForeignMethod(WrenVM* vm,",
                "                                             const char* moduleName,",
                "                                             const char* className,",
                "                                             bool isStatic,",
                "                                             const char* signature)",
                "{",
                "  WRENCH_PROFILE_SCOPE();",
                "",
            ].join("\n"))

            data = data.replace([
                "static void bindMethod(WrenVM* vm, int methodType, int symbol,",
                "                       ObjModule* module, ObjClass* classObj, Value methodValue)",
                "{",
            ].join("\n"),
            [
                "static void bindMethod(WrenVM* vm, int methodType, int symbol,",
                "                       ObjModule* module, ObjClass* classObj, Value methodValue)",
                "{",
                "  WRENCH_PROFILE_SCOPE();",
                "",
            ].join("\n"))

            data = data.replace([
                "static void callForeign(WrenVM* vm, ObjFiber* fiber,",
                "                        WrenForeignMethodFn foreign, int numArgs)",
                "{",
            ].join("\n"),
            [
                "static void callForeign(WrenVM* vm, ObjFiber* fiber,",
                "                        WrenForeignMethodFn foreign, int numArgs)",
                "{",
                "  WRENCH_PROFILE_SCOPE();",
                "",
            ].join("\n"))

            data = data.replace([
                "static ObjModule* getModule(WrenVM* vm, Value name)",
                "{",
            ].join("\n"),
            [
                "static ObjModule* getModule(WrenVM* vm, Value name)",
                "{",
                "  WRENCH_PROFILE_SCOPE();",
                "",
            ].join("\n"))

            data = data.replace([
                "static ObjClosure* compileInModule(WrenVM* vm, Value name, const char* source,",
                "                                   bool isExpression, bool printErrors)",
                "{",
            ].join("\n"),
            [
                "static ObjClosure* compileInModule(WrenVM* vm, Value name, const char* source,",
                "                                   bool isExpression, bool printErrors)",
                "{",
                "  WRENCH_PROFILE_SCOPE();",
                "",
            ].join("\n"))

            data = data.replace([
                "static Value validateSuperclass(WrenVM* vm, Value name, Value superclassValue,",
                "                                int numFields)",
                "{",
            ].join("\n"),
            [
                "static Value validateSuperclass(WrenVM* vm, Value name, Value superclassValue,",
                "                                int numFields)",
                "{",
                "  WRENCH_PROFILE_SCOPE();",
                "",
            ].join("\n"))

            data = data.replace([
                "static void bindForeignClass(WrenVM* vm, ObjClass* classObj, ObjModule* module)",
                "{",
            ].join("\n"),
            [
                "static void bindForeignClass(WrenVM* vm, ObjClass* classObj, ObjModule* module)",
                "{",
                "  WRENCH_PROFILE_SCOPE();",
                "",
            ].join("\n"))

            data = data.replace([
                "static void endClass(WrenVM* vm) ",
                "{",
            ].join("\n"),
            [
                "static void endClass(WrenVM* vm)",
                "{",
                "  WRENCH_PROFILE_SCOPE();",
                "",
            ].join("\n"))

            data = data.replace([
                "static void createClass(WrenVM* vm, int numFields, ObjModule* module)",
                "{",
            ].join("\n"),
            [
                "static void createClass(WrenVM* vm, int numFields, ObjModule* module)",
                "{",
                "  WRENCH_PROFILE_SCOPE();",
                "",
            ].join("\n"))

            data = data.replace([
                "static void createForeign(WrenVM* vm, ObjFiber* fiber, Value* stack)",
                "{",
            ].join("\n"),
            [
                "static void createForeign(WrenVM* vm, ObjFiber* fiber, Value* stack)",
                "{",
                "  WRENCH_PROFILE_SCOPE();",
                "",
            ].join("\n"))

            data = data.replace([
                "void wrenFinalizeForeign(WrenVM* vm, ObjForeign* foreign)",
                "{",
            ].join("\n"),
            [
                "void wrenFinalizeForeign(WrenVM* vm, ObjForeign* foreign)",
                "{",
                "  WRENCH_PROFILE_SCOPE();",
                "",
            ].join("\n"))

            data = data.replace([
                "static Value resolveModule(WrenVM* vm, Value name)",
                "{",
            ].join("\n"),
            [
                "static Value resolveModule(WrenVM* vm, Value name)",
                "{",
                "  WRENCH_PROFILE_SCOPE();",
                "",
            ].join("\n"))

            data = data.replace([
                "static Value importModule(WrenVM* vm, Value name)",
                "{",
            ].join("\n"),
            [
                "static Value importModule(WrenVM* vm, Value name)",
                "{",
                "  WRENCH_PROFILE_SCOPE();",
                "",
            ].join("\n"))

            data = data.replace([
                "static Value getModuleVariable(WrenVM* vm, ObjModule* module,",
                "                               Value variableName)",
                "{",
            ].join("\n"),
            [
                "static Value getModuleVariable(WrenVM* vm, ObjModule* module,",
                "                               Value variableName)",
                "{",
                "  WRENCH_PROFILE_SCOPE();",
                "",
            ].join("\n"))

            data = data.replace([
                "inline static bool checkArity(WrenVM* vm, Value value, int numArgs)",
                "{",
            ].join("\n"),
            [
                "inline static bool checkArity(WrenVM* vm, Value value, int numArgs)",
                "{",
                "  WRENCH_PROFILE_SCOPE();",
                "",
            ].join("\n"))

            data = data.replace([
                "ObjClosure* wrenCompileSource(WrenVM* vm, const char* module, const char* source,",
                "                            bool isExpression, bool printErrors)",
                "{",
            ].join("\n"),
            [
                "ObjClosure* wrenCompileSource(WrenVM* vm, const char* module, const char* source,",
                "                            bool isExpression, bool printErrors)",
                "{",
                "  WRENCH_PROFILE_SCOPE();",
                "",
            ].join("\n"))

            data = data.replace([
                "Value wrenGetModuleVariable(WrenVM* vm, Value moduleName, Value variableName)",
                "{",
            ].join("\n"),
            [
                "Value wrenGetModuleVariable(WrenVM* vm, Value moduleName, Value variableName)",
                "{",
                "  WRENCH_PROFILE_SCOPE();",
                "",
            ].join("\n"))

            data = data.replace([
                "Value wrenFindVariable(WrenVM* vm, ObjModule* module, const char* name)",
                "{",
            ].join("\n"),
            [
                "Value wrenFindVariable(WrenVM* vm, ObjModule* module, const char* name)",
                "{",
                "  WRENCH_PROFILE_SCOPE();",
                "",
            ].join("\n"))

            data = data.replace([
                "int wrenDeclareVariable(WrenVM* vm, ObjModule* module, const char* name,",
                "                        size_t length, int line)",
                "{",
            ].join("\n"),
            [
                "int wrenDeclareVariable(WrenVM* vm, ObjModule* module, const char* name,",
                "                        size_t length, int line)",
                "{",
                "  WRENCH_PROFILE_SCOPE();",
                "",
            ].join("\n"))

            data = data.replace([
                "int wrenDefineVariable(WrenVM* vm, ObjModule* module, const char* name,",
                "                       size_t length, Value value, int* line)",
                "{",
            ].join("\n"),
            [
                "int wrenDefineVariable(WrenVM* vm, ObjModule* module, const char* name,",
                "                       size_t length, Value value, int* line)",
                "{",
                "  WRENCH_PROFILE_SCOPE();",
                "",
            ].join("\n"))

            data = data.replace([
                "void wrenPushRoot(WrenVM* vm, Obj* obj)",
                "{",
            ].join("\n"),
            [
                "void wrenPushRoot(WrenVM* vm, Obj* obj)",
                "{",
                "  WRENCH_PROFILE_SCOPE();",
                "",
            ].join("\n"))

            data = data.replace([
                "void wrenPopRoot(WrenVM* vm)",
                "{",
            ].join("\n"),
            [
                "void wrenPopRoot(WrenVM* vm)",
                "{",
                "  WRENCH_PROFILE_SCOPE();",
                "",
            ].join("\n"))

            data = data.replace([
                "static void validateApiSlot(WrenVM* vm, int slot)",
                "{",
            ].join("\n"),
            [
                "static void validateApiSlot(WrenVM* vm, int slot)",
                "{",
                "  WRENCH_PROFILE_SCOPE();",
                "",
            ].join("\n"))

            data = data.replace([
                "static void setSlot(WrenVM* vm, int slot, Value value)",
                "{",
            ].join("\n"),
            [
                "static void setSlot(WrenVM* vm, int slot, Value value)",
                "{",
                "  WRENCH_PROFILE_SCOPE();",
                "",
            ].join("\n"))
        }

        return patchWrenAmalgamationForReentrancy_(filename, data)
    }

    /* Re-entrancy courtesy of Mattias Ljungström's "Lom" fork of Wren.
     */
    static patchWrenAmalgamationForReentrancy_(filename, data) {
        if (filename == "wren/src/vm/wren_vm.c") {
            if (false) {
                data = data.split("\n").map { |line| line.trimEnd() }.join("\n")
            }

            /* Interpret.
             */
            data = [
                "static WrenInterpretResult runInterpreter(WrenVM* vm, ObjFiber* fiber);",
                "",
                "static WrenInterpretResult wrenInterpretReentrant(WrenVM* vm, ObjClosure* closure)",
                "{",
                "  // Save state that will be modified.",
                "  Value* savedApiStack = vm->apiStack;",
                "  ObjFiber* outerFiber = vm->fiber;",
                "",
                "  wrenPushRoot(vm, (Obj*)closure);",
                "  ObjFiber* fiber = wrenNewFiber(vm, closure);",
                "  wrenPopRoot(vm);",
                "  vm->apiStack = NULL;",
                "",
                "  WrenInterpretResult result = runInterpreter(vm, fiber);",
                "",
                "  // Restore the outer state.",
                "  vm->fiber = outerFiber;",
                "  vm->apiStack = savedApiStack;",
                "",
                "  return result;",
                "}",
                "\n",
            ].join("\n") + data

            data = data.replace([
                "  if (closure == NULL) return WREN_RESULT_COMPILE_ERROR;",
            ].join("\n"),
            [
                "  if (closure == NULL) return WREN_RESULT_COMPILE_ERROR;",
                "",
                "  if (vm->fiber != NULL && vm->fiber->numFrames > 0)",
                "  {",
                "    return wrenInterpretReentrant(vm, closure);",
                "  }",
            ].join("\n"))

            /* Call.
             */
            data = [
                "static WrenInterpretResult runInterpreter(WrenVM* vm, ObjFiber* fiber);",
                "",
                "static WrenInterpretResult wrenCallReentrant(WrenVM* vm, ObjClosure* closure)",
                "{",
                "  // For re-entrant calls, we use a separate fiber to avoid corrupting the outer call's stack state.",
                "  Value* savedApiStack = vm->apiStack;",
                "  ObjFiber* outerFiber = vm->fiber;",
                "",
                "  // Copy arguments from the current apiStack to a new fiber.",
                "  int numArgs = (int)(vm->fiber->stackTop - vm->apiStack);",
                "",
                "  // Create a new fiber for this call. Root both closure and fiber to protect from GC during setup.",
                "  wrenPushRoot(vm, (Obj*)closure);",
                "  ObjFiber* innerFiber = wrenNewFiber(vm, NULL);",
                "  wrenPushRoot(vm, (Obj*)innerFiber);",
                "",
                "  // Copy arguments to the new fiber's stack.",
                "  wrenEnsureStack(vm, innerFiber, numArgs + closure->fn->maxSlots);",
                "  for (int i = 0; i < numArgs; i++)",
                "  {",
                "    innerFiber->stack[i] = savedApiStack[i];",
                "  }",
                "  innerFiber->stackTop = innerFiber->stack + closure->fn->maxSlots;",
                "",
                "  vm->apiStack = NULL;",
                "",
                "  wrenCallFunction(vm, innerFiber, closure, 0);",
                "",
                "  // Pop roots before running - runInterpreter sets vm->fiber which roots it.",
                "  wrenPopRoot(vm); // innerFiber",
                "  wrenPopRoot(vm); // closure",
                "",
                "  WrenInterpretResult result = runInterpreter(vm, innerFiber);",
                "",
                "  // Copy the result back to the caller's stack.",
                "  if (result == WREN_RESULT_SUCCESS && innerFiber->stackTop > innerFiber->stack)",
                "  {",
                "    savedApiStack[0] = innerFiber->stack[0];",
                "  }",
                "",
                "  // Restore outer state.",
                "  vm->fiber = outerFiber;",
                "  vm->apiStack = savedApiStack;",
                "",
                "  return result;",
                "}",
                "\n",
            ].join("\n") + data

            data = data.replace([
                "  ASSERT(vm->apiStack != NULL, \"Must set up arguments for call first.\");",
                "  ASSERT(vm->fiber->numFrames == 0, \"Can not call from a foreign method.\");",
            ].join("\n"),
            [
                "  ASSERT(vm->apiStack != NULL, \"Must set up arguments for call first.\");",
            ].join("\n"))

            data = data.replace([
                "  // Clear the API stack. Now that wrenCall() has control, we no longer need",
            ].join("\n"),
            [
                "  if (vm->fiber->numFrames > 0)",
                "  {",
                "    return wrenCallReentrant(vm, closure);",
                "  }",
                "",
                "  // Clear the API stack. Now that wrenCall() has control, we no longer need",
            ].join("\n"))
        }

        return patchWrenAmalgamationForSwitchedGoto_(filename, data)
    }

    /* Performance improvement for MSVC courtesy of Mattias Ljungström's "Lom" fork of Wren.
     */
    static patchWrenAmalgamationForSwitchedGoto_(filename, data) {
        if (filename == "wren/src/vm/wren_vm.c") {
            if (false) {
                data = data.split("\n").map { |line| line.trimEnd() }.join("\n")
            }

            data = [
                "#ifndef WREN_SWITCHED_GOTO",
                "#define WREN_SWITCHED_GOTO 1",
                "#endif",
                "",
            ].join("\n") + data

            data = data.replace([
                "  #else",
                "",
                "  #define INTERPRET_LOOP",
            ].join("\n"),
            [
                /* Switched goto: each DISPATCH() has its own switch for better branch prediction.
                 * XXX TODO FIXME: Manually put in all cases from wren_opcodes.h - parse header!!!
                 */
                "  #elif WREN_SWITCHED_GOTO",
                "",
                "  typedef int _ensure_all_77_opcodes_are_covered[CODE_END == 76 ? 1 : -1];",
                "",
                "  #define DISPATCH() do \\",
                "  { \\",
                "    DEBUG_TRACE_INSTRUCTIONS(); \\",
                "    switch (instruction = (Code)READ_BYTE()) { \\",
                "      case CODE_CONSTANT: goto lbl_CONSTANT; \\",
                "      case CODE_NULL: goto lbl_NULL; \\",
                "      case CODE_FALSE: goto lbl_FALSE; \\",
                "      case CODE_TRUE: goto lbl_TRUE; \\",
                "      case CODE_LOAD_LOCAL_0: goto lbl_LOAD_LOCAL_0; \\",
                "      case CODE_LOAD_LOCAL_1: goto lbl_LOAD_LOCAL_1; \\",
                "      case CODE_LOAD_LOCAL_2: goto lbl_LOAD_LOCAL_2; \\",
                "      case CODE_LOAD_LOCAL_3: goto lbl_LOAD_LOCAL_3; \\",
                "      case CODE_LOAD_LOCAL_4: goto lbl_LOAD_LOCAL_4; \\",
                "      case CODE_LOAD_LOCAL_5: goto lbl_LOAD_LOCAL_5; \\",
                "      case CODE_LOAD_LOCAL_6: goto lbl_LOAD_LOCAL_6; \\",
                "      case CODE_LOAD_LOCAL_7: goto lbl_LOAD_LOCAL_7; \\",
                "      case CODE_LOAD_LOCAL_8: goto lbl_LOAD_LOCAL_8; \\",
                "      case CODE_LOAD_LOCAL: goto lbl_LOAD_LOCAL; \\",
                "      case CODE_STORE_LOCAL: goto lbl_STORE_LOCAL; \\",
                "      case CODE_LOAD_UPVALUE: goto lbl_LOAD_UPVALUE; \\",
                "      case CODE_STORE_UPVALUE: goto lbl_STORE_UPVALUE; \\",
                "      case CODE_LOAD_MODULE_VAR: goto lbl_LOAD_MODULE_VAR; \\",
                "      case CODE_STORE_MODULE_VAR: goto lbl_STORE_MODULE_VAR; \\",
                "      case CODE_LOAD_FIELD_THIS: goto lbl_LOAD_FIELD_THIS; \\",
                "      case CODE_STORE_FIELD_THIS: goto lbl_STORE_FIELD_THIS; \\",
                "      case CODE_LOAD_FIELD: goto lbl_LOAD_FIELD; \\",
                "      case CODE_STORE_FIELD: goto lbl_STORE_FIELD; \\",
                "      case CODE_POP: goto lbl_POP; \\",
                "      case CODE_CALL_0: goto lbl_CALL_0; \\",
                "      case CODE_CALL_1: goto lbl_CALL_1; \\",
                "      case CODE_CALL_2: goto lbl_CALL_2; \\",
                "      case CODE_CALL_3: goto lbl_CALL_3; \\",
                "      case CODE_CALL_4: goto lbl_CALL_4; \\",
                "      case CODE_CALL_5: goto lbl_CALL_5; \\",
                "      case CODE_CALL_6: goto lbl_CALL_6; \\",
                "      case CODE_CALL_7: goto lbl_CALL_7; \\",
                "      case CODE_CALL_8: goto lbl_CALL_8; \\",
                "      case CODE_CALL_9: goto lbl_CALL_9; \\",
                "      case CODE_CALL_10: goto lbl_CALL_10; \\",
                "      case CODE_CALL_11: goto lbl_CALL_11; \\",
                "      case CODE_CALL_12: goto lbl_CALL_12; \\",
                "      case CODE_CALL_13: goto lbl_CALL_13; \\",
                "      case CODE_CALL_14: goto lbl_CALL_14; \\",
                "      case CODE_CALL_15: goto lbl_CALL_15; \\",
                "      case CODE_CALL_16: goto lbl_CALL_16; \\",
                "      case CODE_SUPER_0: goto lbl_SUPER_0; \\",
                "      case CODE_SUPER_1: goto lbl_SUPER_1; \\",
                "      case CODE_SUPER_2: goto lbl_SUPER_2; \\",
                "      case CODE_SUPER_3: goto lbl_SUPER_3; \\",
                "      case CODE_SUPER_4: goto lbl_SUPER_4; \\",
                "      case CODE_SUPER_5: goto lbl_SUPER_5; \\",
                "      case CODE_SUPER_6: goto lbl_SUPER_6; \\",
                "      case CODE_SUPER_7: goto lbl_SUPER_7; \\",
                "      case CODE_SUPER_8: goto lbl_SUPER_8; \\",
                "      case CODE_SUPER_9: goto lbl_SUPER_9; \\",
                "      case CODE_SUPER_10: goto lbl_SUPER_10; \\",
                "      case CODE_SUPER_11: goto lbl_SUPER_11; \\",
                "      case CODE_SUPER_12: goto lbl_SUPER_12; \\",
                "      case CODE_SUPER_13: goto lbl_SUPER_13; \\",
                "      case CODE_SUPER_14: goto lbl_SUPER_14; \\",
                "      case CODE_SUPER_15: goto lbl_SUPER_15; \\",
                "      case CODE_SUPER_16: goto lbl_SUPER_16; \\",
                "      case CODE_JUMP: goto lbl_JUMP; \\",
                "      case CODE_LOOP: goto lbl_LOOP; \\",
                "      case CODE_JUMP_IF: goto lbl_JUMP_IF; \\",
                "      case CODE_AND: goto lbl_AND; \\",
                "      case CODE_OR: goto lbl_OR; \\",
                "      case CODE_CLOSE_UPVALUE: goto lbl_CLOSE_UPVALUE; \\",
                "      case CODE_RETURN: goto lbl_RETURN; \\",
                "      case CODE_CLOSURE: goto lbl_CLOSURE; \\",
                "      case CODE_CONSTRUCT: goto lbl_CONSTRUCT; \\",
                "      case CODE_FOREIGN_CONSTRUCT: goto lbl_FOREIGN_CONSTRUCT; \\",
                "      case CODE_CLASS: goto lbl_CLASS; \\",
                "      case CODE_END_CLASS: goto lbl_END_CLASS; \\",
                "      case CODE_FOREIGN_CLASS: goto lbl_FOREIGN_CLASS; \\",
                "      case CODE_METHOD_INSTANCE: goto lbl_METHOD_INSTANCE; \\",
                "      case CODE_METHOD_STATIC: goto lbl_METHOD_STATIC; \\",
                "      case CODE_END_MODULE: goto lbl_END_MODULE; \\",
                "      case CODE_IMPORT_MODULE: goto lbl_IMPORT_MODULE; \\",
                "      case CODE_IMPORT_VARIABLE: goto lbl_IMPORT_VARIABLE; \\",
                "      case CODE_END: goto lbl_END; \\",
                "      default: UNREACHABLE(); \\",
                "    } \\",
                "  } while (false)",
                "",
                "  #define INTERPRET_LOOP  DISPATCH();",
                "  #define CASE_CODE(name) lbl_##name",
                "",
                "  #else",
                "",
                "  #define INTERPRET_LOOP",
            ].join("\n"))
        }

        return data
    }

    /* Remove #include "foo.h" statements from wren.c, and insert opcodes + builtin stdlib modules.
     */
    static removeWrenIncludes(data) {
        data = data.replace("#include \"wren_opcodes.h\"", File.read("wren/src/vm/wren_opcodes.h"))

        data = data.replace("#include \"wren_opt_meta.wren.inc\"",
            "const char* metaModuleSource =\n" +
            File.readLines("wren/src/optional/wren_opt_meta.wren").map { |line| "\"" + Util.escapeString(line) + "\\n\"" }.join("\n") +
            ";")

        data = data.replace("#include \"wren_opt_random.wren.inc\"",
            "const char* randomModuleSource =\n" +
            File.readLines("wren/src/optional/wren_opt_random.wren").map { |line| "\"" + Util.escapeString(line) + "\\n\"" }.join("\n") +
            ";")

        data = data.replace("#include \"", "//#include \"")
        return data
    }
}

/*
================================================================================
 * ~~ [ project ] ~~ *
--------------------------------------------------------------------------------
*/

/* The root node of the build tree, keeps list of nodes (can be other projects).
 */
class Project {
    /*
     * TODO: Export to cmake, premake, make, Visual Studio project files, etc.
     * TODO: Only rebuild sources if files are modified (cache modified time).
     */
    construct new(project, name) {
        _project = project
        _name = name != null ? name : (_project != null ? _project.name.toString : "main")

        /* TODO: Check environment variables.
         */
        if (_project != null) {
            _compiler = _project.compiler.toString
            _linker = _project.linker.toString
        } else if (Platform.isWindows) {
            _compiler = "cl"
            _linker = "link"
        } else {
            /*
             * FIXME: GCC "linker input file unused because linking not done" for static libraries.
             */
            _compiler = "clang"

            /* XXX: `ld` is very barebones.
             */
            if (false) {
                _linker = "ld"
            } else {
                _linker = _compiler.toString
            }
        }

        _nodes = _project != null ? _project.nodes.toList : []
        _sources = _project != null ? _project.sources.toList : []
        _includePaths = _project != null ? _project.includePaths.toList : ["."]
        _debug = _project != null ? _project.debug : WrenVM.debug
        _verbose = _project != null ? _project.verbose : true
        _defines = _project != null ? _project.defines.toList : []
        _undefs = _project != null ? _project.undefs.toList : []
        _compilerPrefix = _project != null ? _project.compilerPrefix : ""
        _linkerPrefix = _project != null ? _project.linkerPrefix : ""
        _extraCompilerFlags = _project != null ? _project.extraCompilerFlags.toList : []
        _extraLinkerFlags = _project != null ? _project.extraLinkerFlags.toList : []
        _libraryPaths = _project != null ? _project.libraryPaths.toList : ["."]
        _runtimeLibraryPaths = _project != null ? _project.runtimeLibraryPaths.toList : ["."]
        _extraObjects = _project != null ? _project.extraObjects.toList : []
        _libraries = _project != null ? _project.libraries.toList : []
        _build32bit = _project != null ? _project.build32bit : false
        _addressSanitizer = _project != null ? _project.addressSanitizer : false
        _threadSanitizer = _project != null ? _project.threadSanitizer : false
        _undefinedBehaviorSanitizer = _project != null ? _project.undefinedBehaviorSanitizer : false
        _integerSanitizer = _project != null ? _project.integerSanitizer : false
        _async = _project != null ? _project.async : false
        _enableRTTI = _project != null ? _project.enableRTTI : false
        _enableExceptions = _project != null ? _project.enableExceptions : true
        _isGUI = _project != null ? _project.isGUI : false
        _dynamicCRT = _project != null ? _project.dynamicCRT : false // Bloat code to avoid DLL hell.
        _linkTimeOptimization = _project != null ? _project.linkTimeOptimization : true
        _stripDebugSymbols = _project != null ? _project.stripDebugSymbols : true
        _finalizeCompilerCommandLine = _project != null ? _project.finalizeCompilerCommandLine : null
        _finalizeLinkerCommandLine = _project != null ? _project.finalizeLinkerCommandLine : null
        _warningLevel = _project != null ? _project.warningLevel : 0
        _optimizeForCodeSize = _project != null ? _project.optimizeForCodeSize : true
        _maxAsyncCompileJobs = _project != null ? _project.maxAsyncCompileJobs : Platform.logicalCoreCount
        _printBanners = true

        if (_maxAsyncCompileJobs < 1) {
            _maxAsyncCompileJobs = 1
        }

        _extraCleanFiles = []

        if (_project != null) {
            _project.nodes.add(this)
        }
    }

    // Cannot be changed, as we're in the projects node list.
    project { _project }

    nodes { _nodes }
    nodes=(value) { _nodes = value }

    toString { "%(type)(%(name))" }

    name { _name }
    name=(value) { _name = value }

    printBanners { _printBanners }
    printBanners=(value) { _printBanners = value }

    compiler { _compiler }
    compiler=(value) { _compiler = value }

    linker { _linker }
    linker=(value) { _linker = value }

    sources { _sources }
    sources=(value) { _sources = value }

    includePaths { _includePaths }
    includePaths=(value) { _includePaths = value }

    debug { _debug }
    debug=(value) { _debug = value }

    release { !debug }
    release=(value) { debug = !value }

    verbose { _verbose }
    verbose=(value) { _verbose = value }

    quiet { !_verbose }
    quiet=(value) { _verbose = !value }

    defines { _defines }
    defines=(value) { _defines = value }

    undefs { _undefs }
    undefs=(value) { _undefs = value }

    define(name, value) {
        defines.add([name, value])
    }

    define(name) {
        defines.add(name)
    }

    undefine(name) {
        undefs.add(name)
    }

    isDefined(key) {
        return Project.isDefined_(key, defines)
    }

    isDefinedTrue(key) {
        return Project.isDefinedTrue_(key, defines)
    }

    isDefinedFalse(key) {
        return Project.isDefinedFalse_(key, defines)
    }

    /* NOTE: For prepending cross-compiler prefixes, i.e. "riscv64-linux-gnu-gcc".
     */
    compilerPrefix { _compilerPrefix }
    compilerPrefix=(value) { _compilerPrefix = value }

    linkerPrefix { _linkerPrefix }
    linkerPrefix=(value) { _linkerPrefix = value }

    extraCompilerFlags { _extraCompilerFlags }
    extraCompilerFlags=(value) { _extraCompilerFlags = value }

    extraLinkerFlags { _extraLinkerFlags }
    extraLinkerFlags=(value) { _extraLinkerFlags = value }

    libraryPaths { _libraryPaths }
    libraryPaths=(value) { _libraryPaths = value }

    runtimeLibraryPaths { _runtimeLibraryPaths }
    runtimeLibraryPaths=(value) { _runtimeLibraryPaths = value }

    addLibraryPath(value) {
        return Project.addLibraryPath_(this, value)
    }

    addLibraryPaths(values) {
        for (value in values) {
            addLibraryPath(value)
        }
    }

    build32bit { _build32bit }
    build32bit=(value) { _build32bit = value }

    build64bit { !build32bit }
    build64bit=(value) { build32bit = !value }

    // TODO: linkerScript (ld -T)
    // TODO: noStandardLibrary (/NODEFAULTLIB, use ld, -nostartfiles, -nodefaultlibs, and/or -nostdlib etc.)

    // TODO: extraCompilerFlagsPerFile ({ filename : flags })
    // TODO: extraLinkerFlagsPerFile ({ filename : flags })

    // TODO: bufferGuard (-fstack-protector on Clang/GCC, /GS and/or /RTCs on MSVC)
    // TODO: precompiledHeaders

    addressSanitizer { _addressSanitizer }
    addressSanitizer=(value) { _addressSanitizer = value }

    threadSanitizer { _threadSanitizer }
    threadSanitizer=(value) { _threadSanitizer = value }

    undefinedBehaviorSanitizer { _undefinedBehaviorSanitizer }
    undefinedBehaviorSanitizer=(value) { _undefinedBehaviorSanitizer = value }

    integerSanitizer { _integerSanitizer }
    integerSanitizer=(value) { _integerSanitizer = value }

    warningLevel { _warningLevel }
    warningLevel=(value) { _warningLevel = value }

    optimizeForCodeSize { _optimizeForCodeSize }
    optimizeForCodeSize=(value) { _optimizeForCodeSize = value }

    optimizeForPerformance { !optimizeForCodeSize }
    optimizeForPerformance=(value) { optimizeForCodeSize = !value }

    extraObjects { _extraObjects }
    extraObjects=(value) { _extraObjects = value }

    extraCleanFiles { _extraCleanFiles }
    extraCleanFiles=(value) { _extraCleanFiles = value }

    libraries { _libraries }
    libraries=(value) { _libraries = value }

    async { _async }
    async=(value) { _async = value }

    blocking { !_async }
    blocking=(value) { _async = !value }

    maxAsyncCompileJobs { _maxAsyncCompileJobs }
    maxAsyncCompileJobs=(value) { _maxAsyncCompileJobs = value }

    /* Enable/disable C++ run-time type information.
     */
    enableRTTI { _enableRTTI }
    enableRTTI=(value) { _enableRTTI = value }

    disableRTTI { !enableRTTI }
    disableRTTI=(value) { enableRTTI = !value }

    enableExceptions { _enableExceptions }
    enableExceptions=(value) { _enableExceptions = value }

    disableExceptions { !enableExceptions }
    disableExceptions=(value) { enableExceptions = !value }

    linkTimeOptimization { _linkTimeOptimization }
    linkTimeOptimization=(value) { _linkTimeOptimization = value }

    stripDebugSymbols { _stripDebugSymbols }
    stripDebugSymbols=(value) { _stripDebugSymbols = value }

    isGUI { _isGUI }
    isGUI=(value) { _isGUI = value }

    isCLI { !isGUI }
    isCLI=(value) { isGUI = !value }

    dynamicCRT { _dynamicCRT }
    dynamicCRT=(value) { _dynamicCRT = value }

    staticCRT { !dynamicCRT }
    staticCRT=(value) { dynamicCRT = !value }

    finalizeCompilerCommandLine { _finalizeCompilerCommandLine }
    finalizeCompilerCommandLine=(value) { _finalizeCompilerCommandLine = value }

    finalizeLinkerCommandLine { _finalizeLinkerCommandLine }
    finalizeLinkerCommandLine=(value) { _finalizeLinkerCommandLine = value }

    configure(config) {
        Project.configure_(this, config)
    }

    build() {
        var start_time = Timer.seconds

        for (node in nodes) {
            node.build()
        }

        /* For joining asynchronous tasks, running code after building it, etc.
         */
        for (node in nodes) {
            node.finish("build")
        }

        if (verbose) {
            System.print("%(this).build done in %(Timer.seconds - start_time) seconds.")
        }
    }

    clean() {
        var start_time = Timer.seconds

        for (node in nodes) {
            node.clean()
        }

        for (node in nodes) {
            node.finish("clean")
        }

        for (filename in extraCleanFiles) {
            Path.tryRemove(filename)
        }

        if (verbose) {
            System.print("%(this).clean done in %(Timer.seconds - start_time) seconds.")
        }
    }

    // ===== [ private utils ] =================================================

    static configure_(node, config) {
        //var storeFallbacks = config.storeFallbacks
        //config.storeFallbacks = false

        if (node is Project) {
            node.printBanners = config.getBool("print-banners", node.printBanners)
        }

        node.compiler = config.getStr("compiler", node.compiler)
        node.linker = config.getStr("linker", node.linker)

        node.warningLevel = config.getNum("warning-level", node.warningLevel)
        node.maxAsyncCompileJobs = config.getNum("max-async-compile-jobs", node.maxAsyncCompileJobs)

        node.linkTimeOptimization = config.getBool("link-time-optimization", node.linkTimeOptimization)
        node.linkTimeOptimization = config.getBool("lto", node.linkTimeOptimization)

        node.stripDebugSymbols = config.getBool("strip-debug-symbols", node.stripDebugSymbols)

        node.debug = config.getBool("debug", node.debug)
        node.release = config.getBool("release", node.release)

        node.verbose = config.getBool("verbose", node.verbose)
        node.quiet = config.getBool("quiet", node.quiet)

        node.optimizeForCodeSize = config.getBool("optimize-for-code-size", node.optimizeForCodeSize)
        node.optimizeForCodeSize = config.getBool("optimize-for-size", node.optimizeForCodeSize)

        node.optimizeForPerformance = config.getBool("optimize-for-performance", node.optimizeForPerformance)
        node.optimizeForPerformance = config.getBool("optimize-for-speed", node.optimizeForPerformance)

        node.async = config.getBool("async", node.async)
        node.blocking = config.getBool("blocking", node.blocking)

        node.enableRTTI = config.getBool("enable-rtti", node.enableRTTI)
        node.disableRTTI = config.getBool("disable-rtti", node.disableRTTI)

        node.enableExceptions = config.getBool("enable-exceptions", node.enableExceptions)
        node.disableExceptions = config.getBool("disable-exceptions", node.disableExceptions)

        node.isGUI = config.getBool("gui", node.isGUI)
        node.isCLI = config.getBool("cli", node.isCLI)

        node.dynamicCRT = config.getBool("dynamic-crt", node.dynamicCRT)
        node.staticCRT = config.getBool("static-crt", node.staticCRT)

        node.build32bit = config.getBool("build-32bit", node.build32bit)
        node.build64bit = config.getBool("build-64bit", node.build64bit)

        node.addressSanitizer = config.getBool("address-sanitizer", node.addressSanitizer)
        node.addressSanitizer = config.getBool("asan", node.addressSanitizer)

        node.threadSanitizer = config.getBool("thread-sanitizer", node.threadSanitizer)
        node.threadSanitizer = config.getBool("tsan", node.threadSanitizer)

        node.undefinedBehaviorSanitizer = config.getBool("undefined-behavior-sanitizer", node.undefinedBehaviorSanitizer)
        node.undefinedBehaviorSanitizer = config.getBool("ub-sanitizer", node.undefinedBehaviorSanitizer)
        node.undefinedBehaviorSanitizer = config.getBool("ubsan", node.undefinedBehaviorSanitizer)

        node.integerSanitizer = config.getBool("integer-sanitizer", node.integerSanitizer)
        node.integerSanitizer = config.getBool("int-sanitizer", node.integerSanitizer)
        node.integerSanitizer = config.getBool("isan", node.integerSanitizer)

        // TODO: sources=A,B,C
        // TODO: include-paths=A,B,C
        // TODO: defines=A,B,C
        // TODO: undefs=A,B,C
        // TODO: extra-compiler-flags=A,B,C
        // TODO: extra-linker-flags=A,B,C
        // TODO: extra-objects=A,B,C
        // TODO: libraries=A,B,C
        // TODO: library-paths=A,B,C
        // TODO: runtime-library-paths=A,B,C

        //config.storeFallbacks = storeFallbacks
    }

    static isDefined_(key, list) {
        for (item in list) {
            if (item is String) {
                var s = item.split("=")

                if (s[0] == key) {
                    return true
                }
            } else if (item is List) {
                if (item[0] == key) {
                    return true
                }
            } else {
                Fiber.abort("TODO")
            }
        }

        return false
    }

    static isDefinedTrue_(key, list) {
        for (item in list) {
            if (item is String) {
                var s = item.split("=")

                if (s[0] != key) {
                    continue
                }

                if (s.count > 1 && s[1] == "0") {
                    return false
                }

                return true
            } else if (item is List) {
                if (item[0] != key) {
                    continue
                }

                if (item[1] == 0 || item[1] == "0") {
                    return false
                }

                return true
            } else {
                Fiber.abort("TODO")
            }
        }

        return false
    }

    static isDefinedFalse_(key, list) {
        Fiber.abort("TODO")
    }

    static addLibraryPath_(node, path) {
        node.libraryPaths.add(path)

        if (!Platform.isWindows) {
            node.runtimeLibraryPaths.add(path)
        }
    }

    ensureVisualStudioCompilerSetup_() {
        /*
         * Find/run vcvarsall so we don't have to use Developer Command Prompt on Win32.
         * This will also enable us to do 32-bit builds, and cross-compile for ARM etc.
         * TODO: Use "%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
         */
        if (__msvcIsInit) {
            return
        }

        __msvcIsInit = true

        // ===== [ 2026 ] ======================================================

        if (Path.isFile("C:\\Program Files\\Microsoft Visual Studio\\18\\Enterprise\\VC\\Auxiliary\\Build\\vcvarsall.bat")) {
            if (build32bit) {
                Fiber.abort("TODO")
            } else {
                Fiber.abort("TODO")
            }

            return
        }

        if (Path.isFile("C:\\Program Files\\Microsoft Visual Studio\\18\\Professional\\VC\\Auxiliary\\Build\\vcvarsall.bat")) {
            if (build32bit) {
                Fiber.abort("TODO")
            } else {
                Fiber.abort("TODO")
            }

            return
        }

        if (Path.isFile("C:\\Program Files\\Microsoft Visual Studio\\18\\Community\\VC\\Auxiliary\\Build\\vcvarsall.bat")) {
            if (build32bit) {
                Fiber.abort("TODO")
            } else {
                Fiber.abort("TODO")
            }

            return
        }

        // ===== [ 2022 ] ======================================================

        if (Path.isFile("C:\\Program Files\\Microsoft Visual Studio\\2022\\Enterprise\\VC\\Auxiliary\\Build\\vcvarsall.bat")) {
            if (build32bit) {
                Fiber.abort("TODO")
            } else {
                Fiber.abort("TODO")
            }

            return
        }

        if (Path.isFile("C:\\Program Files\\Microsoft Visual Studio\\2022\\Professional\\VC\\Auxiliary\\Build\\vcvarsall.bat")) {
            if (build32bit) {
                Fiber.abort("TODO")
            } else {
                Fiber.abort("TODO")
            }

            return
        }

        if (Path.isFile("C:\\Program Files\\Microsoft Visual Studio\\2022\\Community\\VC\\Auxiliary\\Build\\vcvarsall.bat")) {
            if (build32bit) {
                Fiber.abort("TODO")
            } else {
                Fiber.abort("TODO")
            }

            return
        }

        // ===== [ 2019 ] ======================================================

        // TODO

        // ===== [ 2017 ] ======================================================

        // TODO

        // ===== [ 2015 ] ======================================================

        // TODO

        // ===== [ 2013 ] ======================================================

        // TODO

        // ===== [ 2012 ] ======================================================

        // TODO

        // ===== [ 2010 ] ======================================================

        // TODO

        // ===== [ 2008 ] ======================================================

        // TODO

        // ===== [ 2005 ] ======================================================

        // TODO

        // ===== [ .NET 2003 ] =================================================

        // TODO

        // ===== [ .NET 2002 ] =================================================

        // TODO

        // ===== [ 6.0 ] =======================================================

        // TODO

        // ===== [ 97 ] ========================================================

        // TODO
    }
}

/*
================================================================================
 * ~~ [ foreign node ] ~~ *
--------------------------------------------------------------------------------
*/

/* Node for executables, static or shared libraries, object files, etc.
 * Anything that requires a compiler, a linker, and/or an assembler.
 */
class NativeNode {
    /*
     * TODO: Node base class to avoid some of this repetitive boilerplate.
     * TODO: Export/import JSON, so we can farm projects out to processes.
     */
    construct new(project, name, mode) {
        if (project == null) {
            _project = Project.new(null, name)
        } else {
            _project = project
        }

        _name = name != null ? name : _project.name.toString
        _mode = mode != null ? mode : "objects"

        _compiler = _project.compiler.toString
        _linker = _project.linker.toString
        _sources = _project.sources.toList
        _includePaths = _project.includePaths.toList
        _debug = _project.debug
        _verbose = _project.verbose
        _defines = _project.defines.toList
        _undefs = _project.undefs.toList
        _compilerPrefix = _project.compilerPrefix
        _linkerPrefix = _project.linkerPrefix
        _extraCompilerFlags = _project.extraCompilerFlags.toList
        _extraLinkerFlags = _project.extraLinkerFlags.toList
        _libraryPaths = _project.libraryPaths.toList
        _runtimeLibraryPaths = _project.runtimeLibraryPaths.toList
        _extraObjects = _project.extraObjects.toList
        _libraries = _project.libraries.toList
        _build32bit = _project.build32bit
        _addressSanitizer = _project.addressSanitizer
        _threadSanitizer = _project.threadSanitizer
        _undefinedBehaviorSanitizer = _project.undefinedBehaviorSanitizer
        _integerSanitizer = _project.integerSanitizer
        _async = _project.async
        _maxAsyncCompileJobs = _project.maxAsyncCompileJobs
        _optimizeForCodeSize = _project.optimizeForCodeSize
        _enableRTTI = _project.enableRTTI
        _enableExceptions = _project.enableExceptions
        _isGUI = _project.isGUI
        _dynamicCRT = _project.dynamicCRT
        _linkTimeOptimization = _project.linkTimeOptimization
        _stripDebugSymbols = _project.stripDebugSymbols
        _finalizeCompilerCommandLine = _project.finalizeCompilerCommandLine
        _finalizeLinkerCommandLine = _project.finalizeLinkerCommandLine
        _warningLevel = _project.warningLevel
        _extraCleanFiles = []
        _haveCompiledCpp = false

        _project.nodes.add(this)
    }

    // Cannot be changed, as we're in the projects node list.
    project { _project }

    mode { _mode }
    mode=(value) { _mode = value }

    isExecutable {
        var mode = StringUtil.toLower(_mode)

        return (mode == "executable" ||
                mode == "exe")
    }

    isSharedLibrary {
        var mode = StringUtil.toLower(_mode)

        return (mode == "shared_library" ||
                mode == "shared_lib" ||
                mode == "sharedlibrary" ||
                mode == "sharedlib" ||
                mode == "shared" ||

                mode == "dynamic_library" ||
                mode == "dynamic_lib" ||
                mode == "dynamiclibrary" ||
                mode == "dynamiclib" ||
                mode == "dynamic" ||

                mode == "dll" ||
                mode == "so")
    }

    isStaticLibrary {
        var mode = StringUtil.toLower(_mode)

        return (mode == "static_library" ||
                mode == "static_lib" ||
                mode == "staticlibrary" ||
                mode == "staticlib" ||
                mode == "static" ||
                mode == "lib" ||
                mode == "a")
    }

    isObjects {
        var mode = StringUtil.toLower(_mode)

        return (mode == "object" ||
                mode == "objects" ||
                mode == "obj" ||
                mode == "o")
    }

    // TODO: isPrecompiledHeader

    executableExtension {
        if (compiler == "emcc") {
            if (isCLI) {
                return ".js"
            } else {
                return ".html"
            }
        } else if (Platform.isWindows) {
            return ".exe"
        } else {
            return ""
        }
    }

    sharedLibraryExtension {
        if (compiler == "emcc") {
            return ".wasm"
        } else if (Platform.isWindows) {
            return ".dll"
        } else {
            return ".so"
        }
    }

    staticLibraryExtension {
        if (compiler == "cl") {
            return ".lib"
        } else {
            return ".a"
        }
    }

    objectExtension {
        if (compiler == "cl") {
            return ".obj"
        } else {
            return ".o"
        }
    }

    fileExtension {
        if (isExecutable) {
            return executableExtension
        }

        if (isSharedLibrary) {
            return sharedLibraryExtension
        }

        if (isStaticLibrary) {
            return staticLibraryExtension
        }

        if (isObjects) {
            return objectExtension
        }

        Fiber.abort("%(this)")
    }

    // TODO: assembly mode - instead of outputting object, files, output asm code

    toString { "%(type)(%(name))" }

    name { _name }
    name=(value) { _name = value }

    compiler { _compiler }
    compiler=(value) { _compiler = value }

    linker { _linker }
    linker=(value) { _linker = value }

    sources { _sources }
    sources=(value) { _sources = value }

    includePaths { _includePaths }
    includePaths=(value) { _includePaths = value }

    debug { _debug }
    debug=(value) { _debug = value }

    release { !debug }
    release=(value) { debug = !value }

    verbose { _verbose }
    verbose=(value) { _verbose = value }

    quiet { !_verbose }
    quiet=(value) { _verbose = !value }

    defines { _defines }
    defines=(value) { _defines = value }

    undefs { _undefs }
    undefs=(value) { _undefs = value }

    define(name, value) {
        defines.add([name, value])
    }

    define(name) {
        defines.add(name)
    }

    undefine(name) {
        undefs.add(name)
    }

    isDefined(key) {
        return Project.isDefined_(key, defines)
    }

    isDefinedTrue(key) {
        return Project.isDefinedTrue_(key, defines)
    }

    isDefinedFalse(key) {
        return Project.isDefinedFalse_(key, defines)
    }

    /* NOTE: For prepending cross-compiler prefixes, i.e. "riscv64-linux-gnu-gcc".
     */
    compilerPrefix { _compilerPrefix }
    compilerPrefix=(value) { _compilerPrefix = value }

    linkerPrefix { _linkerPrefix }
    linkerPrefix=(value) { _linkerPrefix = value }

    extraCompilerFlags { _extraCompilerFlags }
    extraCompilerFlags=(value) { _extraCompilerFlags = value }

    extraLinkerFlags { _extraLinkerFlags }
    extraLinkerFlags=(value) { _extraLinkerFlags = value }

    libraryPaths { _libraryPaths }
    libraryPaths=(value) { _libraryPaths = value }

    runtimeLibraryPaths { _runtimeLibraryPaths }
    runtimeLibraryPaths=(value) { _runtimeLibraryPaths = value }

    addLibraryPath(value) {
        return Project.addLibraryPath_(this, value)
    }

    addLibraryPaths(values) {
        for (value in values) {
            addLibraryPath(value)
        }
    }

    build32bit { _build32bit }
    build32bit=(value) { _build32bit = value }

    build64bit { !build32bit }
    build64bit=(value) { build32bit = !value }

    // TODO: linkerScript (ld -T)
    // TODO: noStandardLibrary (/NODEFAULTLIB, use ld, -nostartfiles, -nodefaultlibs, and/or -nostdlib etc.)

    // TODO: extraCompilerFlagsPerFile ({ filename : flags })
    // TODO: extraLinkerFlagsPerFile ({ filename : flags })

    // TODO: bufferGuard (-fstack-protector on Clang/GCC, /GS and/or /RTCs on MSVC)
    // TODO: precompiledHeaders

    addressSanitizer { _addressSanitizer }
    addressSanitizer=(value) { _addressSanitizer = value }

    threadSanitizer { _threadSanitizer }
    threadSanitizer=(value) { _threadSanitizer = value }

    undefinedBehaviorSanitizer { _undefinedBehaviorSanitizer }
    undefinedBehaviorSanitizer=(value) { _undefinedBehaviorSanitizer = value }

    integerSanitizer { _integerSanitizer }
    integerSanitizer=(value) { _integerSanitizer = value }

    warningLevel { _warningLevel }
    warningLevel=(value) { _warningLevel = value }

    optimizeForCodeSize { _optimizeForCodeSize }
    optimizeForCodeSize=(value) { _optimizeForCodeSize = value }

    optimizeForPerformance { !optimizeForCodeSize }
    optimizeForPerformance=(value) { optimizeForCodeSize = !value }

    extraObjects { _extraObjects }
    extraObjects=(value) { _extraObjects = value }

    extraCleanFiles { _extraCleanFiles }
    extraCleanFiles=(value) { _extraCleanFiles = value }

    libraries { _libraries }
    libraries=(value) { _libraries = value }

    async { _async }
    async=(value) { _async = value }

    blocking { !_async }
    blocking=(value) { _async = !value }

    maxAsyncCompileJobs { _maxAsyncCompileJobs }
    maxAsyncCompileJobs=(value) { _maxAsyncCompileJobs = value }

    /* Enable/disable C++ run-time type information.
     */
    enableRTTI { _enableRTTI }
    enableRTTI=(value) { _enableRTTI = value }

    disableRTTI { !enableRTTI }
    disableRTTI=(value) { enableRTTI = !value }

    enableExceptions { _enableExceptions }
    enableExceptions=(value) { _enableExceptions = value }

    disableExceptions { !enableExceptions }
    disableExceptions=(value) { enableExceptions = !value }

    linkTimeOptimization { _linkTimeOptimization }
    linkTimeOptimization=(value) { _linkTimeOptimization = value }

    stripDebugSymbols { _stripDebugSymbols }
    stripDebugSymbols=(value) { _stripDebugSymbols = value }

    isGUI { _isGUI }
    isGUI=(value) { _isGUI = value }

    isCLI { !isGUI }
    isCLI=(value) { isGUI = !value }

    dynamicCRT { _dynamicCRT }
    dynamicCRT=(value) { _dynamicCRT = value }

    staticCRT { !dynamicCRT }
    staticCRT=(value) { dynamicCRT = !value }

    finalizeCompilerCommandLine { _finalizeCompilerCommandLine }
    finalizeCompilerCommandLine=(value) { _finalizeCompilerCommandLine = value }

    finalizeLinkerCommandLine { _finalizeLinkerCommandLine }
    finalizeLinkerCommandLine=(value) { _finalizeLinkerCommandLine = value }

    haveCompiledCpp { _haveCompiledCpp }
    haveCompiledCpp=(value) { _haveCompiledCpp = value }

    configure(config) {
        Project.configure_(this, config)
    }

    compile() {
        if (verbose && project.printBanners) {
            System.print("-" * 80)
            System.print("\tCompiling %(name) (%(mode)) for " + (debug ? "debugging" : (optimizeForCodeSize ? "size" : "speed")))
            System.print("-" * 80)
        }

        var jobs = []

        var flushJobs = Fn.new {
            for (process in jobs.where { |job| job != null }) {
                System.write(process.readStdout())

                if (process.join() != 0) {
                    Fiber.abort(process.readStderr())
                } else {
                    System.write(process.readStderr())
                }
            }

            jobs.clear()
        }

        /* FIXME: Need to get call to vcvarsall.bat working.
         */
        if (false && compiler == "cl") {
            project.ensureVisualStudioCompilerSetup_()
        }

        for (source in sources) {
            var commandLine = compilerCommandLine_(source)

            if (verbose) {
                System.print(commandLine)
            }

            if (source.endsWith(".cpp") || source.endsWith(".cc") || source.endsWith(".cxx")) {
                haveCompiledCpp = true
            }

            if (async) {
                if (false) {
                    /*
                     * NOTE: Rather than waiting for one file that takes much longer than
                     * others, we scan this list in a round-robin fashion (calling sleep
                     * between checks) to replace completed build jobs with new processes.
                     */
                    jobs.add(Process.create(commandLine))

                    if (jobs.count == maxAsyncCompileJobs) {
                        flushJobs.call()
                    }
                } else {
                    if (jobs.count == 0) {
                        jobs = [null] * maxAsyncCompileJobs
                    }

                    while (jobs.all { |job| job != null && job.alive }) {
                        /*
                         * Don't burn a core - just check every tenth of a second or so.
                         */
                        Timer.sleepMS(100)
                    }

                    for (i in 0...jobs.count) {
                        if (jobs[i] == null || !jobs[i].alive) {
                            if (jobs[i] != null) {
                                System.write(jobs[i].readStdout())

                                if (jobs[i].join() != 0) {
                                    Fiber.abort(jobs[i].readStderr())
                                } else {
                                    System.write(jobs[i].readStderr())
                                }
                            }

                            jobs[i] = Process.create(commandLine)
                            break
                        }
                    }
                }
            } else {
                /*
                 * FIXME: Large stdout/stderr buffers hang the calling process, at least
                 * on Linux (must test other systems). run also disables terminal colors.
                 */
                if (Process.system(commandLine) != 0) {
                    Fiber.abort("%(project).%(this) failed to compile \"%(source)\"!")
                }
            }
        }

        flushJobs.call()
    }

    link() {
        if (isObjects) {
            return
        }

        if (verbose && project.printBanners) {
            System.print("-" * 80)
            System.print("\tLinking %(name) (%(mode)) for " + (debug ? "debugging" : (optimizeForCodeSize ? "size" : "speed")))
            System.print("-" * 80)
        }

        var commandLine = linkerCommandLine_

        if (verbose) {
            System.print(commandLine)
        }

        /* FIXME: Large stdout/stderr buffers hang the calling process, at least
         * on Linux (must test other systems). run also disables terminal colors.
         */
        if (Process.system(commandLine) != 0) {
            Fiber.abort("%(project).%(this) failed to link!")
        }

        if (release && stripDebugSymbols) {
            stripDebugSymbols_()
        }
    }

    build() {
        compile()
        link()
    }

    clean() {
        for (source in sources) {
            Path.tryRemove(sourceToObject_(source))

            if (compiler != "cl") {
                Path.tryRemove(sourceToMakefileDependency_(source))
                Path.tryRemove(sourceToDWARF_(source))
            }
        }

        if (isExecutable) {
            if (Platform.isWindows) {
                Path.tryRemove(name + ".exe")
            } else {
                Path.tryRemove(name)
            }
        } else if (isSharedLibrary) {
            if (Platform.isWindows) {
                Path.tryRemove(name + ".dll")
                Path.tryRemove(name + ".lib")
            } else {
                Path.tryRemove("lib" + name + ".so")
                Path.tryRemove(name + ".so")
            }
        } else if (isStaticLibrary) {
            if (Platform.isWindows) {
                Path.tryRemove(name + ".lib")
            } else {
                Path.tryRemove("lib" + name + ".a")
                Path.tryRemove(name + ".a")
            }
        }

        if (Platform.isWindows) {
            Path.tryRemove(name + ".exp")
        }

        for (filename in extraCleanFiles) {
            Path.tryRemove(filename)
        }
    }

    finish(command) {
        //
    }

    // ===== [ platform-specific implementation details (compiler) ] ===========

    compilerName_(filename) {
        var compilerName = compiler

        if (compilerName == "cl") {
            /*
             * Call into the Visual Studio macro assembler instead.
             */
            if (filename.endsWith(".S")) {
                compilerName = "ml"
            }
        } else if (compilerName == "cc") {
            /*
             * Handle C++ files for a generic Unix compiler.
             */
            if (filename.endsWith(".cpp") || filename.endsWith(".cc") || filename.endsWith(".cxx")) {
                compilerName = "c++"
            }
        } else if (compilerName == "gcc") {
            /*
             * Handle C++ files for the GNU compiler collection.
             */
            if (filename.endsWith(".cpp") || filename.endsWith(".cc") || filename.endsWith(".cxx")) {
                compilerName = "g++"
            }
        } else if (compilerName == "clang") {
            /*
             * Handle C++ files for the LLVM C and C++ frontend.
             */
            if (filename.endsWith(".cpp") || filename.endsWith(".cc") || filename.endsWith(".cxx")) {
                compilerName = "clang++"
            }
        } else if (compilerName == "emcc") {
            /*
             * Handle C++ files for the Emscripten compiler.
             */
            if (filename.endsWith(".cpp") || filename.endsWith(".cc") || filename.endsWith(".cxx")) {
                compilerName = "em++"
            }
        }

        return "%(compilerPrefix)%(compilerName) "
    }

    compilerIncludePaths_ {
        /*
         * TODO: Ensure paths are properly escaped.
         */
        var s = []

        if (compiler == "cl") {
            for (path in includePaths) {
                if (path.contains(" ")) {
                    s.add("\"/I%(path)\" ")
                } else {
                    s.add("/I%(path) ")
                }
            }
        } else {
            for (path in includePaths) {
                s.add("-I%(path) ")
            }
        }

        return s.join()
    }

    compilerOptimization_ {
        var s = []

        if (compiler == "cl") {
            if (debug) {
                s.add("/Od")
                s.add("/Zi")
                s.add("/DEBUG")
                s.add("/DDEBUG=1")
            } else {
                if (optimizeForCodeSize) {
                    if (true) {
                        s.add("/O1")
                    } else if (true) {
                        s.add("/Oi")
                    } else {
                        // XXX: Seems to do nothing in my testing.
                        s.add("/Os")
                    }
                } else {
                    if (true) {
                        s.add("/Ox")
                    } else if (true) {
                        s.add("/O2")
                    } else {
                        // XXX: Seems to do nothing in my testing.
                        s.add("/Ot")
                    }
                }

                s.add("/DNDEBUG=1")
                s.add("/INCREMENTAL:NO")

                // Package global data in COMDAT sections for optimization.
                s.add("/Gw")

                // Remove unreferenced functions or data if they're COMDAT or have internal linkage only.
                s.add("/Zc:inline")
            }
        } else {
            if (debug) {
                s.add("-O0")
                s.add("-g")
                s.add("-DDEBUG=1")
            } else {
                if (optimizeForCodeSize) {
                    if (true) {
                        s.add("-Os")
                    } else if (true) {
                        s.add("-Oi")
                    } else {
                        s.add("-Oz")
                    }
                } else {
                    if (false) {
                        // Former `-Ofast` flag.
                        s.add("-O3 -ffast-math")
                    } else if (true) {
                        s.add("-O3")
                    } else if (true) {
                        s.add("-O2")
                    } else {
                        s.add("-O1")
                    }
                }

                /* XXX: Some versions of `ar` don't like link-time optimization.
                 */
                if (linkTimeOptimization && !isStaticLibrary) {
                    s.add("-flto")
                }

                s.add("-DNDEBUG=1")
            }
        }

        if (s.isEmpty) {
            return ""
        } else {
            return s.join(" ") + " "
        }
    }

    compilerDefines_ {
        var s = []

        for (define in defines) {
            var d

            if (define is List) {
                d = define[0] + "=" + define[1].toString
            } else if (define.contains("=")) {
                d = define
            } else {
                d = define + "=1"
            }

            if (compiler == "cl") {
                s.add("/D%(d)")
            } else {
                s.add("-D%(d)")
            }
        }

        if (s.isEmpty) {
            return ""
        } else {
            return s.join(" ") + " "
        }
    }

    compilerUndefines_ {
        var s = []

        for (undef in undefs) {
            Fiber.abort("TODO")
        }

        if (s.isEmpty) {
            return ""
        } else {
            return s.join(" ") + " "
        }
    }

    compilerWarningFlags_ {
        var s = []

        if (compiler == "cl") {
            if (warningLevel < 0) {
                s.add("/w")
            } else if (warningLevel >= 4) {
                s.add("/Wall")
            } else if (warningLevel >= 3) {
                s.add("/W4")
            } else if (warningLevel >= 2) {
                s.add("/W3")
            } else if (warningLevel >= 1) {
                s.add("/W2")
            } else {
                s.add("/W1")
            }

            if (true && warningLevel >= 3) {
                s.add("/wd4100") // unreferenced parameter
                s.add("/wd4127") // conditional expression is constant
                s.add("/wd4200") // nonstandard extension used
            }
        } else {
            if (warningLevel < 0) {
                s.add("-w")
            } else {
                /*
                 * TODO: silenceUnimportantErrors (or something to that effect).
                 */
                if (true) {
                    s.add("-Wno-constant-logical-operand")
                    s.add("-Wno-format-truncation")
                    s.add("-Wno-format-zero-length")
                    s.add("-Wno-gnu-inline-cpp-without-extern")
                    s.add("-Wno-ignored-attributes")
                    s.add("-Wno-inconsistent-missing-override")
                    s.add("-Wno-tautological-pointer-compare")
                }

                if (warningLevel >= 1) {
                    s.add("-Wall")

                    if (warningLevel >= 2) {
                        s.add("-Wextra")

                        if (true) {
                            s.add("-Wno-unused-function")
                            s.add("-Wno-unused-parameter")
                        }

                        if (warningLevel >= 3) {
                            s.add("-Wpedantic")

                            if (true) {
                                s.add("-Wno-gnu-label-as-value")
                                s.add("-Wno-format-pedantic")
                                s.add("-Wno-overlength-strings")
                                s.add("-Wno-strict-prototypes")
                            }
                        }
                    }
                }
            }

            /* TODO: terminateOnFirstError (can't do this on MSVC).
             */
            if (true) {
                s.add("-Wfatal-errors")
            }
        }

        // TODO: warningsAsErrors

        if (s.isEmpty) {
            return ""
        } else {
            return s.join(" ") + " "
        }
    }

    compilerPlatformFlags_ {
        /*
         * Only build object files, don't link.
         */
        var s = []

        if (compiler == "cl") {
            s.add("/nologo")
            s.add("/c")

            // For very large builds.
            s.add("/bigobj")

            // Sync PDB writes.
            if (async) {
                s.add("/FS")
            }

            if (enableRTTI) {
                s.add("/GR")
            } else {
                s.add("/GR-")
            }

            if (enableExceptions) {
                /*
                 * TODO: Profile difference.
                 */
                if (true) {
                    s.add("/EHsc")
                } else {
                    s.add("/EHa")
                }
            } else {
                // Implicitly disabled.
            }

            if (dynamicCRT) {
                if (debug) {
                    s.add("/MDd")
                } else {
                    s.add("/MD")
                }
            } else {
                if (debug) {
                    s.add("/MTd")
                } else {
                    s.add("/MT")
                }
            }
        } else {
            if (isExecutable) {
                s.add("-fPIE")
            } else {
                s.add("-fPIC")
            }

            s.add("-c")

            if (compiler == "emcc") {
                //s.add("-s WASM=1")
            } else {
                // For very large builds.
                if (release) {
                    if (!Platform.isRiscV) {
                        s.add("-gsplit-dwarf")
                    }
                } else {
                    s.add("-gsplit-dwarf")

                    if (Platform.isRiscV) {
                        s.add("-mno-relax")
                    }
                }
            }

            if (enableRTTI) {
                s.add("-frtti")
            } else {
                s.add("-fno-rtti")
            }

            if (enableExceptions) {
                s.add("-fexceptions")
            } else {
                s.add("-fno-exceptions")
            }

            // TODO: strictAliasing option/flag - for now we go with the safe route.
            s.add("-fno-strict-aliasing")

            /*if (staticCRT) {
                Fiber.abort("TODO")
            }*/

            if (build32bit) {
                s.add("-m32")
            }
        }

        /* TODO: if (!noStandardLibrary)
         */
        if (!Platform.isWindows) {
            s.add("-pthread")
        }

        if (s.isEmpty) {
            return ""
        } else {
            return s.join(" ") + " "
        }
    }

    compilerSanitizerFlags_ {
        var s = []

        if (compiler == "cl") {
            if (addressSanitizer) {
                s.add("/fsanitize=address")
            }

            if (threadSanitizer) {
                Fiber.abort("TODO")
            }

            if (undefinedBehaviorSanitizer) {
                Fiber.abort("TODO")
            }

            if (integerSanitizer) {
                Fiber.abort("TODO")
            }
        } else {
            if (addressSanitizer) {
                s.add("-fsanitize=address")
            }

            if (threadSanitizer) {
                s.add("-fsanitize=thread")
            }

            if (undefinedBehaviorSanitizer) {
                s.add("-fsanitize=undefined")
            }

            if (integerSanitizer) {
                s.add("-fsanitize=integer")
            }
        }

        if (s.isEmpty) {
            return ""
        } else {
            return s.join(" ") + " "
        }
    }

    compilerExtraFlags_ {
        var s = extraCompilerFlags.join(" ")

        if (s == "") {
            return s
        } else {
            return s + " "
        }
    }

    compilerLibraries_ {
        return ""
    }

    compilerLibraryPaths_ {
        return ""
    }

    compilerCommandLine_(filename) {
        var data = (compilerName_(filename) +
                    compilerIncludePaths_ +
                    compilerOptimization_ +
                    compilerDefines_ +
                    compilerUndefines_ +
                    compilerWarningFlags_ +
                    compilerPlatformFlags_ +
                    compilerSanitizerFlags_ +
                    compilerExtraFlags_ +
                    filename +
                    compilerLibraries_ +
                    compilerLibraryPaths_)

        if (finalizeCompilerCommandLine != null) {
            data = finalizeCompilerCommandLine.call(filename, data)

            if (!(data is String)) {
                Fiber.abort("%(this).finalizeCompilerCommandLine must return a string!")
            }
        }

        return data
    }

    // ===== [ platform-specific implementation details (linker) ] =============

    linkerName_ {
        var prefix = linkerPrefix
        var linkerName = linker

        if (isStaticLibrary) {
            prefix = ""

            if (linkerName == "link") {
                linkerName = "lib"
            } else if (linkerName == "tcc") {
                linkerName = "tcc -ar"
            } else if (linkerName == "emcc") {
                linkerName = "emar rcs"
            } else {
                // TODO: Profile the difference here.
                linkerName = /*"gcc-" +*/ "ar rcs"
            }
        } else if (linker != "link") {
            var cppHasBeenCompiled = sources.any { |filename| filename.endsWith(".cpp") }

            if (linkerName == "cc" && cppHasBeenCompiled) {
                linkerName = "c++"
            } else if (linkerName == "gcc" && cppHasBeenCompiled) {
                linkerName = "g++"
            } else if (linkerName == "clang" && cppHasBeenCompiled) {
                linkerName = "clang++"
            } else if (linkerName == "emcc" && cppHasBeenCompiled) {
                linkerName = "em++"
            }
        }

        return "%(prefix)%(linkerName) "
    }

    linkerPlatformFlags_ {
        var s = []

        if (linker == "link") {
            s.add("/nologo")

            if (isExecutable) {
                s.add("/SUBSYSTEM:" + (isGUI ? "WINDOWS" : "CONSOLE"))
            }

            if (release && linkTimeOptimization && sources.count > 1) {
                s.add("/LTCG")
            }

            if (release) {
                // Eliminate functions and data that are never referenced.
                s.add("/OPT:REF")

                // Perform COMDAT folding, then do it again.
                s.add("/OPT:ICF=2")
            }
        } else if (linker == "emcc") {
            //s.add("-s WASM=1")

            if (isExecutable) {
                s.add("-s ALLOW_MEMORY_GROWTH=1")
                s.add("-s MAIN_MODULE=1")
            } else if (isSharedLibrary) {
                s.add("-s SIDE_MODULE=1")
            }

            // Enable filesystem when running in node.js instead of a browser.
            //s.add("-s NODERAWFS=1")
        } else {
            /*
             * XXX: Some versions of `ar` don't like link-time optimization.
             */
            if (release && linkTimeOptimization && !isStaticLibrary) {
                s.add("-flto")
            }

            /* XXX: This might require disabling link-time optimization.
             */
            if (build32bit && !isStaticLibrary) {
                s.add("-m32")
            }
        }

        /* TODO: if (!noStandardLibrary)
         */
        if (!Platform.isWindows && !isStaticLibrary && linker != "emcc") {
            s.add("-pthread")
        }

        if (s.isEmpty) {
            return ""
        } else {
            return s.join(" ") + " "
        }
    }

    linkerSanitizerFlags_ {
        if (isStaticLibrary) {
            return ""
        }

        return compilerSanitizerFlags_
    }

    linkerExtraFlags_ {
        var s = extraLinkerFlags.join(" ")

        if (s == "") {
            return s
        } else {
            return s + " "
        }
    }

    linkerOutput_ {
        var s = []

        if (isExecutable) {
            if (linker == "link") {
                s.add("/OUT")
            } else {
                s.add("-o")
            }

            s.add(name + executableExtension)
        } else if (isSharedLibrary) {
            if (linker == "link") {
                s.add("/DLL /OUT")
            } else if (linker == "emcc") {
                s.add("-o")
            } else {
                s.add("-fPIC -shared -o")
            }

            if (Platform.isWindows) {
                s.add(name + ".dll")
            } else if (false && linker == "emcc") {
                /*
                 * Emscripten understands .so files.
                 */
                s.add(name + ".wasm")
            } else {
                /*
                 * NOTE: The "lib" prefix is not required, but is UNIX convention.
                 */
                if (true) {
                    s.add("lib" + name + ".so")
                } else {
                    s.add(name + ".so")
                }
            }
        } else if (isStaticLibrary) {
            if (linker == "link") {
                s.add("/OUT")
            } else {
                /*
                 * NOTE: The "lib" prefix is not required, but is UNIX convention.
                 */
                if (true) {
                    return "lib" + name + ".a "
                } else {
                    return name + ".a "
                }
            }

            if (Platform.isWindows) {
                s.add(name + ".lib")
            }
        } else {
            Fiber.abort(_mode.toString)
        }

        if (linker == "link") {
            return s.join(":") + " "
        } else {
            return s.join(" ") + " "
        }
    }

    sourceToOtherFileType_(source, extension) {
        /*
         * XXX TODO FIXME: This is an ugly and gross hack. Strip extension using the `file.Path` module.
         */
        var s = ( source.replace(".cpp", extension)
                        .replace(".cc", extension)
                        .replace(".cxx", extension)
                        .replace(".c", extension)
                        .replace(".S", extension) )

        s = s.split("\\")[-1]
        s = s.split("/")[-1]

        return s
    }

    sourceToObject_(source) {
        return sourceToOtherFileType_(source, objectExtension)
    }

    sourceToDWARF_(source) {
        return sourceToOtherFileType_(source, ".dwo")
    }

    sourceToMakefileDependency_(source) {
        return sourceToOtherFileType_(source, ".d")
    }

    linkerObjects_ {
        var s = []

        for (source in sources) {
            s.add(sourceToObject_(source))
        }

        for (object in extraObjects) {
            /*
             * NOTE: If name has no extension, append platform object extension.
             */
            if (object.endsWith(objectExtension)) {
                s.add(object)
            } else {
                s.add(sourceToObject_(object))
            }
        }

        return s.join(" ")
    }

    linkerLibraries_ {
        var s = []

        if (linker == "link") {
            for (library in libraries) {
                s.add("%(library).lib")
            }
        } else {
            if (isStaticLibrary) {
                return ""
            }

            for (library in libraries) {
                if (Path.isFile("lib" + library + ".a")) {
                    if (true) {
                        s.add("lib" + library + ".a")
                    } else {
                        s.add("-l:lib%(library).a")
                    }
                } else if (Path.isFile(library + ".a")) {
                    if (true) {
                        s.add(library + ".a")
                    } else {
                        s.add("-l:%(library).a")
                    }
                } else if (Path.isFile(library + ".lib")) {
                    if (true) {
                        s.add(library + ".lib")
                    } else {
                        s.add("-l:%(library).lib")
                    }
                } else {
                    s.add("-l%(library)")
                }
            }
        }

        /* TODO: if (!noStandardLibrary)
         */
        if (!Platform.isWindows && !isStaticLibrary && linker != "emcc") {
            /*
             * NOTE: Not necessary when we're linking with g++ etc.
             */
            /*if (haveCompiledCpp && !libraries.contains("stdc++")) {
                s.add("-lstdc++")
            }*/

            if (!libraries.contains("dl")) {
                s.add("-ldl")
            }

            if (!libraries.contains("m")) {
                s.add("-lm")
            }
        }

        if (s.isEmpty) {
            return ""
        } else {
            return " " + s.join(" ")
        }
    }

    linkerLibraryPaths_ {
        if (isStaticLibrary) {
            return ""
        }

        var s = []

        if (linker == "link") {
            for (path in libraryPaths) {
                /*
                 * This works fine, but it's redundant.
                 */
                if (path == ".") {
                    continue
                }

                s.add("/LIBPATH:" + path)
            }

            for (path in runtimeLibraryPaths) {
                if (path == ".") {
                    continue
                }

                Fiber.abort("TODO")
            }
        } else if (linker == "emcc") {
            for (path in libraryPaths) {
                s.add("-L" + path)
            }
        } else {
            for (path in libraryPaths) {
                s.add("-L" + path)
            }

            for (path in runtimeLibraryPaths) {
                if (path.startsWith("/")) {
                    /*
                     * Absolute path.
                     */
                    s.add("-rpath " + path)
                } else if (path == ".") {
                    /*
                     * Base path (might not be necessary).
                     */
                    s.add("-rpath '$ORIGIN'")
                } else {
                    /*
                     * Relative path.
                     */
                    s.add("-rpath '$ORIGIN/%(path)'")
                }
            }
        }

        if (s.isEmpty) {
            return ""
        } else {
            return " " + s.join(" ")
        }
    }

    linkerCommandLine_ {
        var data = (linkerName_ +
                    linkerPlatformFlags_ +
                    linkerSanitizerFlags_ +
                    linkerExtraFlags_ +
                    linkerOutput_ +
                    linkerObjects_ +
                    linkerLibraries_ +
                    linkerLibraryPaths_)

        if (finalizeLinkerCommandLine != null) {
            data = finalizeLinkerCommandLine.call(data)

            if (!(data is String)) {
                Fiber.abort("%(this).finalizeLinkerCommandLine must return a string!")
            }
        }

        return data
    }

    stripDebugSymbols_() {
        var command = null

        if (Platform.isWindows) {
            /*
             * NOTE: Windows executables don't contain symbols (they use PDBs).
             */
        } else if (Platform.isMacOSX) {
            if (false && isSharedLibrary) {
                /*
                 * "symbols referenced by indirect symbol table entries that can't be stripped".
                 */
                command = "strip %(name).so"
            } else if (isExecutable) {
                command = "strip %(name)"
            }
        } else {
            if (isSharedLibrary) {
                if (true) {
                    command = "strip -s lib%(name).so"
                } else {
                    command = "strip -s %(name).so"
                }
            } else if (isExecutable) {
                command = "strip -s %(name)"
            }
        }

        if (command != null) {
            if (verbose) {
                System.print(command)
            }

            /* FIXME: Large stdout/stderr buffers hang the calling process, at least
             * on Linux (must test other systems). run also disables terminal colors.
             */
            Process.system(command)
        }
    }
}

/*
================================================================================
 * ~~ [ project generation ] ~~ *
--------------------------------------------------------------------------------
*/

/* TODO: Audit this to ensure all flags match what we'd get building a normal project.
 * Until then, consider this experimental... don't use makefile for shipping releases.
 * Also note that you're baking in the project config (debug vs. release, files, etc).
 * TODO: Hardcodes a bunch of things - should use standard variables like CC, LD, etc.
 */
class MakefileNode {
    construct new(project, name) {
        if (project == null) {
            _project = Project.new(null, name)
        } else {
            _project = project
        }

        _name = name != null ? name : _project.name.toString
        _verbose = _project.verbose

        _project.nodes.add(this)
    }

    toString { "%(type)(%(name))" }

    // Cannot be changed, as we're in the projects node list.
    project { _project }

    name { _name }
    name=(value) { _name = value }

    verbose { _verbose }
    verbose=(value) { _verbose = value }

    quiet { !_verbose }
    quiet=(value) { _verbose = !value }

    build() {
        if (verbose && project.printBanners) {
            System.print("-" * 80)
            System.print("\tGenerating makefile for %(project.name)")
            System.print("-" * 80)
        }

        var native_nodes = project.nodes.where { |node| node is NativeNode }.toList
        var file

        if (true) {
            file = File.open("Makefile", "w")
        } else {
            file = File.stdout
        }

        file.write([
            "# ==============================================================================",
            "# Automatically generated makefile for %(project.name)",
            "# ==============================================================================",
            "",
            ".PHONY: all clean",
            "\n",
        ].join("\n"))

        var all_targets = []
        var all_clean_files = []
        var all_dependency_files = []

        for (node in native_nodes) {
            all_targets.add(MakefileNode.targetName_(node))
        }

        file.write("all: %(all_targets.join(" "))\n\n")

        for (node in native_nodes) {
            MakefileNode.generateNodeRules_(file, node, all_clean_files, all_dependency_files)
        }

        file.write([
            "# ==============================================================================",
            "# Cleanup",
            "# ==============================================================================",
            "clean:",
            "",
        ].join("\n"))

        // Makefiles can choke on massively long lines, so we split the clean payload.
        var chunk_size = 8

        for (i in 0...(all_clean_files.count / chunk_size).ceil) {
            var chunk = all_clean_files[(i * chunk_size)...((i + 1) * chunk_size).min(all_clean_files.count)]
            file.write("\trm -f %(chunk.join(" "))\n")
        }

        if (project.extraCleanFiles.count > 0) {
            file.write("\trm -f %(project.extraCleanFiles.join(" "))\n")
        }

        file.write("\n")

        if (all_dependency_files.count > 0) {
            file.write("-include %(all_dependency_files.join(" "))\n")
        }

        if (file != File.stdout) {
            file.close()
        }
    }

    clean() {
        Path.tryRemove("Makefile")
    }

    finish(command) {
        //
    }

    static targetName_(node) {
        /*
         * We assume here that we're building Unix/ELF binaries.
         */
        if (node.isExecutable) {
            return node.name + node.executableExtension
        } else if (node.isSharedLibrary) {
            return "lib%(node.name)%(node.sharedLibraryExtension)"
        } else if (node.isStaticLibrary) {
            return "lib%(node.name)%(node.staticLibraryExtension)"
        } else {
            return node.name + node.objectExtension
        }
    }

    static getCompiler_(node, src) {
        return node.compilerName_(src).trim()
    }

    static getLinker_(node) {
        return node.linkerName_.trim()
    }

    static compilerFlags_(node) {
        /*
         * TODO: Remove this and just append to node.compilerCommandLine_
         */
        return (node.compilerIncludePaths_ +
                node.compilerOptimization_ +
                node.compilerDefines_ +
                node.compilerUndefines_ +
                node.compilerWarningFlags_ +
                node.compilerPlatformFlags_ +
                node.compilerSanitizerFlags_ +
                node.compilerExtraFlags_).trim() +

                // Auto-dependency generation flags.
                " -MMD -MP"
    }

    static linkerFlags_(node) {
        /*
         * TODO: Remove this and just append to node.linkerCommandLine_
         */
        return (node.linkerPlatformFlags_ +
                node.linkerSanitizerFlags_ +
                node.linkerExtraFlags_ +
                node.linkerLibraries_ +
                node.linkerLibraryPaths_).trim().replace("-rpath '$ORIGIN", "-rpath '$$ORIGIN")
    }

    static generateNodeRules_(file, node, clean_files, dependency_files) {
        var target = MakefileNode.targetName_(node)
        var cc_flags = MakefileNode.compilerFlags_(node)
        var ld_flags = MakefileNode.linkerFlags_(node)

        var objs = []
        file.write("#\n# ----- [ TARGET: %(node.name) ]\n#\n")

        // Write explicit compilation rules for each source file.
        for (src in node.sources) {
            var obj = node.sourceToObject_(src)
            var dep = node.sourceToMakefileDependency_(src)

            var compiler = MakefileNode.getCompiler_(node, src)

            objs.add(obj)
            clean_files.add(obj)
            dependency_files.add(dep)

            file.write("%(obj): %(src)\n")

            // Write out the final command line to the makefile.
            var cmd = "%(compiler) %(cc_flags) -c $< -o $@"

            if (node.finalizeCompilerCommandLine != null) {
                cmd = node.finalizeCompilerCommandLine.call(src, cmd)

                if (!(cmd is String)) {
                    Fiber.abort("%(node).finalizeCompilerCommandLine must return a string!")
                }
            }

            file.write("\t%(cmd)\n\n")
        }

        /* Append any extra objects (e.g. precompiled binaries or assembly outputs).
         * If the filename has no extension, append the platform;s object extension.
         */
        for (extra in node.extraObjects) {
            if (extra.endsWith(node.objectExtension)) {
                objs.add(extra)
            } else {
                objs.add(node.sourceToObject_(extra))
            }
        }

        for (extra in node.extraCleanFiles) {
            clean_files.add(extra)
        }

        clean_files.add(target)
        var obj_str = objs.join(" ")

        // Write the linking rule to the makefile.
        file.write("%(target): %(obj_str)\n")

        var linker = MakefileNode.getLinker_(node)

        if (node.isStaticLibrary) {
            var cmd = "%(linker) $@ $^"

            if (node.finalizeLinkerCommandLine != null) {
                cmd = node.finalizeLinkerCommandLine.call(cmd)

                if (!(cmd is String)) {
                    Fiber.abort("%(node).finalizeLinkerCommandLine must return a string!")
                }
            }

            file.write("\t%(cmd)\n\n")
        } else {
            var out_flag

            if (node.isSharedLibrary && node.linker != "emcc") {
                out_flag = "-fPIC -shared -o $@"
            } else {
                out_flag = "-o $@"
            }

            var cmd = "%(linker) %(out_flag) $^ %(ld_flags)"

            if (node.finalizeLinkerCommandLine != null) {
                cmd = node.finalizeLinkerCommandLine.call(cmd)

                if (!(cmd is String)) {
                    Fiber.abort("%(node).finalizeLinkerCommandLine must return a string!")
                }
            }

            file.write("\t%(cmd)\n")

            if (node.release && node.stripDebugSymbols) {
                if (Platform.isMacOSX) {
                    file.write("\tstrip $@\n")
                } else if (!Platform.isWindows) {
                    file.write("\tstrip -s $@\n")
                }
            }

            file.write("\n")
        }
    }
}

/* TODO: Audit this to ensure all flags match what we'd get building a normal project.
 * Until then, consider this experimental... don't use makefile for shipping releases.
 * Also note that you're baking in the project config (debug vs. release, files, etc).
 *
 * TODO: Rebuild on top of a class that manipulates Visual Studio projects/solutions,
 * which in turn would be built on top of a class capable of reading and writing XML.
 */
class VisualStudioNode {
    construct new(project, name) {
        if (project == null) {
            _project = Project.new(null, name)
        } else {
            _project = project
        }

        _name = name != null ? name : _project.name.toString
        _verbose = _project.verbose

        /* TODO: Defaulting to Visual Studio 2022 (v143).
         * Find the latest (or oldest) version installed.
         */
        _vsVersion = "2022"
        _platformToolset = "v143"

        _project.nodes.add(this)
    }

    toString { "%(type)(%(name))" }

    project { _project }

    name { _name }
    name=(value) { _name = value }

    verbose { _verbose }
    verbose=(value) { _verbose = value }

    quiet { !_verbose }
    quiet=(value) { _verbose = !value }

    vsVersion { _vsVersion }
    vsVersion=(value) { _vsVersion = value }

    platformToolset { _platformToolset }
    platformToolset=(value) { _platformToolset = value }

    build() {
        if (verbose && project.printBanners) {
            System.print("-" * 80)
            System.print("\tGenerating Visual Studio %(vsVersion) solution for %(project.name)")
            System.print("-" * 80)
        }

        var native_nodes = project.nodes.where { |node| node is NativeNode }.toList

        if (native_nodes.isEmpty) {
            if (verbose) {
                System.print("No native targets found. Skipping solution generation.")
            }

            return
        }

        generateSolution_(native_nodes)

        for (node in native_nodes) {
            generateProject_(node)
            generateFilters_(node)
        }
    }

    clean() {
        Path.tryRemove(name + ".sln")

        for (node in project.nodes.where { |node| node is NativeNode }.toList) {
            Path.tryRemove(node.name + ".vcxproj")
            Path.tryRemove(node.name + ".vcxproj.filters")
            Path.tryRemove(node.name + ".vcxproj.user")
        }
    }

    finish(command) {
        //
    }

    // ===== [ private generators ] ============================================

    generateSolution_(nodes) {
        var file

        if (true) {
            file = File.open(name + ".sln", "w")
        } else {
            file = File.stdout
        }

        /* XXX TODO FIXME: Tie this into vsVersion and platformToolset.
         */
        file.write("\r\nMicrosoft Visual Studio Solution File, Format Version 12.00\r\n")
        file.write("# Visual Studio Version 17\r\n")

        for (node in nodes) {
            var projectGuid = StringUtil.generateVisualStudioGUID(node.name)

            /* The standard C++ project type GUID in Visual Studio.
             */
            file.write("Project(\"{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}\") = \"%(node.name)\", \"%(node.name).vcxproj\", \"{%(projectGuid)}\"\r\n")
            file.write("EndProject\r\n")
        }

        file.write("Global\r\n")
        file.write("\tGlobalSection(SolutionConfigurationPlatforms) = preSolution\r\n")

        var configs =
        [
            "Debug|x64",
            "Release|x64",
            "Debug|x86",
            "Release|x86",
        ]

        for (config in configs) {
            file.write("\t\t%(config) = %(config)\r\n")
        }

        file.write("\tEndGlobalSection\r\n")
        file.write("\tGlobalSection(ProjectConfigurationPlatforms) = postSolution\r\n")

        for (node in nodes) {
            var project_guid = StringUtil.generateVisualStudioGUID(node.name)

            for (config in configs) {
                /*
                 * MSBuild requires mapping the Solution's 'x86' string to the Project's 'Win32' string.
                 */
                var project_config = config.replace("x86", "Win32")

                file.write("\t\t{%(project_guid)}.%(config).ActiveCfg = %(project_config)\r\n")
                file.write("\t\t{%(project_guid)}.%(config).Build.0 = %(project_config)\r\n")
            }
        }

        file.write("\tEndGlobalSection\r\n")

        file.write("\tGlobalSection(SolutionProperties) = preSolution\r\n")
        file.write("\t\tHideSolutionNode = FALSE\r\n")
        file.write("\tEndGlobalSection\r\n")

        file.write("EndGlobal\r\n")

        if (file != File.stdout) {
            file.close()
        }
    }

    generateProject_(node) {
        var file

        if (true) {
            file = File.open(node.name + ".vcxproj", "w")
        } else {
            file = File.stdout
        }

        var project_guid = StringUtil.generateVisualStudioGUID(node.name)
        var config_type = getConfigurationType_(node)

        file.write("<?xml version=\"1.0\" encoding=\"utf-8\"?>\r\n")
        file.write("<Project DefaultTargets=\"Build\" xmlns=\"http://schemas.microsoft.com/developer/msbuild/2003\">\r\n")

        // Project Configurations.
        file.write("  <ItemGroup Label=\"ProjectConfigurations\">\r\n")

        var configs =
        [
            "Debug|x64",
            "Release|x64",
            "Debug|Win32",
            "Release|Win32",
        ]

        for (config in configs) {
            var split = config.split("|")

            file.write("    <ProjectConfiguration Include=\"%(config)\">\r\n")
            file.write("      <Configuration>%(split[0])</Configuration>\r\n")
            file.write("      <Platform>%(split[1])</Platform>\r\n")
            file.write("    </ProjectConfiguration>\r\n")
        }

        file.write("  </ItemGroup>\r\n")

        // Globals.
        file.write("  <PropertyGroup Label=\"Globals\">\r\n")
        file.write("    <ProjectGuid>{%(project_guid)}</ProjectGuid>\r\n")
        file.write("    <Keyword>Win32Proj</Keyword>\r\n")
        file.write("    <RootNamespace>%(node.name)</RootNamespace>\r\n")
        file.write("    <WindowsTargetPlatformVersion>10.0</WindowsTargetPlatformVersion>\r\n")
        file.write("  </PropertyGroup>\r\n")

        file.write("  <Import Project=\"$(VCTargetsPath)\\Microsoft.Cpp.Default.props\" />\r\n")

        // Configuration specific properties.
        for (config in configs) {
            var is_debug = config.startsWith("Debug") ? "true" : "false"
            var is_rel = config.startsWith("Release") ? "true" : "false"
            var opt_lto = (is_rel == "true" && node.linkTimeOptimization && !node.isStaticLibrary) ? "true" : "false"

            file.write("  <PropertyGroup Condition=\"'$(Configuration)|$(Platform)'=='%(config)'\" Label=\"Configuration\">\r\n")
            file.write("    <ConfigurationType>%(config_type)</ConfigurationType>\r\n")
            file.write("    <UseDebugLibraries>%(is_debug)</UseDebugLibraries>\r\n")
            file.write("    <PlatformToolset>%(platformToolset)</PlatformToolset>\r\n")
            file.write("    <WholeProgramOptimization>%(opt_lto)</WholeProgramOptimization>\r\n")
            file.write("    <CharacterSet>Unicode</CharacterSet>\r\n")
            file.write("  </PropertyGroup>\r\n")
        }

        file.write("  <Import Project=\"$(VCTargetsPath)\\Microsoft.Cpp.props\" />\r\n")

        // Combine /bigobj with any user-defined extra compiler flags.
        var extra_flags = ["/bigobj"]

        for (flag in node.extraCompilerFlags) {
            extra_flags.add(flag)
        }

        var additionalOptions = extra_flags.join(" ") + " \%(AdditionalOptions)"

        // ItemDefinitionGroup (Compiler/Linker flags).
        var inc_dirs = node.includePaths.join(";")

        if (inc_dirs != "") {
            inc_dirs = inc_dirs + ";\%(AdditionalIncludeDirectories)"
        }

        var preprocessor_str = getPreprocessorDefs_(node)
        var lib_dirs = node.libraryPaths.join(";")

        if (lib_dirs != "") {
            lib_dirs = lib_dirs + ";\%(AdditionalLibraryDirectories)"
        }

        var deps = node.libraries.map { |l| l + ".lib" }.join(";")

        if (deps != "") {
            deps = deps + ";\%(AdditionalDependencies)"
        }

        for (config in configs) {
            var is_debug = config.startsWith("Debug")
            var platform = config.endsWith("x64") ? "x64" : "Win32"
            var crt = is_debug ? (node.dynamicCRT ? "MultiThreadedDebugDLL" : "MultiThreadedDebug") : (node.dynamicCRT ? "MultiThreadedDLL" : "MultiThreaded")
            var opt_level = is_debug ? "Disabled" : (node.optimizeForCodeSize ? "MinSpace" : "MaxSpeed")

            file.write("  <ItemDefinitionGroup Condition=\"'$(Configuration)|$(Platform)'=='%(config)'\">\r\n")
            file.write("    <ClCompile>\r\n")
            file.write("      <WarningLevel>%(getWarningLevelString_(node.warningLevel))</WarningLevel>\r\n")
            file.write("      <Optimization>%(opt_level)</Optimization>\r\n")
            if (inc_dirs != "") file.write("      <AdditionalIncludeDirectories>%(inc_dirs)</AdditionalIncludeDirectories>\r\n")
            file.write("      <PreprocessorDefinitions>%(is_debug ? "DEBUG" : "NDEBUG");%(preprocessor_str)%(platform == "x64" ? "WIN64;" : "WIN32;")%(config_type == "Application" ? "_CONSOLE;" : "")%(preprocessor_str)</PreprocessorDefinitions>\r\n")
            file.write("      <RuntimeLibrary>%(crt)</RuntimeLibrary>\r\n")
            file.write("      <RuntimeTypeInfo>%(node.enableRTTI ? "true" : "false")</RuntimeTypeInfo>\r\n")
            file.write("      <ExceptionHandling>%(node.enableExceptions ? "Sync" : "false")</ExceptionHandling>\r\n")
            file.write("      <AdditionalOptions>%(additionalOptions)</AdditionalOptions>\r\n")
            file.write("    </ClCompile>\r\n")

            if (!node.isStaticLibrary) {
                file.write("    <Link>\r\n")
                file.write("      <SubSystem>%(node.isGUI ? "Windows" : "Console")</SubSystem>\r\n")
                if (lib_dirs != "") file.write("      <AdditionalLibraryDirectories>%(lib_dirs)</AdditionalLibraryDirectories>\r\n")
                if (deps != "") file.write("      <AdditionalDependencies>%(deps)</AdditionalDependencies>\r\n")
                file.write("      <GenerateDebugInformation>%(is_debug ? "true" : "false")</GenerateDebugInformation>\r\n")
                file.write("    </Link>\r\n")
            }

            file.write("  </ItemDefinitionGroup>\r\n")
        }

        // Source Files.
        file.write("  <ItemGroup>\r\n")

        for (src in node.sources) {
            // Very rudimentary distinction, but fits standard C/C++ projects.
            if (src.endsWith(".c") || src.endsWith(".cpp") || src.endsWith(".cc") || src.endsWith(".cxx")) {
                file.write("    <ClCompile Include=\"%(src)\" />\r\n")
            } else if (src.endsWith(".h") || src.endsWith(".hpp")) {
                file.write("    <ClInclude Include=\"%(src)\" />\r\n")
            } else {
                file.write("    <None Include=\"%(src)\" />\r\n")
            }
        }

        file.write("  </ItemGroup>\r\n")

        file.write("  <Import Project=\"$(VCTargetsPath)\\Microsoft.Cpp.targets\" />\r\n")
        file.write("</Project>\r\n")

        if (file != File.stdout) {
            file.close()
        }
    }

    generateFilters_(node) {
        var file

        if (true) {
            file = File.open(node.name + ".vcxproj.filters", "w")
        } else {
            file = File.stdout
        }

        file.write("<?xml version=\"1.0\" encoding=\"utf-8\"?>\r\n")
        file.write("<Project ToolsVersion=\"4.0\" xmlns=\"http://schemas.microsoft.com/developer/msbuild/2003\">\r\n")

        file.write("  <ItemGroup>\r\n")
        file.write("    <Filter Include=\"Source Files\">\r\n")
        file.write("      <UniqueIdentifier>{4FC737F1-C7A5-4376-A066-2A32D752A2FF}</UniqueIdentifier>\r\n")
        file.write("    </Filter>\r\n")
        file.write("    <Filter Include=\"Header Files\">\r\n")
        file.write("      <UniqueIdentifier>{93995380-89BD-4b04-88EB-625FBE52EBFB}</UniqueIdentifier>\r\n")
        file.write("    </Filter>\r\n")
        file.write("  </ItemGroup>\r\n")

        file.write("  <ItemGroup>\r\n")

        for (src in node.sources) {
            if (src.endsWith(".c") || src.endsWith(".cpp") || src.endsWith(".cc") || src.endsWith(".cxx")) {
                file.write("    <ClCompile Include=\"%(src)\">\r\n")
                file.write("      <Filter>Source Files</Filter>\r\n")
                file.write("    </ClCompile>\r\n")
            } else if (src.endsWith(".h") || src.endsWith(".hpp")) {
                file.write("    <ClInclude Include=\"%(src)\">\r\n")
                file.write("      <Filter>Header Files</Filter>\r\n")
                file.write("    </ClInclude>\r\n")
            }
        }

        file.write("  </ItemGroup>\r\n")
        file.write("</Project>\r\n")

        if (file != File.stdout) {
            file.close()
        }
    }

    getConfigurationType_(node) {
        if (node.isSharedLibrary) {
            return "DynamicLibrary"
        }

        if (node.isStaticLibrary) {
            return "StaticLibrary"
        }

        return "Application"
    }

    getPreprocessorDefs_(node) {
        var list = []

        for (def in node.defines) {
            if (def is List) {
                list.add("%(def[0])=%(def[1])")
            } else if (def.contains("=")) {
                list.add(def)
            } else {
                list.add(def + "=1")
            }
        }

        if (list.isEmpty) {
            return ""
        }

        return list.join(";") + ";"
    }

    getWarningLevelString_(level) {
        if (level < 0) return "TurnOffAllWarnings" // /w
        if (level == 0) return "Level1"            // /W1
        if (level == 1) return "Level2"            // /W2
        if (level == 2) return "Level3"            // /W3
        if (level == 3) return "Level4"            // /W4
        return "EnableAllWarnings"                 // /Wall
    }
}

/*
================================================================================
 * ~~ [ misc. nodes ] ~~ *
--------------------------------------------------------------------------------
*/

/* Node for running arbitrary Wren code during the build or cleanup phase.
 */
class WrenNode {
    /*
     * TODO: async - spawn thread with its own VM, transfer results to main VM?
     */
    construct new(project, name, buildFunc, cleanFunc) {
        if (project == null) {
            _project = Project.new(null, name)
        } else {
            _project = project
        }

        _name = name != null ? name : _project.name.toString

        _buildFunc = buildFunc
        _cleanFunc = cleanFunc

        _finishBuildFunc = null
        _finishCleanFunc = null

        _project.nodes.add(this)
    }

    toString { "%(type)(%(name))" }

    // Cannot be changed, as we're in the projects node list.
    project { _project }

    name { _name }
    name=(value) { _name = value }

    buildFunc { _buildFunc }
    buildFunc=(value) { _buildFunc = value }

    cleanFunc { _cleanFunc }
    cleanFunc=(value) { _cleanFunc = value }

    finishBuildFunc { _finishBuildFunc }
    finishBuildFunc=(value) { _finishBuildFunc = value }

    finishCleanFunc { _finishCleanFunc }
    finishCleanFunc=(value) { _finishCleanFunc = value }

    build() {
        if (buildFunc == null) {
            return
        }

        if (buildFunc is String) {
            /*
             * Import locally in case we built without it.
             */
            import "meta" for Meta
            return Meta.eval(buildFunc)
        }

        return buildFunc.call()
    }

    clean() {
        if (cleanFunc == null) {
            return
        }

        if (cleanFunc is String) {
            import "meta" for Meta
            return Meta.eval(cleanFunc)
        }

        return cleanFunc.call()
    }

    finish(command) {
        if (command == "build") {
            if (finishBuildFunc == null) {
                return
            }

            if (finishBuildFunc is String) {
                import "meta" for Meta
                return Meta.eval(finishBuildFunc)
            }

            return finishBuildFunc.call()
        } else if (command == "clean") {
            if (finishCleanFunc == null) {
                return
            }

            if (finishCleanFunc is String) {
                import "meta" for Meta
                return Meta.eval(finishCleanFunc)
            }

            return finishCleanFunc.call()
        } else {
            Fiber.abort(command)
        }
    }
}

/* Node for running arbitrary programs during the build or cleanup phase.
 */
class ProcessNode {
    construct new(project, name, buildCommand, cleanCommand) {
        if (project == null) {
            _project = Project.new(null, name)
        } else {
            _project = project
        }

        _name = name != null ? name : _project.name.toString
        _verbose = _project.verbose

        _async = false

        _buildCommand = buildCommand
        _cleanCommand = cleanCommand

        _finishBuildCommand = null
        _finishCleanCommand = null

        _finalizeCommand = null

        _project.nodes.add(this)
    }

    toString { "%(type)(%(name))" }

    // Cannot be changed, as we're in the projects node list.
    project { _project }

    name { _name }
    name=(value) { _name = value }

    verbose { _verbose }
    verbose=(value) { _verbose = value }

    quiet { !_verbose }
    quiet=(value) { _verbose = !value }

    async { _async }
    async=(value) { _async = value }

    blocking { !_async }
    blocking=(value) { _async = !value }

    buildCommand { _buildCommand }
    buildCommand=(value) { _buildCommand = value }

    cleanCommand { _cleanCommand }
    cleanCommand=(value) { _cleanCommand = value }

    finishBuildCommand { _finishBuildCommand }
    finishBuildCommand=(value) { _finishBuildCommand = value }

    finishCleanCommand { _finishCleanCommand }
    finishCleanCommand=(value) { _finishCleanCommand = value }

    finalizeCommand { _finalizeCommand }
    finalizeCommand=(value) { _finalizeCommand = value }

    build() {
        var command = buildCommand

        if (finalizeCommand != null) {
            command = finalizeCommand.call("build", command)

            if (!(command is String || command == null)) {
                Fiber.abort("%(this).finalizeCommand must return a string!")
            }
        }

        if (command == null || command == "") {
            return
        }

        if (verbose) {
            System.print("running " + (async ? "async" : "blocking") + " build command \"%(command)\"")
        }

        if (async) {
            _buildProcess = Process.create(command)
        } else {
            /*
             * FIXME: Large stdout/stderr buffers hang the calling process, at least
             * on Linux (must test other systems). run also disables terminal colors.
             */
            Process.system(command)
        }
    }

    clean() {
        var command = cleanCommand

        if (finalizeCommand != null) {
            command = finalizeCommand.call("clean", command)

            if (!(command is String || command == null)) {
                Fiber.abort("%(this).finalizeCommand must return a string!")
            }
        }

        if (command == null || command == "") {
            return
        }

        if (verbose) {
            System.print("running " + (async ? "async" : "blocking") + " clean command \"%(command)\"")
        }

        if (async) {
            _cleanProcess = Process.create(command)
        } else {
            /*
             * FIXME: Large stdout/stderr buffers hang the calling process, at least
             * on Linux (must test other systems). run also disables terminal colors.
             */
            Process.system(command)
        }
    }

    finish(command) {
        if (command == "build") {
            if (_buildProcess != null) {
                var code = _buildProcess.join()

                System.write(_buildProcess.readStdout())

                if (code != 0) {
                    Fiber.abort(_buildProcess.readStderr())
                } else {
                    System.write(_buildProcess.readStderr())
                }
            }

            var finish_command = finishBuildCommand

            if (finalizeCommand != null) {
                finish_command = finalizeCommand.call("finish_build", finish_command)

                if (!(finish_command is String || finish_command == null)) {
                    Fiber.abort("%(this).finalizeCommand must return a string!")
                }
            }

            if (finish_command != null && finish_command != "") {
                if (verbose) {
                    System.print("running build completion command \"%(finish_command)\"")
                }

                /* FIXME: Large stdout/stderr buffers hang the calling process, at least
                 * on Linux (must test other systems). run also disables terminal colors.
                 */
                Process.system(finish_command)
            }
        } else if (command == "clean") {
            if (_cleanProcess != null) {
                var code = _cleanProcess.join()

                System.write(_cleanProcess.readStdout())

                if (code != 0) {
                    Fiber.abort(_cleanProcess.readStderr())
                } else {
                    System.write(_cleanProcess.readStderr())
                }
            }

            var finish_command = finishCleanCommand

            if (finalizeCommand != null) {
                finish_command = finalizeCommand.call("finish_clean", finish_command)

                if (!(finish_command is String || finish_command == null)) {
                    Fiber.abort("%(this).finalizeCommand must return a string!")
                }
            }

            if (finish_command != null && finish_command != "") {
                if (verbose) {
                    System.print("running clean completion command \"%(finish_command)\"")
                }

                /* FIXME: Large stdout/stderr buffers hang the calling process, at least
                 * on Linux (must test other systems). run also disables terminal colors.
                 */
                Process.system(finish_command)
            }
        } else {
            Fiber.abort(command)
        }
    }
}

/* Node for copying arbitrary files during the build phase.
 */
class CopyNode {
    construct new(project, name, src, dst) {
        if (project == null) {
            _project = Project.new(null, name)
        } else {
            _project = project
        }

        _src = src

        if (dst == null) {
            _dst = _src.split("\\")[-1]
            _dst = _dst.split("/")[-1]
        } else {
            _dst = dst
        }

        if (false) {
            _name = name != null ? name : _project.name.toString
        } else {
            _name = name != null ? name : _dst
        }

        _verbose = _project.verbose

        _deleteSrcOnClean = false
        _deleteDstOnClean = true

        _project.nodes.add(this)
    }

    toString { "%(type)(%(name))" }

    // Cannot be changed, as we're in the projects node list.
    project { _project }

    name { _name }
    name=(value) { _name = value }

    verbose { _verbose }
    verbose=(value) { _verbose = value }

    quiet { !_verbose }
    quiet=(value) { _verbose = !value }

    src { _src }
    src=(value) { _src = value }

    dst { _dst }
    dst=(value) { _dst = value }

    deleteSrcOnClean { _deleteSrcOnClean }
    deleteSrcOnClean=(value) { _deleteSrcOnClean = value }

    deleteDstOnClean { _deleteDstOnClean }
    deleteDstOnClean=(value) { _deleteDstOnClean = value }

    // TODO: extraProcessing

    build() {
        if (verbose) {
            System.print("copying \"%(src)\" to \"%(dst)\"")
        }

        Path.copy(src, dst)
    }

    clean() {
        if (deleteSrcOnClean) {
            if (verbose) {
                System.print("removing \"%(src)\"")
            }

            Path.tryRemove(src)
        }

        if (deleteDstOnClean) {
            if (verbose) {
                System.print("removing \"%(dst)\"")
            }

            Path.tryRemove(dst)
        }
    }

    finish(command) {
        //
    }
}

// TODO: RemoveNode (for clean)

/* Node for embedding arbitrary files into C programs or creating Wren modules.
 */
class HeaderNode {
    construct new(project, name, mode, src, dst) {
        if (project == null) {
            _project = Project.new(null, name)
        } else {
            _project = project
        }

        _name = name != null ? name : _project.name.toString
        _mode = mode

        _verbose = _project.verbose

        _src = src
        _dst = dst

        _deleteSrcOnClean = false
        _deleteDstOnClean = true

        _stripLeadingWhitespace = true
        _appendNewlines = true
        _terminator = !isModule
        _extraIndentation = isModule ? 4 : 0

        _project.nodes.add(this)
    }

    toString { "%(type)(%(name))" }

    // Cannot be changed, as we're in the projects node list.
    project { _project }

    name { _name }
    name=(value) { _name = value }

    verbose { _verbose }
    verbose=(value) { _verbose = value }

    quiet { !_verbose }
    quiet=(value) { _verbose = !value }

    mode { _mode }
    mode=(value) { _mode = value }

    isBinary {
        var mode = StringUtil.toLower(_mode)
        return mode == "binary"
    }

    isString {
        var mode = StringUtil.toLower(_mode)
        return mode == "string"
    }

    isText {
        var mode = StringUtil.toLower(_mode)
        return mode == "text"
    }

    isModule {
        var mode = StringUtil.toLower(_mode)
        return mode == "module"
    }

    src { _src }
    src=(value) { _src = value }

    dst { _dst }
    dst=(value) { _dst = value }

    deleteSrcOnClean { _deleteSrcOnClean }
    deleteSrcOnClean=(value) { _deleteSrcOnClean = value }

    deleteDstOnClean { _deleteDstOnClean }
    deleteDstOnClean=(value) { _deleteDstOnClean = value }

    stripLeadingWhitespace { _stripLeadingWhitespace }
    stripLeadingWhitespace=(value) { _stripLeadingWhitespace = value }

    appendNewlines { _appendNewlines }
    appendNewlines=(value) { _appendNewlines = value }

    terminator { _terminator }
    terminator=(value) { _terminator = value }

    extraIndentation { _extraIndentation }
    extraIndentation=(value) { _extraIndentation = value }

    // TODO: extraProcessing

    build() {
        if (verbose) {
            System.print("converting \"%(src)\" to %(mode) header \"%(dst)\"")
        }

        var src_file = File.open(src, "rb")
        var dst_file = File.open(dst, "w")

        dst_file.write("// This file was automatically generated from \"%(src)\". Do not edit!\n\n")

        if (isBinary) {
            headerizeBinary_(src_file, dst_file)
        } else if (isString) {
            headerizeString_(src_file, dst_file)
        } else if (isText) {
            headerizeText_(src_file, dst_file)
        } else if (isModule) {
            headerizeModule_(src_file, dst_file)
        } else {
            Fiber.abort(mode)
        }

        src_file.close()
        dst_file.close()
    }

    clean() {
        if (deleteSrcOnClean) {
            if (verbose) {
                System.print("removing %(mode) header input \"%(src)\"")
            }

            Path.tryRemove(src)
        }

        if (deleteDstOnClean) {
            if (verbose) {
                System.print("removing %(mode) header output \"%(dst)\"")
            }

            Path.tryRemove(dst)
        }
    }

    finish(command) {
        //
    }

    headerizeBinary_(src_file, dst_file) {
        var v = 0

        while (true) {
            var c = src_file.getc()

            if (c == File.EOF) {
                break
            }

            if (extraIndentation != 0) {
                dst_file.write(" " * extraIndentation)
            }

            dst_file.write("0x%(NumUtil.hex8(c)), ")
            v = v + 1

            /* Keep lines under 80 horizontal chars.
             */
            if (v != 0 && v % 13 == 0) {
                dst_file.putc("\n")
            }
        }

        dst_file.putc("\n")
    }

    headerize_(src_file, dst_file, multi_line) {
        if (stripLeadingWhitespace) {
            while (true) {
                var line = src_file.readLine()

                /* TODO: Return null at end of file?
                 */
                if (line == "" && src_file.eof()) {
                    break
                }

                var indentation = (line.count - line.trimStart().count) + extraIndentation
                line = Util.escapeString(line.trim())

                /* TODO: Return null at end of file?
                 */
                if (line == "" && src_file.eof()) {
                    dst_file.write("\n")
                    continue
                }

                if (indentation != 0) {
                    dst_file.write(" " * indentation)
                }

                dst_file.write("\"")
                dst_file.write(line)

                if (appendNewlines) {
                    dst_file.write("\\n")
                }

                if (multi_line) {
                    dst_file.write("\",\n")
                } else {
                    dst_file.write("\"\n")
                }
            }
        } else {
            while (true) {
                var line = src_file.readLine()

                /* TODO: Return null at end of file?
                 */
                if (line == "" && src_file.eof()) {
                    break
                }

                line = Util.escapeString(line.trimEnd())

                dst_file.write("\"")
                dst_file.write(line)

                if (appendNewlines) {
                    dst_file.write("\\n")
                }

                if (multi_line) {
                    dst_file.write("\",\n")
                } else {
                    dst_file.write("\"\n")
                }
            }
        }

        if (terminator) {
            if (multi_line) {
                dst_file.write("NULL,\n")
            } else {
                dst_file.write(";\n")
            }
        }
    }

    headerizeText_(src_file, dst_file) {
        headerize_(src_file, dst_file, true)
    }

    headerizeString_(src_file, dst_file) {
        headerize_(src_file, dst_file, false)
    }

    headerizeModule_(src_file, dst_file) {
        /*
         * TODO: Assign multiple lines to a constant array, and iterate through (calling wrenCode on each line).
         * This is necessary as source files grow (as MSVC has a 65535-character size limit on string literals).
         */
        dst_file.write("#ifndef WRENCH_IMPLEMENTATION\n")
        dst_file.write("#define WRENCH_IMPLEMENTATION 1\n")
        dst_file.write("#endif\n")
        dst_file.write("#include <wrench.h>\n")
        dst_file.write("\n")
        dst_file.write("/*\n")
        dst_file.write("================================================================================\n")
        dst_file.write(" * ~~ [ (un)hook ] ~~ *\n")
        dst_file.write("--------------------------------------------------------------------------------\n")
        dst_file.write("*/\n")
        dst_file.write("\n")
        dst_file.write("#if WRENCH_%(StringUtil.toUpper(name))_EXTENDED\n")
        dst_file.write("    /*\n")
        dst_file.write("     * Enable user extension of stdlib modules.\n")
        dst_file.write("     */\n")
        dst_file.write("    #ifndef __%(StringUtil.toUpper(name))_EX_INL__\n")
        dst_file.write("    #include <%(StringUtil.toLower(name))_ex.inl>\n")
        dst_file.write("    #endif\n")
        dst_file.write("#else\n")
        dst_file.write("    static bool %(StringUtil.toLower(name))WrenInitEx(WrenVM* vm)\n")
        dst_file.write("    {\n")
        dst_file.write("        return true;\n")
        dst_file.write("    }\n")
        dst_file.write("\n")
        dst_file.write("    static void %(StringUtil.toLower(name))WrenQuitEx(void)\n")
        dst_file.write("    {\n")
        dst_file.write("        //\n")
        dst_file.write("    }\n")
        dst_file.write("#endif /* WRENCH_%(StringUtil.toUpper(name))_EXTENDED */\n")
        dst_file.write("\n")
        dst_file.write("WRENCH_EXPORT bool %(StringUtil.toLower(name))WrenInit(WrenVM* vm)\n")
        dst_file.write("{\n")
        dst_file.write("    if (!wrenBeginModule(vm, \"%(StringUtil.toLower(name))\"))\n")
        dst_file.write("    {\n")
        dst_file.write("        return false;\n")
        dst_file.write("    }\n")
        dst_file.write("\n")
        dst_file.write("    if (!wrenCode(vm,\n")
        dst_file.write("\n")

        headerizeString_(src_file, dst_file)

        dst_file.write("\n")
        dst_file.write("    )) { return false; }\n")
        dst_file.write("\n")
        dst_file.write("    if (!%(StringUtil.toLower(name))WrenInitEx(vm))\n")
        dst_file.write("    {\n")
        dst_file.write("        return false;\n")
        dst_file.write("    }\n")
        dst_file.write("\n")
        dst_file.write("    return wrenEndModule(vm);\n")
        dst_file.write("}\n")
        dst_file.write("\n")
        dst_file.write("WRENCH_EXPORT void %(StringUtil.toLower(name))WrenQuit(void)\n")
        dst_file.write("{\n")
        dst_file.write("    %(StringUtil.toLower(name))WrenQuitEx();\n")
        dst_file.write("}\n")
    }
}

/* Node for creating "unity builds", possibly transforming individual files.
 */
class AmalgamationNode {
    construct new(project, name) {
        if (project == null) {
            _project = Project.new(null, name)
        } else {
            _project = project
        }

        _name = name != null ? name : _project.name.toString
        _verbose = _project.verbose

        _sources = []
        _includesOnly = false
        _addFilenames = true
        _deleteOnClean = true
        _trimLeft = false
        _trimRight = false
        _replacements = []
        _extraProcessing = null

        _project.nodes.add(this)
    }

    toString { "%(type)(%(name))" }

    // Cannot be changed, as we're in the projects node list.
    project { _project }

    name { _name }
    name=(value) { _name = value }

    verbose { _verbose }
    verbose=(value) { _verbose = value }

    quiet { !_verbose }
    quiet=(value) { _verbose = !value }

    sources { _sources }
    sources=(value) { _sources = value }

    includesOnly { _includesOnly }
    includesOnly=(value) { _includesOnly = value }

    addFilenames { _addFilenames }
    addFilenames=(value) { _addFilenames = value }

    deleteOnClean { _deleteOnClean }
    deleteOnClean=(value) { _deleteOnClean = value }

    trimLeft { _trimLeft }
    trimLeft=(value) { _trimLeft = value }

    trimRight { _trimRight }
    trimRight=(value) { _trimRight = value }

    replacements { _replacements }
    replacements=(value) { _replacements = value }

    replace(old_string, new_string) {
        replacements.add([old_string, new_string])
    }

    extraProcessing { _extraProcessing }
    extraProcessing=(value) { _extraProcessing = value }

    build() {
        var dst_file = File.open(name, "w")

        if (verbose) {
            System.print("amalgamating %(sources.count) source files into \"%(name)\"" +
                                            (includesOnly ? " (#includes only)" : ""))
        }

        if (includesOnly) {
            for (filename in sources) {
                dst_file.write("#include <%(filename)>\n")
            }
        } else {
            for (filename in sources) {
                if (addFilenames) {
                    dst_file.write("// %(filename)\n\n")
                }

                var data = File.read(filename)

                // Strip Windows-style carriage returns.
                data = data.replace("\r", "")

                if (trimLeft || trimRight) {
                    var split = data.split("\n")

                    for (i in 0...split.count) {
                        var line = split[i]

                        if (trimLeft && trimRight) {
                            split[i] = line.trim()
                        } else if (trimLeft) {
                            split[i] = line.trimStart()
                        } else if (trimRight) {
                            split[i] = line.trimEnd()
                        } else {
                            Fiber.abort("???")
                        }
                    }

                    data = split.join("\n")
                }

                /* TODO: Evaluate/match using regular expressions here.
                 */
                for (replacement in replacements) {
                    data = data.replace(replacement[0], replacement[1])
                }

                if (extraProcessing != null) {
                    data = extraProcessing.call(filename, data)

                    if (!(data is String)) {
                        Fiber.abort("%(type).extraProcessing callback must return a string!")
                    }
                }

                dst_file.write(data)
            }
        }

        dst_file.close()
    }

    clean() {
        if (deleteOnClean) {
            if (verbose) {
                System.print("removing amalgamated source file \"%(name)\"")
            }

            Path.tryRemove(name)
        }
    }

    finish(command) {
        //
    }
}

// TODO: ZipArchiveNode

// TODO: DownloadNode

/*
================================================================================
 * ~~ [ main ] ~~ *
--------------------------------------------------------------------------------
*/

var main = Fn.new {
    /*
     * TODO: Remove mode and just use config.
     */
    var command
    var mode

    var previousEnsureCRLF = File.ensureCRLF

    if (false) {
        File.ensureCRLF = true
    }

    var firstArg = 2

    if (WrenVM.self.commandLine.count < 3) {
        command = "build"
    } else {
        command = StringUtil.toLower(WrenVM.self.commandLine[2])
        firstArg = 3
    }

    if (WrenVM.self.commandLine.count < 4) {
        mode = "debug"
    } else {
        mode = StringUtil.toLower(WrenVM.self.commandLine[3])
        firstArg = 4
    }

    var project = Project.new(null, "wrench")
    project.configure(Config.new().parseArgs(WrenVM.self.commandLine[firstArg..-1]))

    // Includes all standard library modules in the executable for easier distribution and use.
    var unity = WrenVM.self.commandLine.any { |arg| arg.trimStart("-") == "builtin-stdlib" }

    if (unity) {
        var headers = Project.new(null, "headers")

        HeaderNode.new(headers, "config", "module", "config.wren", "wrench_config.c")
        project.define("WRENCH_HAVE_CONFIG")

        HeaderNode.new(headers, "project", "module", "project.wren", "wrench_project.c")
        project.define("WRENCH_HAVE_PROJECT")

        HeaderNode.new(headers, "scheduler", "module", "scheduler.wren", "wrench_scheduler.c")
        project.define("WRENCH_HAVE_SCHEDULER")

        if (command == "build") {
            headers.build()
        } else if (command == "clean") {
            headers.clean()
        } else {
            Fiber.abort("Invalid command \"%(command)\".")
        }
    }

    if (true || !Path.isFile("wren.c")) {
        var amalgamator = AmalgamationNode.new(null, "wren.c")
        var include_headers = true

        if (include_headers) {
            amalgamator.sources.add("wren/src/include/wren.h")

            amalgamator.sources.add("wren/src/optional/wren_opt_meta.h")
            amalgamator.sources.add("wren/src/optional/wren_opt_random.h")

            amalgamator.sources.add("wren/src/vm/wren_common.h")
            amalgamator.sources.add("wren/src/vm/wren_core.h")
            amalgamator.sources.add("wren/src/vm/wren_utils.h")
            amalgamator.sources.add("wren/src/vm/wren_math.h")
            amalgamator.sources.add("wren/src/vm/wren_value.h")
            amalgamator.sources.add("wren/src/vm/wren_primitive.h")
            amalgamator.sources.add("wren/src/vm/wren_debug.h")
            amalgamator.sources.add("wren/src/vm/wren_compiler.h")
            amalgamator.sources.add("wren/src/vm/wren_vm.h")
        }

        // Make it easier to port to brand-new platforms, or create a new Wren implementation.
        var stub = WrenVM.self.commandLine.any { |arg| arg.trimStart("-") == "stub-wren" }

        if (stub) {
            amalgamator.sources.add("wren_stubs.c")
        } else {
            for (filename in Path.list("wren/src/optional")) {
                if (filename.endsWith(".c")) {
                    amalgamator.sources.add(filename)
                }
            }

            for (filename in Path.list("wren/src/vm")) {
                if (filename.endsWith(".c")) {
                    amalgamator.sources.add(filename)
                }
            }
        }

        amalgamator.extraProcessing = Fn.new { |filename, data|
            data = Util.patchWrenAmalgamation(filename, data)

            if (include_headers) {
                data = Util.removeWrenIncludes(data)
            }

            return data
        }

        if (command == "build") {
            amalgamator.build()
        } else if (command == "clean") {
            amalgamator.clean()
        } else {
            Fiber.abort("Invalid command \"%(command)\".")
        }
    }

    if (mode == "debug") {
        project.debug = true
    } else if (mode == "profile") {
        project.release = true
        project.finalizeCompilerCommandLine = Fn.new { |filename, data| data.replace("NDEBUG", "DEBUG") }
    } else if (mode == "release") {
        project.debug = false
    } else if (mode == "size") {
        project.debug = false
        project.optimizeForCodeSize = true
    } else if (mode == "speed") {
        project.debug = false
        project.optimizeForPerformance = true
    } else {
        Fiber.abort("Invalid mode \"%(mode)\".")
    }

    project.includePaths.add("wren/src/include")
    project.includePaths.add("wren/src/optional")
    project.includePaths.add("wren/src/vm")

    project.define("WRENCH_USE_STB_SPRINTF")

    // Enable building the script runner alone, without any of our standard library modules.
    var no_stdlib = WrenVM.self.commandLine.any { |arg| arg.trimStart("-") == "no-stdlib" }

    if (no_stdlib) {
        project.define("WREN_OPT_META", 0)
        project.define("WREN_OPT_RANDOM", 0)
    }

    if (false) {
        project.define("WREN_NAN_TAGGING", 0)
        project.define("WREN_COMPUTED_GOTO", 0)
        project.define("WREN_SWITCHED_GOTO", 0)
    }

    var wren = NativeNode.new(project, "wren", "static_library")

    if (Path.isFile("wren.c")) {
        wren.sources.add("wren.c")
    } else {
        wren.includePaths.add("wren/src/optional")
        wren.includePaths.add("wren/src/vm")

        wren.sources.add("wren/src/optional/wren_opt_meta.c")
        wren.sources.add("wren/src/optional/wren_opt_random.c")

        wren.sources.add("wren/src/vm/wren_compiler.c")
        wren.sources.add("wren/src/vm/wren_core.c")
        wren.sources.add("wren/src/vm/wren_debug.c")
        wren.sources.add("wren/src/vm/wren_primitive.c")
        wren.sources.add("wren/src/vm/wren_utils.c")
        wren.sources.add("wren/src/vm/wren_value.c")
        wren.sources.add("wren/src/vm/wren_vm.c")
    }

    project.libraries.add("wren")

    var run_wren = NativeNode.new(project, "run_wren", "exe")
    run_wren.sources.add("wrench_main.c")

    if (unity) {
        run_wren.define("WRENCH_STDLIB")
    } else if (!no_stdlib) {
        var node

        node = NativeNode.new(project, "file", "shared_library")
        node.sources.add("wrench_file.c")

        node = NativeNode.new(project, "image", "shared_library")
        node.sources.add("wrench_image.c")

        node = NativeNode.new(project, "platform", "shared_library")
        node.sources.add("wrench_platform.c")

        node = NativeNode.new(project, "process", "shared_library")
        node.sources.add("wrench_process.c")

        node = NativeNode.new(project, "rect", "shared_library")
        node.sources.add("wrench_rect.c")

        node = NativeNode.new(project, "time", "shared_library")
        node.sources.add("wrench_time.c")

        node = NativeNode.new(project, "util", "shared_library")
        node.sources.add("wrench_util.c")

        node = NativeNode.new(project, "vector", "shared_library")
        node.sources.add("wrench_vector.c")

        node = NativeNode.new(project, "vm", "shared_library")
        node.sources.add("wrench_vm.c")

        node = NativeNode.new(project, "zip", "shared_library")
        node.sources.add("wrench_zip.c")

        if (Path.isFile("wrench_config.c")) {
            node = NativeNode.new(project, "config", "shared_library")
            node.sources.add("wrench_config.c")
        }

        if (Path.isFile("wrench_project.c")) {
            node = NativeNode.new(project, "project", "shared_library")
            node.sources.add("wrench_project.c")
        }

        if (Path.isFile("wrench_scheduler.c")) {
            node = NativeNode.new(project, "scheduler", "shared_library")
            node.sources.add("wrench_scheduler.c")
        }

        if (false) {
            node = NativeNode.new(project, "tcc", "shared_library")
            node.sources.add("wrench_tcc.c")
            node.libraries.add("tcc")
        }
    }

    if (command == "build") {
        project.build()
    } else if (command == "clean") {
        project.clean()
    } else {
        Fiber.abort("Invalid command \"%(command)\".")
    }

    File.ensureCRLF = previousEnsureCRLF
}
