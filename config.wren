/* -----------------------------------------------------------------------------
--- Copyright (c) 2012-2026 Adam Schackart / "AJ Hackman", all rights reserved.
--- Distributed under the BSD license v2 (opensource.org/licenses/BSD-3-Clause)
----------------------------------------------------------------------------- */
import "file" for File
import "util" for StringUtil
import "vm" for WrenVM

class Config {
    construct new() {
        _comments = {}
        _table = {}
        _ignore = {}
        _storeFallbacks = true
        _allowOverride = false
    }

    storeFallbacks { _storeFallbacks }
    storeFallbacks=(value) { _storeFallbacks = value }

    allowOverride { _allowOverride }
    allowOverride=(value) { _allowOverride = value }

    getComment(key) {
        if (_comments.containsKey(key)) {
            return _comments[key]
        } else {
            return ""
        }
    }

    setComment(key, value) {
        _comments[key] = value
        return this
    }

    getIgnored(key) { _ignore.containsKey(key) }

    setIgnored(key, ignored) {
        if (ignored) {
            _ignore[key] = true
        } else {
            _ignore.remove(key)
        }

        return this
    }

    ignore(key) { setIgnored(key, true) }

    get(key, fallback) {
        if (_table.containsKey(key)) {
            return _table[key]
        } else {
            if (_storeFallbacks) {
                _table[key] = fallback
            }

            return fallback
        }
    }

    get(key) { get(key, null) }

    set(key, value) {
        _table[key] = value
        return this
    }

    [key, fallback] { get(key, fallback) }
    [key] { get(key) }
    [key]=(value) { set(key, value) }

    /* TODO: Rather than calling toString here, ensure all values are strings in set().
     */
    getStr(key, fallback) { get(key, fallback).toString }
    setStr(key, value) { set(key, value) }

    getNum(key, fallback) { Num.fromString(getStr(key, fallback)) }
    setNum(key, value) { set(key, value) }

    getBool(key, fallback) {
        var s = StringUtil.toLower(getStr(key, fallback))

        if (s == "true" || s == "yes" || s == "on") {
            return true
        }

        if (s == "false" || s == "no" || s == "off") {
            return false
        }

        return fallback
    }

    setBool(key, value) { set(key, value) }

    parseLine_(line) {
        line = line.split(";")[0].trim().trimStart("-")

        if (line.count == 0) {
            return
        }

        var split = line.split("=")

        var lhs = split[0]
        var rhs

        if (split.count > 1) {
            rhs = split[1]
        } else {
            rhs = "true"
        }

        if (_ignore.containsKey(lhs)) {
            return
        }

        if (_allowOverride || !_table.containsKey(lhs)) {
            _table[lhs] = rhs
        }
    }

    parseArgs(args) {
        /*
         * TODO: Skip the name of the program + script?
         */
        for (i in 0...args.count) {
            parseLine_(args[i])
        }

        return this
    }

    parseArgs() { parseArgs(WrenVM.self.commandLine) }

    parseFile(file) {
        while (!file.eof()) {
            parseLine_(file.readLine(false))
        }

        return this
    }

    load(filename) {
        var file = File.open(filename, "r")
        var r = parseFile(file)

        file.close()
        return r
    }

    writeFile(file) {
        /*
         * Commented.
         */
        for (entry in _table) {
            if (_comments.containsKey(entry.key)) {
                file.write("; %(getComment(entry.key))\n%(entry.key)=%(entry.value)\n\n")
            }
        }

        /* Uncommented.
         */
        for (entry in _table) {
            if (!_comments.containsKey(entry.key)) {
                file.write("%(entry.key)=%(entry.value)\n")
            }
        }

        return this
    }

    save(filename) {
        var file = File.open(filename, "w")
        var r = writeFile(file)

        file.close()
        return r
    }

    printHelp(file) {
        for (entry in _comments) {
            file.write("--%(entry.key)\n\t%(entry.value)\n")
        }

        return this
    }

    printHelp() { printHelp(File.stdout) }
}
