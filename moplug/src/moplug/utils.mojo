def fnv1a32(text: String) -> UInt32:
    var result = UInt32(2166136261)
    var bytes = text.as_bytes()

    for i in range(text.byte_length()):
        result = result ^ UInt32(bytes[i])
        result = result * UInt32(16777619)

    return result
