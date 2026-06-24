/* -----------------------------------------------------------------------------
--- Copyright (c) 2012-2026 Adam Schackart / "AJ Hackman", all rights reserved.
--- Distributed under the BSD license v2 (opensource.org/licenses/BSD-3-Clause)
----------------------------------------------------------------------------- */

class Base64 {
    static chars { "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/" }

    /* Encodes a string or a list of byte integers into a base64 string.
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
        var i = 0
        var len = bytes.count
        var alphabet = chars

        while (i < len) {
            var b1 = bytes[i]
            var b2 = (i + 1 < len) ? bytes[i + 1] : 0
            var b3 = (i + 2 < len) ? bytes[i + 2] : 0

            var enc1 = b1 >> 2
            var enc2 = ((b1 &  3) << 4) | (b2 >> 4)
            var enc3 = ((b2 & 15) << 2) | (b3 >> 6)
            var enc4 = b3 & 63

            result = result + alphabet[enc1]
            result = result + alphabet[enc2]
            result = result + ((i + 1 < len) ? alphabet[enc3] : "=")
            result = result + ((i + 2 < len) ? alphabet[enc4] : "=")

            i = i + 3
        }

        return result
    }

    /* Decodes a base64 string into a list of byte integers.
     */
    static decodeBytes(data) {
        if (data == null || data == "") {
            return []
        }

        /* Strip common whitespace/newlines that might be present in formatted base64.
         */
        var cleanData = data.replace("\n", "").replace("\r", "").replace(" ", "")
        var bytes = cleanData.bytes.toList
        var len = bytes.count
        var i = 0

        var revLookup = {}
        var alphaBytes = chars.bytes.toList

        for (j in 0...64) {
            revLookup[alphaBytes[j]] = j
        }

        var result = []

        while (i < len) {
            var c1 = bytes[i + 0]
            var c2 = bytes[i + 1]
            var c3 = bytes[i + 2]
            var c4 = bytes[i + 3]

            // 61 is the ASCII character for '='.
            var enc1 = revLookup[c1]
            var enc2 = revLookup[c2]
            var enc3 = (c3 == 61) ? 0 : revLookup[c3]
            var enc4 = (c4 == 61) ? 0 : revLookup[c4]

            var b1 = (enc1 << 2) | (enc2 >> 4)
            var b2 = ((enc2 & 15) << 4) | (enc3 >> 2)
            var b3 = ((enc3 & 3) << 6) | enc4

            result.add(b1)
            if (c3 != 61) result.add(b2)
            if (c4 != 61) result.add(b3)

            i = i + 4
        }

        return result
    }

    /* Decodes a base64 string back into a standard UTF-8 string.
     */
    static decode(data) {
        var decodedBytes = decodeBytes(data)
        var result = ""

        for (b in decodedBytes) {
            result = result + String.fromByte(b)
        }

        return result
    }
}

var main = Fn.new {
    /*
     * OUTPUT: V3JlbiBpcyBhIHNtYWxsLCBmYXN0LCBjbGFzcy1iYXNlZCBjb25jdXJyZW50IHNjcmlwdGluZyBsYW5ndWFnZS4=
     */
    var encoded = Base64.encode("Wren is a small, fast, class-based concurrent scripting language.")
    System.print(encoded)

    /* OUTPUT: Wren is a small, fast, class-based concurrent scripting language.
     */
    var decodedText = Base64.decode(encoded)
    System.print(decodedText)

    /* OUTPUT: 3q2+7w==
     */
    var byteList = [0xde, 0xad, 0xbe, 0xef]
    var encodedBytes = Base64.encode(byteList)
    System.print(encodedBytes)

    /* OUTPUT: [222, 173, 190, 239]
     */
    var decodedBytes = Base64.decodeBytes(encodedBytes)
    System.print(decodedBytes)
}
