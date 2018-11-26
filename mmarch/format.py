class Format():
    def __init__(self, options):
        self.BE = options.big_endian
        print(options)

    @property
    def _prefix(self):
        return '>' if self.BE else '<'

    @property
    def header(self):
        return self._prefix + 'I' #magic

class Record():
    def __init__(self, options):
        self.options = options
        self.format = Format(options)

class Header(object):
    def __init__(self, options):
        super()
