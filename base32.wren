/* -----------------------------------------------------------------------------
--- Copyright (c) 2012-2026 Adam Schackart / "AJ Hackman", all rights reserved.
--- Distributed under the BSD license v2 (opensource.org/licenses/BSD-3-Clause)
----------------------------------------------------------------------------- */

class Base32 {
    static chars { "ABCDEFGHIJKLMNOPQRSTUVWXYZ234567" }

    /* Encodes a string or a list of byte integers into a base32 string.
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
        var buffer = 0
        var bitsLeft = 0
        var alphabet = chars

        // Process bits dynamically.
        for (b in bytes) {
            buffer = (buffer << 8) | b
            bitsLeft = bitsLeft + 8

            while (bitsLeft >= 5) {
                bitsLeft = bitsLeft - 5
                var index = (buffer >> bitsLeft) & 31
                result = result + alphabet[index]
            }
        }

        // Process any remaining bits.
        if (bitsLeft > 0) {
            var index = (buffer << (5 - bitsLeft)) & 31
            result = result + alphabet[index]
        }

        // Add padding to make length a multiple of 8.
        while (result.count % 8 != 0) {
            result = result + "="
        }

        return result
    }

    /* Decodes a base32 string into a list of byte integers.
     */
    static decodeBytes(data) {
        if (data == null || data == "") return []

        // Strip padding and common whitespace.
        var cleanData = data.replace("\n", "").replace("\r", "").replace(" ", "").replace("=", "")
        var bytes = cleanData.bytes.toList

        var result = []
        var buffer = 0
        var bitsLeft = 0

        for (c in bytes) {
            var val = charToInt_(c)
            buffer = (buffer << 5) | val
            bitsLeft = bitsLeft + 5

            // Once we have a full byte (8 bits), extract and save it.
            if (bitsLeft >= 8) {
                bitsLeft = bitsLeft - 8
                result.add((buffer >> bitsLeft) & 255)
            }
        }

        return result
    }

    /* Decodes a base32 string back into a standard UTF-8 string.
     */
    static decode(data) {
        var decodedBytes = decodeBytes(data)
        var result = ""

        for (b in decodedBytes) {
            result = result + String.fromByte(b)
        }

        return result
    }

    /* Helper: Converts a base32 ASCII byte to its 0-31 integer value.
     */
    static charToInt_(byte) {
        /*
         * 'A'-'Z'
         */
        if (byte >= 65 && byte <= 90) {
            return byte - 65
        }

        /* 'a'-'z' (case-insensitive support)
         */
        if (byte >= 97 && byte <= 122) {
            return byte - 97
        }

        /* '2'-'7'
         */
        if (byte >= 50 && byte <= 55) {
            return byte - 24
        }

        Fiber.abort("Invalid base32 character: %(String.fromByte(byte))")
    }
}

var main = Fn.new {
    /*
     * OUTPUT: JBSWY3DPFQQHO33ONQ======
     */
    var encoded = Base32.encode("Hello, Wren!")
    System.print(encoded)

    /* OUTPUT: Hello, Wren!
     */
    var decodedText = Base32.decode(encoded)
    System.print(decodedText)

    /* OUTPUT: 6A23VQQ=
     */
    var byteList = [0xf0, 0x0d, 0xba, 0xbe]
    var encodedBytes = Base32.encode(byteList)
    System.print(encodedBytes)

    /* OUTPUT: [240, 13, 186, 190]
     */
    var decodedBytes = Base32.decodeBytes(encodedBytes)
    System.print(decodedBytes)
}
