#!/usr/bin/python

import sys
import getopt
import string
import shutil
import subprocess
import os.path

class Usage(Exception):
    def __init__(self, msg):
        self.msg = msg

def doprint(str):
    subprocess.call(["echo", str])

def spcall(array):
    return subprocess.call(array)

def spcall_cwd(array, dir):
    doprint(dir)
    return subprocess.call(array, cwd=dir)

def main(argv=None):
    if argv is None:
        argv = sys.argv
    
    dirs = ["pa0", "pa1", "pa2", "pa3", "pa4", "xwin"]
    cmds = [ ["make", "clean"], ["make"], ["./image"], ["make", "clean"] ]
    for dir in dirs:
        for cmd in cmds:
            spcall_cwd(cmd, dir)
    return 0

if __name__ == "__main__":
    sys.exit(main())

