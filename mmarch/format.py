class Format(object):
    def __init__(self, options):
        self.BE = options.big_endian

    @property
    def _prefix(self):
        return '>' if self.BE else '<'

    @property
    def header(self):
        return self._prefix + 'I' #magic
