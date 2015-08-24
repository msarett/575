import sys
import getopt

class Usage(Exception):
    def __init__(self, msg):
        self.msg = msg

names_to_exclude = ['code', 'public_html']

def main(argv=None):
    if argv is None:
        argv = sys.argv
    list = argv
    list.pop(0)
    for i in list:
        if not (i in names_to_exclude):
            print i

if __name__ == "__main__":
    sys.exit(main())

