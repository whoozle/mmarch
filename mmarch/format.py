from struct import Struct

class Format():
    MAGIC       = 0x4d415243 #MARC
    VERSION     = 1

    TYPE_DIR    = 1
    TYPE_FILE   = 2

    def __init__(self, options):
        self.BE = options.big_endian
        prefix = '>' if self.BE else '<'
        self._header = Struct(prefix + 'IIII') #magic, version, page_size, total index size
        self._metadata_header = Struct(prefix + 'II') #record count, field count
        self._metadata = Struct(prefix + 'IQIII') #type, offset, size, name offset, name size

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

    def write_dir(self, stream, *args):
        stream.write(self._metadata.pack(Format.TYPE_DIR, *args))

    def write_file(self, stream, *args):
        stream.write(self._metadata.pack(Format.TYPE_FILE, *args))
