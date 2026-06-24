/* -----------------------------------------------------------------------------
--- Copyright (c) 2012-2026 Adam Schackart / "AJ Hackman", all rights reserved.
--- Distributed under the BSD license v2 (opensource.org/licenses/BSD-3-Clause)
----------------------------------------------------------------------------- */
import "random" for Random

class MarkovChain {
    construct new(order) {
        if (!(order is Num) || order < 1 || !order.isInteger) {
            Fiber.abort("Markov chain order must be a positive integer.")
        }

        _random = Random.new()
        _order = order
        _transitions = {}
    }

    train(text) {
        /*
         * Normalize whitespace and filter out empty tokens caused by multiple spaces.
         */
        var words = text.replace("\n", " ")
                        .replace("\r", " ")
                        .replace("\t", " ")
                        .split(" ")
                        .where { |w| w != "" }
                        .toList

        var limit = words.count - _order
        if (limit <= 0) return

        for (i in 0...limit) {
            var prefix = words[i...(i + _order)].join(" ")
            var suffix = words[i + _order]

            var nextWords = _transitions[prefix]
            if (!nextWords) _transitions[prefix] = nextWords = []

            nextWords.add(suffix)
        }
    }

    generate(length) {
        if (!(length is Num) || length <= 0 || !length.isInteger) {
            Fiber.abort("Length must be a positive integer.")
        }

        if (_transitions.isEmpty) {
            return ""
        }

        var keys = _transitions.keys.toList
        var currentPrefixStr = keys[_random.int(keys.count)]
        var result = currentPrefixStr.split(" ")

        if (length <= _order) {
            return result[0...length].join(" ")
        }

        // Sliding window prevents allocating new slices from the expanding result list.
        var window = result[0..-1]

        while (result.count < length) {
            var nextWords = _transitions[currentPrefixStr]

            if (!nextWords || nextWords.isEmpty) {
                break
            }

            var nextWord = nextWords[_random.int(nextWords.count)]
            result.add(nextWord)

            // TODO: Specialized call to shift all list items back one and replace head.
            window.removeAt(0)
            window.add(nextWord)

            currentPrefixStr = window.join(" ")
        }

        return result.join(" ")
    }
}

var main = Fn.new {
    import "file" for File
    import "vm" for WrenVM

    if (WrenVM.self.commandLine.count < 3) {
        Fiber.abort("Usage: run_wren markov [filename]")
    }

    var markov = MarkovChain.new(2)

    markov.train(File.read(WrenVM.self.commandLine[2]))
    System.print(markov.generate(60))
}
