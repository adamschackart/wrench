/* -----------------------------------------------------------------------------
The JSON spec dictates that all object keys must be strings - we ignore it here.

Usage:
```
import "json" for JSON
JSON.parse(string)
JSON.stringify(object)
```
--------------------------------------------------------------------------------
The MIT License (MIT)

Copyright (c) 2015 Matthew Brandly
Copyright (c) 2026 Adam Schackart

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
----------------------------------------------------------------------------- */
import "random" for Random

import "util" for NumUtil
import "util" for StringUtil

import "vm" for WrenVM

/* ===== [ simple API ] ===================================================== */

class JSON {
    static parse(string) {
        return JSONParser.new(string).parse
    }

    static stringify(object) {
        return JSONStringifier.new(object).toString
    }

    static tokenize(string) {
        return JSONScanner.new(string).tokenize
    }

    /* Deterministic random JSON generation for testing and benchmarking.
     */
    static generate(seed, depth, breadth, probability) {
        return JSONRandomGenerator.new(seed, depth, breadth, probability).generate()
    }

    static generate(seed, depth, breadth) {
        return JSONRandomGenerator.new(seed, depth, breadth, 0.8).generate()
    }

    static generate(seed, depth) {
        return JSONRandomGenerator.new(seed, depth, 20, 0.8).generate()
    }

    static generate(seed) {
        return JSONRandomGenerator.new(seed, 2, 20, 0.8).generate()
    }

    static generate() {
        return JSONRandomGenerator.new(12345, 2, 20, 0.8).generate()
    }
}

/* ===== [ helpers ] ======================================================== */

class JSONStringifier {
    construct new(object) {
        _object = object
    }

    /* TODO: Append all strings into list and join them at the end.
     */
    toString { type.stringify_(_object) }

    static stringify_(obj) {
        if (obj is Num || obj is Bool || obj is Null) {
            return obj.toString
        } else if (obj is String) {
            return StringUtil.escapeAndQuote(obj)
        } else if (obj is List) {
            var substrings = obj.map { |element|
                return stringify_(element)
            }.toList

            return "[" + substrings.join(",") + "]"
        } else if (obj is Map) {
            var substrings = obj.keys.map { |key|
                return stringify_(key) + ":" + stringify_(obj[key])
            }.toList

            return "{" + substrings.join(",") + "}"
        } else if (WrenVM.self.objectHasMethod(obj, "toJSON")) {
            return obj.toJSON
        } else {
            Fiber.abort("Unexpected item in JSON writer: %(obj)")
        }
    }
}

class JSONParser {
    construct new(input) {
        _input = input
        _tokens = []
        _current = 0
    }

    parse {
        _tokens = JSONScanner.new(_input).tokenize
        _current = 0

        var result = nest_()

        if (_current < _tokens.count && peek_().type != Token.End) {
            parsingError_(peek_())
        }

        return result
    }

    valueTypes_ {
        return [Token.String, Token.Number, Token.Bool, Token.Null]
    }

    advance_() {
        var token = _tokens[_current]
        _current = _current + 1

        return token
    }

    peek_() {
        if (_current >= _tokens.count) {
            return _tokens[_tokens.count - 1]
        } else {
            return _tokens[_current]
        }
    }

    match_(type) {
        if (peek_().type == type) {
            advance_()

            return true
        } else {
            return false
        }
    }

    nest_() {
        if (_current >= _tokens.count) {
            parsingError_()
        }

        var token = advance_()

        if (token.type == Token.LeftBrace) {
            var map = {}

            while (peek_().type != Token.RightBrace) {
                var key = advance_()

                if (key.type != Token.String) {
                    parsingError_(key)
                }

                var next = advance_()

                if (next.type != Token.Colon) {
                    parsingError_(next)
                }

                map[key.value] = nest_()

                if (match_(Token.Comma)) {
                    if (peek_().type == Token.RightBrace || peek_().type == Token.End) {
                        parsingError_(peek_())
                    }
                } else if (peek_().type != Token.RightBrace) {
                    parsingError_(peek_())
                }
            }

            // Consume Token.RightBrace.
            advance_()

            return map
        } else if (token.type == Token.LeftBracket) {
            var list = []

            while (peek_().type != Token.RightBracket) {
                list.add(nest_())

                if (match_(Token.Comma)) {
                    if (peek_().type == Token.RightBracket) {
                        parsingError_(peek_())
                    }
                } else if (peek_().type != Token.RightBracket) {
                    parsingError_(peek_())
                }
            }

            // Consume Token.RightBracket.
            advance_()

            return list
        } else if (valueTypes_.contains(token.type)) {
            return token.value
        } else {
            parsingError_(token)
        }
    }

    parsingError_(token) {
        var position = Helper.getPositionForIndex(_input, token.index)
        invalidJSON_("Unexpected \"%(token)\" at line %(position["line"]), column %(position["column"])")
    }

    parsingError_() {
        invalidJSON_("")
    }

    invalidJSON_(message) {
        var base = "Invalid JSON"
        Fiber.abort(message.count > 0 ? "%(base): %(message)" : base)
    }
}

class JSONScanner {
    construct new(input) {
        _input = input
        _tokens = []

        // First unconsumed char.
        _start = 0

        // Char that will be considered next.
        _cursor = 0
    }

    numberChars_ {
        return "0123456789.-eE+"
    }

    whitespaceChars_ {
        return " \r\t\n"
    }

    escapedCharMap_ {
        return {
            "\"": "\"",
            "\\": "\\",
            "/": "/",
            "b": "\b",
            "f": "\f",
            "n": "\n",
            "r": "\r",
            "t": "\t",
        }
    }

    tokenize {
        while (!isAtEnd_()) {
            _start = _cursor
            scanToken_()
        }

        addToken_(Token.End)
        return _tokens
    }

    scanToken_() {
        var char = advance_()

        if (char == "{") {
            addToken_(Token.LeftBrace)
        } else if (char == "}") {
            addToken_(Token.RightBrace)
        } else if (char == "[") {
            addToken_(Token.LeftBracket)
        } else if (char == "]") {
            addToken_(Token.RightBracket)
        } else if (char == ":") {
            addToken_(Token.Colon)
        } else if (char == ",") {
            addToken_(Token.Comma)
        } else if (char == "/") {
            // Don't allow comments.
            scanningError_()
        } else if (char == "\"") {
            scanString_()
        } else if (numberChars_.contains(char)) {
            scanNumber_()
        } else if (isAlpha_(char)) {
            scanIdentifier_()
        } else if (whitespaceChars_.contains(char)) {
            // Pass.
        } else {
            scanningError_()
        }
    }

    scanString_() {
        var isEscaping = false
        var valueInProgress = []

        while ((peek_() != "\"" || isEscaping) && !isAtEnd_()) {
            var char = advance_()

            if (isEscaping) {
                if (escapedCharMap_.containsKey(char)) {
                    valueInProgress.add(escapedCharMap_[char])
                } else if (char == "u") { // Unicode char.
                    var charsToPull = 4
                    var start = _cursor

                    if (start + charsToPull > _input.count) {
                        scanningError_()
                    }

                    var hexString = _input[start...(start + charsToPull)]

                    var decimal = Helper.fromHex(hexString)
                    if (decimal == null) { scanningError_() }

                    valueInProgress.add(String.fromCodePoint(decimal))
                    _cursor = _cursor + charsToPull
                } else {
                    scanningError_()
                }

                isEscaping = false
            } else if (char == "\\") {
                isEscaping = true
            } else {
                valueInProgress.add(char)
            }
        }

        if (isAtEnd_()) {
            // Unterminated string.
            scanningError_()
        }

        // Consume closing ".
        advance_()

        addToken_(Token.String, valueInProgress.join(""))
    }

    scanNumber_() {
        while (numberChars_.contains(peek_())) {
            advance_()
        }

        var number = Num.fromString(_input[_start..._cursor])

        if (number == null) {
            scanningError_()
        } else {
            addToken_(Token.Number, number)
        }
    }

    scanIdentifier_() {
        while (isAlpha_(peek_())) {
            advance_()
        }

        var value = _input[_start..._cursor]

        if (value == "true") {
            addToken_(Token.Bool, true)
        } else if (value == "false") {
            addToken_(Token.Bool, false)
        } else if (value == "null") {
            addToken_(Token.Null, null)
        } else {
            scanningError_()
        }
    }

    advance_() {
        _cursor = _cursor + 1
        return _input[_cursor - 1]
    }

    isAlpha_(char) {
        if (false) {
            var pt = char.codePoints[0]

            return ((pt >= "a".codePoints[0] && pt <= "z".codePoints[0]) ||
                    (pt >= "A".codePoints[0] && pt <= "Z".codePoints[0]))
        } else {
            var byte = char.bytes[0]

            return ((byte >= 97 && byte <= 122) ||
                    (byte >= 65 && byte <= 90))
        }
    }

    isAtEnd_() {
        if (false) {
            return _cursor >= _input.count
        } else {
            return _cursor >= _input.byteCount_
        }
    }

    peek_() {
        if (isAtEnd_()) {
            return "\0"
        } else {
            return _input[_cursor]
        }
    }

    addToken_(type) { addToken_(type, null) }
    addToken_(type, value) { _tokens.add(Token.new(type, value, _cursor)) }

    scanningError_() {
        var value = _input[_start..._cursor]
        var position = Helper.getPositionForIndex(_input, _start)

        Fiber.abort("Invalid JSON: Unexpected \"%(value)\" at line %(position["line"]), column %(position["column"])")
    }
}

/* Deterministic JSON creation for testing.
 */
class JSONRandomGenerator {
    /*
     * `seed` - number used to setup pseudorandom number generation state.
     *
     * `depth` - current recursion level of the JSON tree. Used to halt
     *           structural branching (objects/arrays) and force primitive
     *           leaf nodes once maximum depth is reached.
     *
     * `breadth` - maximum items in objects/arrays. 20 is a good default.
     *
     * `probability` - chance to continue nesting if under maxDepth.
     *                 0.8 is a good default.
     */
    construct new(seed, depth, breadth, probability) {
        _rng = Random.new(seed)
        _maxDepth = depth
        _maxBreadth = breadth
        _nestingProbability = probability
    }

    prng { _rng }
    prng=(generator) { _rng = generator }

    maxDepth { _maxDepth }
    maxDepth=(depth) { _maxDepth = depth }

    maxBreadth { _maxBreadth }
    maxBreadth=(breadth) { _maxBreadth = breadth }

    nestingProbability { _nestingProbability }
    nestingProbability=(probability) { _nestingProbability = probability }

    generate() {
        return stringify_(randomBool_ ? generateObject_(0) : generateArray_(0))
    }

    randomBool_ {
        return _rng.float() > 0.5
    }

    generate_(depth) {
        if (depth >= _maxDepth) return generatePrimitive_()

        if (_rng.float() < _nestingProbability) {
            return randomBool_ ? generateArray_(depth) : generateObject_(depth)
        } else {
            return generatePrimitive_()
        }
    }

    generatePrimitive_() {
        var type = _rng.int(4)

        if (type == 0) return null
        if (type == 1) return randomBool_
        if (type == 2) return generateNumber_()
        if (type == 3) return generateString_()
    }

    generateNumber_() {
        /*
         * Randomly create a real or whole number.
         */
        var num = randomBool_ ? (_rng.float() * 1000) : _rng.int(0, 1000)
        return randomBool_ ? num : -num
    }

    generateString_() {
        var chars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789"

        /* TODO: Make min and max string size configurable.
         */
        var len = _rng.int(3, 12)
        var result = ""

        for (i in 1..len) {
            result = result + chars[_rng.int(chars.count)]
        }

        return result
    }

    generateArray_(depth) {
        var len = _rng.int(1, _maxBreadth + 1)
        var arr = []

        for (i in 0...len) {
            arr.add(generate_(depth + 1))
        }

        return arr
    }

    generateObject_(depth) {
        var len = _rng.int(1, _maxBreadth + 1)
        var obj = {}

        for (i in 0...len) {
            obj[generateString_()] = generate_(depth + 1)
        }

        return obj
    }

    stringify_(value) {
        if (value == null) {
            return "null"
        }

        if (value is Bool) {
            return value ? "true" : "false"
        }

        if (value is Num) {
            return value.toString
        }

        if (value is String) {
            return "\"" + value + "\""
        }

        if (value is List) {
            return "[" + value.map {|v| stringify_(v) }.toList.join(",") + "]"
        }

        if (value is Map) {
            var items = []

            for (key in value.keys) {
                items.add("\"" + key + "\":" + stringify_(value[key]))
            }

            return "{" + items.join(",") + "}"
        }
    }
}

class Token {
    static LeftBracket { "LEFT_BRACKET" }
    static RightBracket { "RIGHT_BRACKET" }
    static LeftBrace { "LEFT_BRACE" }
    static RightBrace { "RIGHT_BRACE" }
    static Colon { "COLON" }
    static Comma { "COMMA" }
    static String { "STRING" }
    static Number { "NUMBER" }
    static Bool { "BOOL" }
    static Null { "NULL"}
    static End { "EOF"}

    construct new(type, value, index) {
        _type = type
        _value = value
        _index = index
    }

    toString {
        return (_value != null) ? (_type + " " + _value.toString) : _type
    }

    type { _type }
    value { _value }
    index { _index }
}

class Helper {
    /*
     * TODO: Move to StringUtil. Note that we rely on our patch to
     * Num.fromString to accept single A-F hexadecimal characters.
     */
    static fromHex(str) {
        var power = 0
        var result = 0

        for (char in StringUtil.reverse(str)) {
            var num = Num.fromString(char)

            if (num == null) {
                return null
            }

            result = result + (num * NumUtil.exponent(16, power))
            power = power + 1
        }

        return result
    }

    static getPositionForIndex(text, index) {
        var line = 1
        var column = 1
        var i = 0

        while (i < index && i < text.count) {
            if (text[i] == "\n") {
                line = line + 1
                column = 1
            } else {
                column = column + 1
            }

            i = i + 1
        }

        return {
            "line": line,
            "column": column
        }
    }
}

/* ===== [ testing ] ======================================================== */

var main = Fn.new {
    if (true) {
        /* Basic datatypes.
         */
        System.print(JSON.stringify(JSON.parse(JSON.stringify(JSON.parse([
            "{\n",
            "    \"id\": 1024,\n",
            "    \"username\": \"johndoe_99\",\n",
            "    \"is_active\": true,\n",
            "    \"account_balance\": 1542.50,\n",
            "    \"roles\": [\"user\", \"moderator\"],\n",
            "    \"middle_name\": null,\n",
            "    \"metadata\": {\n",
            "        \"created_at\": \"2026-03-15T09:30:00Z\",\n",
            "        \"login_attempts\": 0\n",
            "    },\n",
            "    \"favorite_epsilon\": 0.0000001\n",
            "}\n",
        ].join("\n"))))))

        /* Deep nesting.
         */
        System.print(JSON.stringify(JSON.parse(JSON.stringify(JSON.parse([
            "{\n",
            "    \"company\": \"TechCorp\",\n",
            "    \"departments\": [\n",
            "        {\n",
            "            \"name\": \"Engineering\",\n",
            "            \"leads\": [\"Alice\", \"Bob\"],\n",
            "            \"projects\": [\n",
            "                {\n",
            "                    \"title\": \"Alpha\",\n",
            "                    \"budget\": 50000,\n",
            "                    \"status\": \"completed\"\n",
            "                },\n",
            "                {\n",
            "                    \"title\": \"Beta\",\n",
            "                    \"budget\": 125000,\n",
            "                    \"status\": \"in-progress\"\n",
            "                }\n",
            "            ]\n",
            "        },\n",
            "        {\n",
            "            \"name\": \"Design\",\n",
            "            \"leads\": [\"Charlie\"],\n",
            "            \"projects\": []\n",
            "        }\n",
            "    ]\n",
            "}\n",
        ].join("\n"))))))

        /* Stress test.
         */
        System.print(JSON.stringify(JSON.parse(JSON.stringify(JSON.parse([
            "{\n",
            "    \"empty_object\": {},\n",
            "    \"empty_array\": [],\n",
            "    \"empty_string\": \"\",\n",
            "    \"special_characters\": \"Café, 100\% ★ Unicode Test: 🦙, \\t Newline: \\n\",\n",
            "    \"escaped_quotes\": \"\\\"Hello World\\\"\",\n",
            "    \"extreme_numbers\": {\n",
            "        \"max_safe_integer\": 9007199254740991,\n",
            "        \"scientific_notation\": 6.022e23,\n",
            "        \"negative_float\": -0.0000001\n",
            "    }\n",
            "}\n",
        ].join("\n"))))))
    }

    if (true) {
        var start = System.clock
        var json

        if (false) {
            json = JSON.generate(12345, 3, 200, 0.9)
        } else {
            json = JSON.generate(12345, 3, 100)
        }

        System.print("generated %((json.count / (1024 * 1024)).toString[0...4]) MB of JSON in %(System.clock - start) seconds")

        for (i in 0...10) {
            start = System.clock
            JSON.stringify(JSON.parse(json))
            System.print("%(i) %(System.clock - start)")
        }
    }
}
