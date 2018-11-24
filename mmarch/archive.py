import logging
import os

logger = logging.getLogger('archive')

class Directory (object):
	def __init__(self, name):
		self.name = name
		self.files = set()

	def add(self, file):
		self.files.add(file)

class Archive (object):
	def __init__(self, args):
		self.args = args
		self.files = {}

	def add_dir(self, dir):
		for dirpath, dirnames, filenames in os.walk(dir, followlinks = self.args.follow_links):
			dirpath = os.path.relpath(dirpath, dir)
			if dirpath == '.':
				dirpath = ''

			for filename in filenames:
				src = os.path.join(dirpath, filename)
				logger.debug('filename: %s', src)
				f = self.files.setdefault(dirpath, Directory(dirpath))
				f.add(filename)
