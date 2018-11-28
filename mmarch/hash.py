import logging
import struct

FNV_32_PRIME = 0x01000193
MASK32 = (1 << 32) - 1

FNV1        = 0
FNV1A       = 1
PYTHON      = 2
R5A         = 3

DEFAULT     = R5A

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

def r5a(text):
    value = 0
    for c in text:
        value = (value + (c << 4) + (c >> 4)) & MASK32
        value = ((value + (value << 1) + (value << 3)) & MASK32) ^ (value >> 29) #peasant multiplication by 11
    return value

def python(text):
    value = text[0] << 7 if text else 0
    for c in text:
        value = _mul32(value, 1000003) ^ c
    value ^= len(text)
    return value

LE32 = struct.Struct('<I')

def get(index):
    _func = {
        FNV1: fnv1,
        FNV1A: fnv1a,
        PYTHON: python,
        R5A: r5a
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
        self.func = func
        self.__func = get(func)
        self.__buckets = []
        self.__step = 0
        self.__size = 0
        self.load_factor = load_factor

    def __add(self, item):
        name, _, hash = item
        index = hash % len(self.__buckets)
        # logger.debug("hash(%s) -> %08x, bucket: %d", name, hash, index)
        bucket = self.__buckets[index]
        # if len(bucket) > 0:
        #     logger.debug('collision detected in bucket %d: %s', index, bucket)
        bucket.append(item)

    def __rehash(self):
        if self.__step >= len(HashMap.PRIMES):
            return

        if len(self.__buckets) > 0 and ((self.__size + 1) / len(self.__buckets)) < self.load_factor:
            return

        n = HashMap.PRIMES[self.__step]
        self.__step += 1
        logger.debug('increasing buckets number from %d to %d', len(self.__buckets), n)

        buckets, self.__buckets = self.__buckets, [[] for _ in range(n)]
        for bucket in buckets:
            for item in bucket:
                self.__add(item)


    def add(self, name, value):
        #logger.debug('adding entry %s of %s', name, value)
        self.__rehash()
        self.__add((name, value, self.__func(name)))
        self.__size += 1

    def serialize(self):
        return (self.func, self.__buckets)

    def __len__(self):
        return self.__size
