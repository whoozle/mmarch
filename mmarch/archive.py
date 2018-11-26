import logging
import os
import mmarch.hash as hashfunc
import mmarch.format as format

logger = logging.getLogger('archive')

class HashMap(object):
	def __init__(self, func):
		self.__data = {}
		self.__func = func

	def add(self, name, value):
		bname = name.encode('utf-8')
		self.__data[bname] = value

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
		self.globals = HashMap(hashfunc.get(hashfunc.FNV1))

	def add_dir(self, src_dir):
		for dirpath, dirnames, filenames in os.walk(src_dir, followlinks = self.options.follow_links, topdown = False):
			rel_dirpath = os.path.relpath(dirpath, src_dir)
			if rel_dirpath == '.':
				rel_dirpath = ''

			try:
				dir = self.dirs.setdefault(rel_dirpath, Directory(dirpath))
			except Exception as ex:
				logger.error("stat failed: %s", ex)
				continue

			for filename in filenames:
				rel_path = os.path.join(rel_dirpath, filename)
				abs_path = os.path.join(src_dir, dirpath, filename)
				logger.debug('filename: %s, local name: %s', abs_path, rel_path)

				index = len(self.files) + 1
				try:
					file = File(abs_path, rel_path, filename, index, self.offset)
					self.files.append(file)
					dir.add(file)
					logger.debug("added file %s at offset %s", file, hex(self.offset))
					self.offset = (self.offset + file.size + self.page_size - 1) // self.page_size * self.page_size
				except Exception as ex:
					logger.error("stat failed: %s", ex)
					continue

	def write(self, stream):
		rec = format.Header(self.options)
		rec.write(stream)
		fs = format.FileStorage(self.options)
		fs.write(stream)
		rec = format.DirectoryStructure(fs, self.options)
		rec.write(stream)
		fs.writeContent(stream)
