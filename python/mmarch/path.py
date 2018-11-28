def create_pool(words):
    pool = bytearray()
    loc = {}
    words = sorted(map(lambda x: x.encode('utf-8'), words), key=lambda x: len(x), reverse = True)
    for word in words:
        pos = pool.find(word)
        if pos < 0:
            pos = len(pool)
            pool += word
        loc[word] = pos
    return pool, loc
