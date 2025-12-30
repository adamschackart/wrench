/* -----------------------------------------------------------------------------
--- Copyright (c) 2012-2026 Adam Schackart / "AJ Hackman", all rights reserved.
--- Distributed under the BSD license v2 (opensource.org/licenses/BSD-3-Clause)
----------------------------------------------------------------------------- */
import "file" for File, Path
import "platform" for Platform
import "process" for Process
import "util" for NumUtil, StringUtil
import "vm" for WrenVM

/*
================================================================================
 * ~~ [ utilities ] ~~ *
--------------------------------------------------------------------------------
*/

/* XXX: This should go in StringUtil, but we've got enough escape chars here.
 */
var escapeString = Fn.new { |s|
    return s.replace("\\", "\\\\")
            .replace("\"", "\\\"")
            .replace("\n", "\\n")
            .replace("\r", "\\r")
            .replace("\t", "\\t")
}

var patchWrenAmalgamation = Fn.new { |filename, data|
    if (filename == "wren/src/vm/wren_core.c") {
        var index
        var lines = File.readLines("wren/src/vm/wren_core.wren")

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

        /* Faster Sequence.where (removes redunant call to iteratorValue), thanks to Thorben Krüger.
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

        /* Headerize core module source.
         */
        data = data.replace("#include \"wren_core.wren.inc\"",

        "static const char* coreModuleSource =\n" +
        lines.map { |line| "\"" + escapeString.call(line) + "\\n\"" }.join("\n") +
        ";")
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
            "      #if _MSC_VER",
            "        hash = _rotl(hash, 7) + string->value[i];",
            "      #elif __GNUC__",
            "        hash = __builtin_rotateleft32(hash, 7) + string->value[i];",
            "      #else",
            "        hash = ((hash << 7) | (hash >> (32 - 7))) + string->value[i];",
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
    }

    return data
}

/*
================================================================================
 * ~~ [ project ] ~~ *
--------------------------------------------------------------------------------
*/

class Project {
    /*
     * TODO: Export to cmake, premake, make, Visual Studio project files, etc.
     */
    construct new(project, name) {
        _project = _project
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
        _includePaths = _project != null ? _project.includePaths.toList : []
        _debug = _project != null ? _project.debug : WrenVM.debug
        _verbose = _project != null ? _project.verbose : true
        _defines = _project != null ? _project.defines.toList : []
        _undefs = _project != null ? _project.undefs.toList : []
        _extraCompilerFlags = _project != null ? _project.extraCompilerFlags.toList : []
        _extraLinkerFlags = _project != null ? _project.extraLinkerFlags.toList : []
        _extraObjects = _project != null ? _project.extraObjects.toList : []
        _libraries = _project != null ? _project.libraries.toList : []
        _async = _project != null ? _project.async : true
        _enableRTTI = _project != null ? _project.enableRTTI : false
        _enableExceptions = _project != null ? _project.enableExceptions : false
        _isGUI = _project != null ? _project.isGUI : false
        _linkTimeOptimization = _project != null ? _project.linkTime_linkTimeOptimization : true

        /* TODO: Platform.logicalCoreCount * 2
         */
        _maxAsyncCompileJobs = _project != null ? _project.maxAsyncCompileJobs : 16
        _optimizeForCodeSize = _project != null ? _project.optimizeForCodeSize : true

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

    extraCompilerFlags { _extraCompilerFlags }
    extraCompilerFlags=(value) { _extraCompilerFlags = value }

    extraLinkerFlags { _extraLinkerFlags }
    extraLinkerFlags=(value) { _extraLinkerFlags = value }

    // TODO: linkerScript (ld -T)
    // TODO: libraryPaths
    // TODO: runtimeLibraryPaths
    // TODO: noStandardLibrary (use ld, -nostartfiles, -nodefaultlibs, and/or -nostdlib etc.)
    // TODO: build32bit (vcvars32 on MSVC, -m32 elsewhere)
    // TODO: extraWarnings (-Wall, -Wextra, -Wpedantic, etc)
    // TODO: extraCleanFiles
    // TODO: extraCompilerFlagsPerFile ({ filename : flags })
    // TODO: extraLinkerFlagsPerFile ({ filename : flags })

    // TODO: addressSanitizer
    // TODO: threadSanitizer
    // TODO: undefinedBehaviorSanitizer
    // TODO: integerSanitizer

    optimizeForCodeSize { _optimizeForCodeSize }
    optimizeForCodeSize=(value) { _optimizeForCodeSize = value }

    optimizeForPerformance { !optimizeForCodeSize }
    optimizeForPerformance=(value) { optimizeForCodeSize = !value }

    extraObjects { _extraObjects }
    extraObjects=(value) { _extraObjects = value }

    libraries { _libraries }
    libraries=(value) { _libraries = value }

    async { _async }
    async=(value) { _async = value }

    maxAsyncCompileJobs { _maxAsyncCompileJobs }
    maxAsyncCompileJobs=(value) { maxAsyncCompileJobs = value }

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

    isGUI { _isGUI }
    isGUI=(value) { _isGUI = value }

    isCLI { !isGUI }
    isCLI=(value) { isGUI = !value }

    // TODO: configure(cfg) - get config.Config values for compiler, linker, async, other flags.

    build() {
        /*
         * FIXME: This uses the C clock() function, which can be low-resolution and imprecise.
         * Need to create a time module that calls QueryPerformanceCounter (or clock_gettime).
         * EDIT: We could also just patch System directly (see wren_core.c for inspiration)...
         */
        //var start_time = System.clock

        for (node in nodes) {
            node.build()
        }

        /* For joining asynchronous tasks, running code after building it, etc.
         */
        for (node in nodes) {
            node.finish("build")
        }

        /*if (verbose) {
            System.print("%(this).build done in %(System.clock - start_time) seconds.")
        }*/
    }

    clean() {
        /*
         * FIXME: This uses the C clock() function, which can be low-resolution and imprecise.
         * Need to create a time module that calls QueryPerformanceCounter (or clock_gettime).
         * EDIT: We could also just patch System directly (see wren_core.c for inspiration)...
         */
        //var start_time = System.clock

        for (node in nodes) {
            node.clean()
        }

        for (node in nodes) {
            node.finish("clean")
        }

        /*if (verbose) {
            System.print("%(this).clean done in %(System.clock - start_time) seconds.")
        }*/
    }

    // ===== [ private utils ] =================================================

    ensureVisualStudioCompilerSetup_() {
        /*
         * Find/run vcvarsall so we don't have to use Developer Command Prompt on Win32.
         */
        if (__msvcIsInit == true) {
            return
        }

        __msvcIsInit = true

        // ===== [ 2026 ] ======================================================

        // TODO

        // ===== [ 2022 ] ======================================================

        if (Path.isFile("C:\\Program Files\\Microsoft Visual Studio\\2022\\Enterprise\\VC\\Auxiliary\\Build\\vcvarsall.bat")) {
            Fiber.abort("TODO")
        }

        if (Path.isFile("C:\\Program Files\\Microsoft Visual Studio\\2022\\Professional\\VC\\Auxiliary\\Build\\vcvarsall.bat")) {
            Fiber.abort("TODO")
        }

        if (Path.isFile("C:\\Program Files\\Microsoft Visual Studio\\2022\\Community\\VC\\Auxiliary\\Build\\vcvarsall.bat")) {
            Fiber.abort("TODO")
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

class ForeignNode {
    /*
     * TODO: Node base class to avoid some of this repetitive boilerplate.
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
        _extraCompilerFlags = _project.extraCompilerFlags.toList
        _extraLinkerFlags = _project.extraLinkerFlags.toList
        _extraObjects = _project.extraObjects.toList
        _libraries = _project.libraries.toList
        _async = _project.async
        _maxAsyncCompileJobs = _project.maxAsyncCompileJobs
        _optimizeForCodeSize = _project.optimizeForCodeSize
        _enableRTTI = _project.enableRTTI
        _enableExceptions = _project.enableExceptions
        _isGUI = _project.isGUI
        _linkTimeOptimization = _project.linkTimeOptimization

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

    extraCompilerFlags { _extraCompilerFlags }
    extraCompilerFlags=(value) { _extraCompilerFlags = value }

    extraLinkerFlags { _extraLinkerFlags }
    extraLinkerFlags=(value) { _extraLinkerFlags = value }

    // TODO: linkerScript (ld -T)
    // TODO: libraryPaths
    // TODO: runtimeLibraryPaths
    // TODO: noStandardLibrary (use ld, -nostartfiles, -nodefaultlibs, and/or -nostdlib etc.)
    // TODO: build32bit (vcvars32 on MSVC, -m32 elsewhere)
    // TODO: extraWarnings (-Wall, -Wextra, -Wpedantic, etc)
    // TODO: extraCleanFiles
    // TODO: extraCompilerFlagsPerFile ({ filename : flags })
    // TODO: extraLinkerFlagsPerFile ({ filename : flags })

    // TODO: addressSanitizer
    // TODO: threadSanitizer
    // TODO: undefinedBehaviorSanitizer
    // TODO: integerSanitizer

    optimizeForCodeSize { _optimizeForCodeSize }
    optimizeForCodeSize=(value) { _optimizeForCodeSize = value }

    optimizeForPerformance { !optimizeForCodeSize }
    optimizeForPerformance=(value) { optimizeForCodeSize = !value }

    extraObjects { _extraObjects }
    extraObjects=(value) { _extraObjects = value }

    libraries { _libraries }
    libraries=(value) { _libraries = value }

    async { _async }
    async=(value) { _async = value }

    maxAsyncCompileJobs { _maxAsyncCompileJobs }
    maxAsyncCompileJobs=(value) { maxAsyncCompileJobs = value }

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

    isGUI { _isGUI }
    isGUI=(value) { _isGUI = value }

    isCLI { !isGUI }
    isCLI=(value) { isGUI = !value }

    compile() {
        var jobs = []

        var flushJobs = Fn.new {
            for (process in jobs) {
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

            if (async) {
                jobs.add(Process.create(commandLine))

                if (jobs.count == maxAsyncCompileJobs) {
                    flushJobs.call()
                }
            } else {
                if (Process.run(commandLine) != 0) {
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

        var commandLine = linkerCommandLine_

        if (verbose) {
            System.print(commandLine)
        }

        if (Process.run(commandLine) != 0) {
            Fiber.abort("%(project).%(this) failed to link!")
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
                Path.tryRemove(name + ".so")
            }
        } else if (isStaticLibrary) {
            if (Platform.isWindows) {
                Path.tryRemove(name + ".lib")
            } else {
                Path.tryRemove(name + ".a")
            }
        }

        if (Platform.isWindows) {
            Path.tryRemove(name + ".exp")
        }
    }

    finish(command) {
        //
    }

    // ===== [ platform-specific implementation details (compiler) ] ===========

    compilerName_(filename) {
        var compilerName = compiler

        /* Call into the Visual Studio macro assembler instead.
         */
        if (compilerName == "cl" && filename.endsWith(".S")) {
            compilerName = "ml"
        }

        return compilerName + " "
    }

    compilerIncludePaths_ {
        /*
         * TODO: Ensure paths are properly escaped.
         */
        var s = []

        if (compiler == "cl") {
            for (path in includePaths) {
                s.add("/I%(path) ")
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
                    /*
                     * TODO: Profile difference.
                     */
                    if (false) {
                        s.add("/O1")
                    } else if (false) {
                        s.add("/Oi")
                    } else {
                        s.add("/Os")
                    }
                } else {
                    /*
                     * TODO: Profile difference.
                     */
                    if (true) {
                        s.add("/Ox")
                    } else if (true) {
                        s.add("/O2")
                    } else {
                        s.add("/Ot")
                    }
                }

                s.add("/DNDEBUG=1")
                s.add("/INCREMENTAL:NO")
            }
        } else {
            if (debug) {
                s.add("-O0")
                s.add("-g")
                s.add("-DDEBUG=1")
            } else {
                if (optimizeForCodeSize) {
                    s.add("-Os")
                } else {
                    s.add("-O3")
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
        } else {
            s.add("-fPIC")
            s.add("-c")

            // For very large builds.
            s.add("-gsplit-dwarf")

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
        return (compilerName_(filename) +
                compilerIncludePaths_ +
                compilerOptimization_ +
                compilerDefines_ +
                compilerUndefines_ +
                compilerPlatformFlags_ +
                compilerExtraFlags_ +
                filename +
                compilerLibraries_ +
                compilerLibraryPaths_)
    }

    // ===== [ platform-specific implementation details (linker) ] =============

    linkerName_ {
        var linkerName = linker

        if (isStaticLibrary) {
            if (linkerName == "link") {
                linkerName = "lib"
            } else if (linkerName == "tcc") {
                linkerName = "tcc -ar"
            } else {
                // TODO: Profile the difference here.
                linkerName = /*"gcc-" +*/ "ar rcs"
            }
        }

        return linkerName + " "
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
        } else {
            /*
             * XXX: Some versions of `ar` don't like link-time optimization.
             */
            if (release && linkTimeOptimization && !isStaticLibrary) {
                s.add("-flto")
            }
        }

        if (s.isEmpty) {
            return ""
        } else {
            return s.join(" ") + " "
        }
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

            if (Platform.isWindows) {
                s.add(name + ".exe")
            } else {
                s.add(name)
            }
        } else if (isSharedLibrary) {
            if (linker == "link") {
                s.add("/DLL /OUT")
            } else {
                s.add("-fPIC -shared -o")
            }

            if (Platform.isWindows) {
                s.add(name + ".dll")
            } else {
                s.add(name + ".so")
            }
        } else if (isStaticLibrary) {
            if (linker == "link") {
                s.add("/OUT")
            } else {
                return name + ".a" + " "
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

    platformObjectExtension_ {
        if (Platform.isWindows) {
            return ".obj"
        } else {
            return ".o"
        }
    }

    sourceToObject_(source) {
        /*
         * XXX TODO FIXME: This is an ugly and gross hack. Strip extension using the `file.Path` module.
         */
        var s = source.replace(".cpp", platformObjectExtension_).replace(".c", platformObjectExtension_)

        s = s.split("\\")[-1]
        s = s.split("/")[-1]

        return s
    }

    sourceToDWARF_(source) {
        /*
         * XXX TODO FIXME: This is an ugly and gross hack. Strip extension using the `file.Path` module.
         */
        var s = source.replace(".cpp", ".dwo").replace(".c", ".dwo")

        s = s.split("\\")[-1]
        s = s.split("/")[-1]

        return s
    }

    linkerObjects_ {
        var s = []

        for (source in sources) {
            s.add(sourceToObject_(source))
        }

        for (object in extraObjects) {
            /*
             * TODO: If name has no extension, append platform object extension.
             */
            s.add(object)
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
                if (Path.isFile(library + ".a")) {
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

        if (s.isEmpty) {
            return ""
        } else {
            return " " + s.join(" ")
        }
    }

    linkerLibraryPaths_ {
        /*
         * TODO: libraryPaths with ["."] as the default.
         */
        if (isStaticLibrary || linker == "link") {
            return ""
        } else {
            return " -L."
        }
    }

    linkerCommandLine_ {
        return (linkerName_ +
                linkerPlatformFlags_ +
                linkerExtraFlags_ +
                linkerOutput_ +
                linkerObjects_ +
                linkerLibraries_ +
                linkerLibraryPaths_)
    }
}

/*
================================================================================
 * ~~ [ misc. nodes ] ~~ *
--------------------------------------------------------------------------------
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

        _project.nodes.add(this)
    }

    toString { "%(type)(%(name))" }

    // Cannot be changed, as we're in the projects node list.
    project { _project }

    name { _name }
    name=(value) { _name = value }

    verbose { _verbose }
    verbose=(value) { _verbose = value }

    async { _async }
    async=(value) { _async = value }

    buildCommand { _buildCommand }
    buildCommand=(value) { _buildCommand = value }

    cleanCommand { _cleanCommand }
    cleanCommand=(value) { _cleanCommand = value }

    finishBuildCommand { _finishBuildCommand }
    finishBuildCommand=(value) { _finishBuildCommand = value }

    finishCleanCommand { _finishCleanCommand }
    finishCleanCommand=(value) { _finishCleanCommand = value }

    build() {
        if (buildCommand == null || buildCommand == "") {
            return
        }

        if (verbose) {
            System.print("running " + (async ? "async" : "blocking") + " build command \"%(buildCommand)\"")
        }

        if (async) {
            _buildProcess = Process.create(buildCommand)
        } else {
            Process.run(buildCommand)
        }
    }

    clean() {
        if (cleanCommand == null || cleanCommand == "") {
            return
        }

        if (verbose) {
            System.print("running " + (async ? "async" : "blocking") + " clean command \"%(cleanCommand)\"")
        }

        if (async) {
            _cleanProcess = Process.create(cleanCommand)
        } else {
            Process.run(cleanCommand)
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

            if (finishBuildCommand != null && finishBuildCommand != "") {
                if (verbose) {
                    System.print("running build completion command \"%(finishBuildCommand)\"")
                }

                Process.run(finishBuildCommand)
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

            if (finishCleanCommand != null && finishCleanCommand != "") {
                if (verbose) {
                    System.print("running clean completion command \"%(finishCleanCommand)\"")
                }

                Process.run(finishCleanCommand)
            }
        } else {
            Fiber.abort(command)
        }
    }
}

class CopyNode {
    construct new(project, name, src, dst) {
        if (project == null) {
            _project = Project.new(null, name)
        } else {
            _project = project
        }

        _name = name != null ? name : _project.name.toString
        _verbose = _project.verbose

        _src = src
        _dst = dst

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

    src { _src }
    src=(value) { _src = value }

    dst { _dst }
    dst=(value) { _dst = value }

    deleteSrcOnClean { _deleteSrcOnClean }
    deleteSrcOnClean=(value) { _deleteSrcOnClean = value }

    deleteDstOnClean { _deleteDstOnClean }
    deleteDstOnClean=(value) { _deleteDstOnClean = value }

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

                if (line == "") {
                    break
                }

                var indentation = (line.count - line.trimStart().count) + extraIndentation
                line = escapeString.call(line.trim())

                if (line == "") {
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

                if (line == "") {
                    break
                }

                line = escapeString.call(line.trimEnd())

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
    var command
    var mode

    if (WrenVM.self.commandLine.count < 3) {
        command = "build"
    } else {
        command = StringUtil.toLower(WrenVM.self.commandLine[2])
    }

    if (WrenVM.self.commandLine.count < 4) {
        mode = "debug"
    } else {
        mode = StringUtil.toLower(WrenVM.self.commandLine[3])
    }

    var project = Project.new(null, "wrench")

    if (false) {
        var headers = Project.new(null, "headers")

        HeaderNode.new(headers, "config", "module", "config.wren", "wrench_config.c")
        project.define("WRENCH_HAVE_CONFIG")

        HeaderNode.new(headers, "project", "module", "project.wren", "wrench_project.c")
        project.define("WRENCH_HAVE_PROJECT")

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

        amalgamator.extraProcessing = patchWrenAmalgamation

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

    project.includePaths.add(".")
    project.includePaths.add("wren/src/include")

    project.includePaths.add("wren/src/optional")
    project.includePaths.add("wren/src/vm")

    if (project.compiler != "cl") {
        project.extraCompilerFlags.add("-Wno-format-truncation")
        project.extraCompilerFlags.add("-Wno-format-zero-length")
    }

    if (!Platform.isWindows) {
        project.libraries.add("m")
        project.libraries.add("dl")
    }

    if (false) {
        project.define("WREN_NAN_TAGGING", 0)
        project.define("WREN_COMPUTED_GOTO", 0)
        project.define("WREN_OPT_META", 0)
        project.define("WREN_OPT_RANDOM", 0)
    }

    var wren = ForeignNode.new(project, "wren", "static_library")

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

    var run_wren = ForeignNode.new(project, "run_wren", "exe")
    run_wren.sources.add("wrench_main.c")

    if (false) {
        run_wren.define("WRENCH_STDLIB")
    } else {
        var node

        node = ForeignNode.new(project, "file", "shared_library")
        node.sources.add("wrench_file.c")

        node = ForeignNode.new(project, "image", "shared_library")
        node.sources.add("wrench_image.c")

        node = ForeignNode.new(project, "platform", "shared_library")
        node.sources.add("wrench_platform.c")

        node = ForeignNode.new(project, "process", "shared_library")
        node.sources.add("wrench_process.c")

        node = ForeignNode.new(project, "rect", "shared_library")
        node.sources.add("wrench_rect.c")

        node = ForeignNode.new(project, "util", "shared_library")
        node.sources.add("wrench_util.c")

        node = ForeignNode.new(project, "vector", "shared_library")
        node.sources.add("wrench_vector.c")

        node = ForeignNode.new(project, "vm", "shared_library")
        node.sources.add("wrench_vm.c")

        node = ForeignNode.new(project, "zip", "shared_library")
        node.sources.add("wrench_zip.c")

        if (Path.isFile("wrench_config.c")) {
            node = ForeignNode.new(project, "config", "shared_library")
            node.sources.add("wrench_config.c")
        }

        if (Path.isFile("wrench_project.c")) {
            node = ForeignNode.new(project, "project", "shared_library")
            node.sources.add("wrench_project.c")
        }

        if (false) {
            node = ForeignNode.new(project, "tcc", "shared_library")
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
}
