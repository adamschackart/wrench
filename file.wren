/* -----------------------------------------------------------------------------
--- Copyright (c) 2012-2026 Adam Schackart / "AJ Hackman", all rights reserved.
--- Distributed under the BSD license v2 (opensource.org/licenses/BSD-3-Clause)
----------------------------------------------------------------------------- */

foreign class Path {
    foreign static list(path, recursive, include_subdirectories)
    static list(path, recursive) { list(path, recursive, true) }
    static list(path) { list(path, false, true) }
    static walk(path) { list(path, true, true) }
}

foreign class File {
    foreign static open(path, mode)
    foreign close()

    // TODO: name
    // TODO: mode

    foreign static stdout
    foreign static stderr
    foreign static stdin

    foreign getc()
    foreign putc(c)

    foreign static EOF
    foreign eof()

    read(count) {
        /*
         * TODO: Native/foreign method for perf.
         */
        var s = []

        for (i in 0...count) {
            var EOF = File.EOF
            var c = getc()

            if (c < 0 || c == EOF) {
                break
            } else {
                s.insert(-1, String.fromByte(c))
            }
        }

        return s.join()
    }

    read() { read(Num.maxSafeInteger) }

    // TODO: write
    // TODO: seek
    // TODO: tell
    // TODO: size

    foreign flush()
}
