import logging
import os
import mmarch.hash as hash
import mmarch.format as format
from mmarch.path import create_pool

logger = logging.getLogger('archive')

def align(size, alignment):
    return (size + alignment - 1) // alignment * alignment

def pad(size, alignment):
    n = align(size, alignment) - size
    return bytearray([0]) * n

class Directory (object):
    def __init__(self, name):
        self.name = name
        self.files = []

    def add(self, file):
        self.files.append(file)

    def __repr__(self):
        return "%s" %self.files

class File(object):
    __slots__ = ('abspath', 'relpath', 'name', 'index', 'offset', 'stat')
    def __init__(self, abspath, relpath, name, offset):
        self.abspath = abspath
        self.relpath = relpath
        self.name = name
        self.index = None
        self.offset = offset
        self.stat = os.stat(abspath)

    @property
    def size(self):
        return self.stat.st_size

    def __repr__(self):
        return "File(%s, size: %d%s)" %(self.relpath, self.size, ", %d" %self.index if self.index else "")

class Archive (object):
    def __init__(self, options):
        self.options = options
        self.page_size = options.page_size
        self.offset = 0
        self.files = []
        self.dirs = {}
        self.global_names = hash.HashMap()
        self.format = format.Format(options)
        self._total = 0

    def add_dir(self, src_dir):
        logger.info("processing directory %s", src_dir)
        for dirpath, dirnames, filenames in os.walk(src_dir, followlinks = self.options.follow_links):
            rel_dirpath = os.path.relpath(dirpath, src_dir)
            if rel_dirpath == '.':
                rel_dirpath = ''

            try:
                dir = self.dirs.setdefault(rel_dirpath, Directory(dirpath))
                self._total += 1
            except Exception as ex:
                logger.error("adding directory failed: %s", ex)
                continue

            for filename in filenames:
                rel_path = os.path.normpath(os.path.join(rel_dirpath, filename))
                abs_path = os.path.normpath(os.path.join(dirpath, filename))
                logger.debug('filename: %s, local name: %s', abs_path, rel_path)

                try:
                    file = File(abs_path, rel_path, filename, self.offset)
                    self.files.append(file)
                    dir.add(file)
                    logger.debug("added file %s at offset %s", file, hex(self.offset))
                    self.offset = align(self.offset + file.size, self.page_size)
                    self._total += 1
                except Exception as ex:
                    logger.error("adding file failed: %s", ex)
                    continue
        logger.info("finished with directory %s", src_dir)

    def _all(self):
        for dir in self.dirs.keys():
            yield dir
        for file in self.files:
            yield file.relpath
        for file in self.files:
            yield file.name

    def write(self, stream):
        logger.info("building string pool and hash map...")
        filenames = [entry for entry in self._all()]
        string_pool, string_loc = create_pool(filenames)

        index = 0
        for dir in self.dirs.keys():
            name = dir.encode('utf8')
            self.global_names.add(name, index)
            index += 1
        for file in self.files:
            name = file.relpath.encode('utf8')
            file.index = index
            self.global_names.add(name, index)
            index += 1


        total = self._total
        assert index == total
        del index

        dirs_count = len(self.dirs)
        format = self.format
        hash_func_id, map_buckets = self.global_names.serialize()

        logger.info("writing %d file%s in %d director%s:", total, "s" if total > 1 else "", dirs_count, "ies" if dirs_count > 1 else "y")

        file_metadata_offset = format.header_size
        file_metadata_size = format.metadata_size * total + format.metadata_header_size
        logger.debug("file metadata offset = 0x%08x (%+d)", file_metadata_offset, file_metadata_size)

        map_offset = file_metadata_offset + file_metadata_size
        map_size = format.map_header_size + format.map_entry_size * total + format.table_entry_size * (len(map_buckets) + 1) # header all file ids + bucket table + 1 extra bucket end entry
        logger.debug("map data offset = %08x (%+d) 0x%08x", map_offset, map_size, map_offset + map_size)

        readdir_offset = map_offset + map_size
        readdir_total = sum(map(lambda x: len(x.files), self.dirs.values()))
        readdir_size = readdir_total * format.readdir_entry_size + format.table_entry_size * (dirs_count + 1)
        logger.debug("readdir data offset = 0x%08x (%+d) 0x%08x", readdir_offset, readdir_size, readdir_offset + readdir_size)

        string_pool_offset = readdir_offset + readdir_size
        logger.debug("string pool offset = 0x%08x (%+d) 0x%08x", string_pool_offset, len(string_pool), string_pool_offset + len(string_pool))

        file_data_offset = string_pool_offset + len(string_pool)
        file_data_offset_aligned = align(file_data_offset, self.page_size)
        logger.debug("file data offset = 0x%08x", file_data_offset_aligned)

        format.write_header(stream, self.page_size, file_data_offset_aligned, self.offset + file_data_offset_aligned, file_metadata_offset, map_offset, readdir_offset)
        format.write_metadata_header(stream, total, dirs_count)
        for dir in self.dirs.keys():
            name = dir.encode('utf8')
            format.write_metadata(stream, 0, 0, string_pool_offset + string_loc[name], len(name))
        for file in self.files:
            name = file.relpath.encode('utf8')
            format.write_metadata(stream, file_data_offset + file.offset, file.size, string_pool_offset + string_loc[name], len(name))

        format.write_map_header(stream, hash_func_id, len(map_buckets))

        def write_map_entry(entries):
            r = bytearray()
            for entry in entries:
                _, id, _ = entry
                r += format.get_map_entry(id)
            return r

        format.write_indexed_table(stream, map_buckets, write_map_entry, map_offset + format.map_header_size)

        def write_readdir_entry(entry):
            dirname, dir = entry
            r = bytearray()
            for file in dir.files:
                name = file.name.encode('utf-8')
                r += format.get_readdir_entry(file.index + dirs_count, string_pool_offset + string_loc[name], len(name))
            return r

        format.write_indexed_table(stream, self.dirs.items(), write_readdir_entry, readdir_offset)
        stream.write(string_pool)

        stream.write(pad(file_data_offset, self.page_size))

        for file in self.files:
            logger.debug("writing %s...", file.relpath)
            with open(file.abspath, 'rb') as f:
                BUFSIZE = 1024 * 1024
                while True:
                    buf = f.read(BUFSIZE)
                    stream.write(buf)
                    if len(buf) < BUFSIZE:
                        break
            stream.write(pad(file.size, self.page_size))
