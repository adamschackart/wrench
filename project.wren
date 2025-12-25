/* -----------------------------------------------------------------------------
--- Copyright (c) 2012-2026 Adam Schackart / "AJ Hackman", all rights reserved.
--- Distributed under the BSD license v2 (opensource.org/licenses/BSD-3-Clause)
----------------------------------------------------------------------------- */
import "file" for Path
import "platform" for Platform
import "process" for Process
import "util" for StringUtil
import "vm" for WrenVM

/*******************************************************************************
-* -----------------------------------------------------------------------------
-* - TODO - *-
-* -----------------------------------------------------------------------------
-* find/run vcvarsall so we don't have to use developer command prompt on Win32
-* export projects to cmake, premake, make, visual studio project files, etc.
-*
-* HeaderizeNode (binary, string, text)
-* (Process/Command)Node (build and/or clean may call Process.(run/system))
-* (Copy/Move)FileNode
-* AssemblerNode
-*
-* address sanitizer
-* thread sanitizer
-* undefined behavior sanitizer
-* integer sanitizer
-* -----------------------------------------------------------------------------
*******************************************************************************/

/*
================================================================================
 * ~~ [ project ] ~~ *
--------------------------------------------------------------------------------
*/

class Project {
    construct new(name) {
        _name = name != null ? name : "main"

        /* TODO: Check environment variables.
         */
        if (Platform.isWindows) {
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

        _nodes = []
        _sources = []
        _includePaths = []
        _debug = WrenVM.debug
        _verbose = true
        _defines = []
        _undefs = []
        _extraCompilerFlags = []
        _extraLinkerFlags = []
        _extraObjects = []
        _libraries = []
        _async = true
        _maxAsyncCompileJobs = 16 // TODO: Platform.logicalCoreCount * 2
        _optimizeForCodeSize = true
    }

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
        define(name, 1)
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
    // TODO: linkTimeOptimization
    // TODO: build32bit (vcvars32 on MSVC, -m32 elsewhere)

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

    build() {
        for (node in nodes) {
            node.build()
        }

        /* For joining asynchronous tasks, running code after building it, etc.
         */
        for (node in nodes) {
            node.finish("build")
        }
    }

    clean() {
        for (node in nodes) {
            node.clean()
        }

        for (node in nodes) {
            node.finish("clean")
        }
    }

    // ===== [ private utils ] =================================================

    ensureVisualStudioCompilerSetup_() {
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
    construct new(project, name, mode) {
        if (project == null) {
            _project = Project.new(name)
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
        define(name, 1)
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
    // TODO: linkTimeOptimization
    // TODO: build32bit (vcvars32 on MSVC, -m32 elsewhere)

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

    compilerName_ {
        return compiler + " "
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
        if (compiler == "cl") {
            if (debug) {
                return "/Od /Zi /DEBUG "
            } else {
                if (optimizeForCodeSize) {
                    /*
                     * TODO: Profile difference.
                     */
                    if (false) {
                        return "/O1 /DNDEBUG "
                    } else {
                        return "/Os /DNDEBUG "
                    }
                } else {
                    /*
                     * TODO: Profile difference.
                     */
                    if (true) {
                        return "/Ox /DNDEBUG "
                    } else if (true) {
                        return "/O2 /DNDEBUG "
                    } else {
                        return "/Ot /DNDEBUG "
                    }
                }
            }
        } else {
            if (debug) {
                return "-O0 -g "
            } else {
                if (optimizeForCodeSize) {
                    return "-Os "
                } else {
                    return "-O3 "
                }
            }
        }
    }

    compilerDefines_ {
        var s = []

        for (define in defines) {
            var d

            if (define is List) {
                d = define[0].toString + "=" + define[1].toString
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
            return " " + s.join(" ")
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
            return " " + s.join(" ")
        }
    }

    compilerPlatformFlags_ {
        /*
         * Only build object files, don't link.
         */
        if (compiler == "cl") {
            return "/c /nologo "
        } else {
            return "-fPIC -c "
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
        return (compilerName_ +
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
                linkerName = "ar rcs"
            }
        }

        return linkerName + " "
    }

    linkerPlatformFlags_ {
        if (linker == "link") {
            return "/nologo "
        } else {
            return ""
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

    var project = Project.new("wrench")

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

    if (project.compiler != "cl") {
        project.extraCompilerFlags.add("-Wno-format-truncation")
        project.extraCompilerFlags.add("-Wno-format-zero-length")
    }

    if (!Platform.isWindows) {
        project.libraries.add("m")
        project.libraries.add("dl")
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
