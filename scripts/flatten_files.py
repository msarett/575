import sys
import getopt
import string
import shutil

class Usage(Exception):
    def __init__(self, msg):
        self.msg = msg

def build_flat_name(path):
    array = string.split(path, "/")
    if array[0] == ".":
        array.pop(0)
    str = ""
    for s in array:
        str += s
        str += "."
    str += "txt"
    return str

#
# find  . -name GContext2.cpp | xargs python flatten_files.py
#
def main(argv=None):
    if argv is None:
        argv = sys.argv
    list = argv
    list.pop(0)
    for i in list:
        newPath = build_flat_name(i)
        print "copy " + i + " to " + newPath
#        shutil.copyfile(i, newPath)

if __name__ == "__main__":
    sys.exit(main())

