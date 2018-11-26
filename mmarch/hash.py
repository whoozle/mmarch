FNV_32_PRIME = 0x01000193
MASK32 = (1 << 32) - 1

FNV1    = 0
FNV1A   = 1

def fnv1(text):
    value = 0x811c9dc5
    for c in text:
        value *= FNV_32_PRIME
        value ^= c
        value &= MASK32
    return value

def fnv1a(text):
    value = 0x811c9dc5
    for c in text:
        value ^= c
        value *= FNV_32_PRIME
        value &= MASK32
    return value

def get(index):
    _func = {
        FNV1: fnv1,
        FNV1A: fnv1a,
    }
    return _func[index]
