from struct import Struct

class Format():
    MAGIC       = 0x4d415243 #MARC
    VERSION     = 1

    TYPE_DIR    = 1
    TYPE_FILE   = 2

    def __init__(self, options):
        self.BE = options.big_endian
        prefix = '>' if self.BE else '<'
        self._header = Struct(prefix + 'IIIIQIII') #magic, version, page_size, total index size, total file size
        self._metadata_header = Struct(prefix + 'III') #field count(version), record count, dir count (first dir_count descriptors are directories)
        self._metadata = Struct(prefix + 'QQII') #offset, size, full name offset, full name size
        self._map_header = Struct(prefix + 'II') #hash function id, bucket count
        self._map_entry = Struct(prefix + 'I') #name offset, name size, id
        self._table_entry = Struct(prefix + 'I') #offset to table
        self._readdir_entry = Struct(prefix + 'III') #file id, local name offset

    @property
    def header_size(self):
        return self._header.size

    def write_header(self, stream, *args):
        stream.write(self._header.pack(Format.MAGIC, Format.VERSION, *args))

    @property
    def metadata_header_size(self):
        return self._metadata_header.size

    def write_metadata_header(self, stream, *args):
        stream.write(self._metadata_header.pack(self.metadata_size // 4, *args))

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

    @property
    def map_entry_size(self):
        return self._map_entry.size

    def get_map_entry(self, *args):
        return self._map_entry.pack(*args)

    @property
    def readdir_entry_size(self):
        return self._readdir_entry.size

    def get_readdir_entry(self, *args):
        return self._readdir_entry.pack(*args)

    @property
    def table_entry_size(self):
        return self._table_entry.size

    def get_table_entry(self, *args):
        return self._table_entry.pack(*args)

    def write_indexed_table(self, stream, data, writer, base):
        r = bytearray()
        offsets = []
        base += (len(data) + 1) * self.table_entry_size
        for item in data:
            offsets.append(len(r) + base)
            r += writer(item)
        offsets.append(len(r) + base) #end

        table = bytearray()
        for offset in offsets:
            table += self.get_table_entry(offset)

        stream.write(table)
        stream.write(r)
