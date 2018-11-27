from struct import Struct

class Format():
    MAGIC       = 0x4d415243 #MARC
    VERSION     = 1

    TYPE_DIR    = 1
    TYPE_FILE   = 2

    def __init__(self, options):
        self.BE = options.big_endian
        prefix = '>' if self.BE else '<'
        self._header = Struct(prefix + 'IIIII') #magic, version, page_size, total index size, dir count (first dir_count descriptors are directories)
        self._metadata_header = Struct(prefix + 'II') #record count, field count
        self._metadata = Struct(prefix + 'QIII') #type, offset, size, name offset, name size
        self._map_header = Struct(prefix + 'II') #hash function id, bucket count

    @property
    def header_size(self):
        return self._header.size

    def write_header(self, stream, *args):
        stream.write(self._header.pack(Format.MAGIC, Format.VERSION, *args))

    @property
    def metadata_header_size(self):
        return self._metadata_header.size

    def write_metadata_header(self, stream, total):
        stream.write(self._metadata_header.pack(total, self.metadata_size // 4))

    @property
    def metadata_size(self):
        return self._metadata.size

    def write_metadata(self, stream, *args):
        stream.write(self._metadata.pack(*args))

    @property
    def map_header_size(self):
        return self._map_header.size

    def write_map_header(self, stream, *args):
        stream.write(self._map_header.pack(*args))
