import logging
import os
import mmarch.hash as hashfunc

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

class Archive (object):
	def __init__(self, options):
		self.options = options
		self.files = {}
		self.globals = HashMap(hashfunc.get(hashfunc.FNV1))

	def add_dir(self, dir):
		for dirpath, dirnames, filenames in os.walk(dir, followlinks = self.options.follow_links, topdown = False):
			dirpath = os.path.relpath(dirpath, dir)
			if dirpath == '.':
				dirpath = ''

			for filename in filenames:
				src = os.path.join(dirpath, filename)
				logger.debug('filename: %s', src)
				f = self.files.setdefault(dirpath, Directory(dirpath))
				f.add(filename)
