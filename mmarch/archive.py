import logging
import os
import mmarch.hash as hash
import mmarch.format as format
from mmarch.path import create_pool

logger = logging.getLogger('archive')

def align(size, alignment):
    return (size + alignment - 1) // alignment * alignment

class Directory (object):
    def __init__(self, name):
        self.name = name
        self.files = set()

    def add(self, file):
        self.files.add(file)

class File(object):
    __slots__ = ('abspath', 'relpath', 'name', 'index', 'offset', 'stat')
    def __init__(self, abspath, relpath, name, index, offset):
        self.abspath = abspath
        self.relpath = relpath
        self.name = name
        self.index = index
        self.offset = offset
        self.stat = os.stat(abspath)

    @property
    def size(self):
        return self.stat.st_size

    def __repr__(self):
        return "File(%s, size: %d, id: %d)" %(self.relpath, self.size, self.index)

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
        for dirpath, dirnames, filenames in os.walk(src_dir, followlinks = self.options.follow_links, topdown = False):
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

                index = len(self.files) + 1
                try:
                    file = File(abs_path, rel_path, filename, index, self.offset)
                    self.files.append(file)
                    dir.add(file)
                    logger.debug("added file %s at offset %s", file, hex(self.offset))
                    self.offset = align(self.offset + file.size, self.page_size)
                    self._total += 1
                except Exception as ex:
                    logger.error("adding file failed: %s", ex)
                    continue

    def _all(self):
        for dir in self.dirs.keys():
            yield dir
        for file in self.files:
            yield file.relpath

    def write(self, stream):
        filenames = [entry for entry in self._all()]
        string_pool, string_loc = create_pool(filenames)

        total = self._total
        string_pool_offset = self.format.header_size + self.format.metadata_size * total + self.format.metadata_header_size
        hash_data_offset = string_pool_offset + len(string_pool)
        file_data_offset = hash_data_offset + total * 4 #fixme

        self.format.write_header(stream, self.page_size, total, len(self.dirs))
        logger.debug('writing %u metadata records', total)
        self.format.write_metadata_header(stream, total)
        index = 1
        for dir in self.dirs.keys():
            name = dir.encode('utf8')
            self.format.write_metadata(stream, 0, 0, string_pool_offset + string_loc[name], len(name))
            self.global_names.add(name, index)
            index += 1
        for file in self.files:
            name = file.relpath.encode('utf8')
            self.format.write_metadata(stream, file_data_offset + file.offset, file.size, string_pool_offset + string_loc[name], len(name))
            self.global_names.add(name, index)
            index += 1
        stream.write(string_pool)
