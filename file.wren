/* -----------------------------------------------------------------------------
--- Copyright (c) 2012-2026 Adam Schackart / "AJ Hackman", all rights reserved.
--- Distributed under the BSD license v2 (opensource.org/licenses/BSD-3-Clause)
----------------------------------------------------------------------------- */

foreign class Path {
    // TODO: exists
    // TODO: current
    // TODO: base

    // TODO: home
    // TODO: desktop
    // TODO: documents
    // TODO: downloads
    // TODO: music
    // TODO: pictures
    // TODO: public_share
    // TODO: saved_games
    // TODO: screenshots
    // TODO: templates
    // TODO: videos

    // TODO: path
    // TODO: fileName
    // TODO: extension

    // TODO: split
    // TODO: join

    // TODO: createDirectory
    // TODO: createFile
    // TODO: copyFile
    // TODO: moveFile
    // TODO: deleteFile

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
        var EOF = type.EOF
        var s = []

        for (i in 0...count) {
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

    static read(path) {
        var file = open(path, "rb")
        var data = file.read()

        file.close()
        return data
    }

    // TODO: write
    // TODO: seek
    // TODO: tell
    // TODO: size

    foreign flush()

    readLine(strip_newlines) {
        var s = []

        while (!eof()) {
            s.insert(-1, read(1))

            if (s[-1] == "\n") {
                if (strip_newlines) {
                    s.removeAt(-1)
                }

                break
            }
        }

        return s.join()
    }

    readLine() { readLine(true) }

    readLines(strip_newlines) {
        var s = []

        while (!eof()) {
            s.insert(-1, readLine(strip_newlines))
        }

        return s
    }

    readLines() { readLines(true) }

    static readLines(path) {
        var file = open(path, "rb")
        var text = file.readLines()

        file.close()
        return text
    }
}
