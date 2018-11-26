from struct import Struct

class Format():
    MAGIC = 0x4d415243 #MARC
    VERSION = 1

    def __init__(self, options):
        self.BE = options.big_endian
        prefix = '>' if self.BE else '<'
        self._header = Struct(prefix + 'III') #magic, version, total index size

    @property
    def header_size(self):
        return self._header.size

    def write_header(self, stream, size):
        stream.write(self._header.pack(Format.MAGIC, Format.VERSION, size))
