/* -----------------------------------------------------------------------------
--- Copyright (c) 2012-2026 Adam Schackart / "AJ Hackman", all rights reserved.
--- Distributed under the BSD license v2 (opensource.org/licenses/BSD-3-Clause)
----------------------------------------------------------------------------- */

class Base16 {
    static chars { "0123456789abcdef" }

    /* Encodes a string or a list of byte integers into a base16 string.
     */
    static encode(data) {
        if (data == null) {
            return ""
        }

        var bytes = (data is String) ? data.bytes.toList : data

        if (bytes.count == 0) {
            return ""
        }

        /* TODO: Append all strings into list and join them at the end.
         */
        var result = ""
        var alphabet = chars

        for (b in bytes) {
            result = result + alphabet[b >> 4]
            result = result + alphabet[b & 15]
        }

        return result
    }

    /* Decodes a base16 string into a list of byte integers.
     */
    static decodeBytes(data) {
        if (data == null || data == "") {
            return []
        }

        /* Strip common whitespace/newlines that might be present in formatted base16.
         */
        var cleanData = data.replace("\n", "").replace("\r", "").replace(" ", "")
        var bytes = cleanData.bytes.toList
        var len = bytes.count

        if (len % 2 != 0) {
            Fiber.abort("Invalid base16 input - odd length.")
        }

        var result = []
        var i = 0

        while (i < len) {
            var high = hexToInt_(bytes[i])
            var low = hexToInt_(bytes[i + 1])

            result.add((high << 4) | low)
            i = i + 2
        }

        return result
    }

    /* Decodes a base16 string back into a standard UTF-8 string.
     */
    static decode(data) {
        var decodedBytes = decodeBytes(data)
        var result = ""

        for (b in decodedBytes) {
            result = result + String.fromByte(b)
        }

        return result
    }

    /* Converts ASCII byte of a hex character to its 0-15 integer value.
     */
    static hexToInt_(byte) {
        /*
         * '0'-'9'
         */
        if (byte >= 48 && byte <= 57) {
            return byte - 48
        }

        /* 'a'-'f'
         */
        if (byte >= 97 && byte <= 102) {
            return byte - 87
        }

        /* 'A'-'F'
         */
        if (byte >= 65 && byte <= 70) {
            return byte - 55
        }

        Fiber.abort("Invalid base16 character: %(String.fromByte(byte))")
    }
}

var main = Fn.new {
    /*
     * OUTPUT: 48657820656e636f64696e6720696e205772656e
     */
    var encoded = Base16.encode("Hex encoding in Wren")
    System.print(encoded)

    /* OUTPUT: Hex encoding in Wren
     */
    var decodedText = Base16.decode(encoded)
    System.print(decodedText)

    /* OUTPUT: deadbeef
     */
    var byteList = [0xde, 0xad, 0xbe, 0xef]
    var encodedBytes = Base16.encode(byteList)
    System.print(encodedBytes)

    /* OUTPUT: [222, 173, 190, 239]
     */
    var decodedBytes = Base16.decodeBytes("DEadBEef")
    System.print(decodedBytes)
}
