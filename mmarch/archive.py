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
	__slots__ = ('abspath', 'relpath', 'name', 'index', 'stat')
	def __init__(self, abspath, relpath, name, index):
		self.abspath = abspath
		self.relpath = relpath
		self.name = name
		self.index = index
		self.stat = os.stat(abspath)

	def __repr__(self):
		return "File(%s, %d)" %(self.relpath, self.index)

class Archive (object):
	def __init__(self, options):
		self.options = options
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
				logger.debug('filename: %s', rel_path)

				index = len(self.files) + 1
				try:
					file = File(os.path.join(src_dir, dirpath, filename), rel_path, filename, index)
					self.files.append(file)
					dir.add(file)
				except Exception as ex:
					logger.error("stat failed: %s", ex)

	def write(self, stream):
		rec = format.Header(self.options)
		rec.write(stream)
		rec = format.FileStorage(self.options)
		rec.write(stream)
		rec = format.DirectoryStructure(self.options)
		rec.write(stream)
