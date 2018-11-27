import logging
import struct

FNV_32_PRIME = 0x01000193
MASK32 = (1 << 32) - 1

FNV1        = 0
FNV1A       = 1
PYTHON      = 2
# XX          = 3

DEFAULT     = FNV1A

def _add32(x, y):
    return (x + y) & MASK32

def _mul32(x, y):
    return (x * y) & MASK32

def _rotl32(x, r):
    return ((x << r) | (x >> (32 - r))) & MASK32


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

def python(text):
    value = text[0] << 7 if text else 0
    for c in text:
        value = _mul32(value, 1000003) ^ c
    value ^= len(text)
    return value

LE32 = struct.Struct('<I')

def xxhash(text, seed = 0):

    def le32(offset):
        v, = LE32.unpack(text[offset: offset + 4])
        return v

    PRIME32_1   = 2654435761
    PRIME32_2   = 2246822519
    PRIME32_3   = 3266489917
    PRIME32_4   =  668265263
    PRIME32_5   =  374761393

    n = len(text)
    p = 0
    if n > 16:
        limit = n - 16
        v1, v2, v3, v4 = seed + PRIME32_1 + PRIME32_2, seed + PRIME32_2, seed, (seed - PRIME32_1 + MASK32 + 1) & MASK32
        while p < limit:
            v1 += _mul32(le32(p), PRIME32_2); v1 = _rotl32(v1, 13); v1 = _mul32(v1, PRIME32_1); p += 4
            v2 += _mul32(le32(p), PRIME32_2); v2 = _rotl32(v2, 13); v2 = _mul32(v2, PRIME32_1); p += 4
            v3 += _mul32(le32(p), PRIME32_2); v3 = _rotl32(v3, 13); v3 = _mul32(v3, PRIME32_1); p += 4
            v4 += _mul32(le32(p), PRIME32_2); v4 = _rotl32(v4, 13); v4 = _mul32(v4, PRIME32_1); p += 4
        value = _rotl32(v1, 1) + _rotl32(v2, 7) + _rotl32(v3, 12) + _rotl32(v4, 18)
    else:
        value = seed + PRIME32_5

    value = _add32(value, n)

    limit = n - 4
    while p < limit:
        value = _add32(value, _mul32(le32(p), PRIME32_3))
        value = _rotl32(value, 17) * PRIME32_4
        p += 4

    while p < n:
        value += _mul32(text[p], PRIME32_5)
        value = _rotl32(value, 11) * PRIME32_1
        p += 1

    value ^= (value >> 15)
    value = _mul32(value, PRIME32_2)
    value ^= (value >> 13)
    value = _mul32(value, PRIME32_3)
    value ^= (value >> 16)

    return value


def get(index):
    _func = {
        FNV1: fnv1,
        FNV1A: fnv1a,
        PYTHON: python,
        # XX: xxhash
    }
    return _func[index]

logger = logging.getLogger('hash-set')

class HashMap(object):
    PRIMES = [
		11, 23, 53, 97, 193, 389,
		769, 1543, 3079, 6151, 12289,
		24593, 49157, 98317, 196613, 393241,
		786433, 1572869, 3145739, 6291469, 12582917,
		25165843, 50331653, 100663319, 201326611, 402653189,
		805306457, 1610612741
    ]

    def __init__(self, func = DEFAULT, load_factor = 1.0):
        self.__data = {}
        self.func = func
        self.__func = get(func)
        self.__buckets = []
        self.__step = 0
        self.__size = 0
        self.load_factor = load_factor

    def __add(self, item):
        name = item[0]
        hash = self.__func(name)
        index = hash % len(self.__buckets)
        logger.debug("hash(%s) -> %08x, bucket: %d", name, hash, index)
        bucket = self.__buckets[index]
        if len(bucket) > 0:
            logger.debug('collision detected in bucket %d: %s', index, bucket)
        bucket.append(item)

    def __rehash(self):
        if self.__step >= len(HashMap.PRIMES):
            return

        if len(self.__buckets) > 0 and ((self.__size + 1) / len(self.__buckets)) < self.load_factor:
            return

        n = HashMap.PRIMES[self.__step]
        self.__step += 1
        logger.debug('increasing bucket size from %d to %d', len(self.__buckets), n)

        buckets, self.__buckets = self.__buckets, [[] for _ in range(n)]
        logger.debug("%s NEW BUCKS %s", buckets, self.__buckets)
        for bucket in buckets:
            for item in bucket:
                self.__add(item)


    def add(self, name, value):
        #logger.debug('adding entry %s of %s', name, value)
        self.__rehash()
        self.__add((name, value))
        self.__size += 1

    def __len__(self):
        return self.__size
