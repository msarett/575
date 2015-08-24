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

def finds_match(str, suffixes):
    for suffix in suffixes:
        if str.endswith(suffix):
            return True
    return False

def scrape_flat(path, matches):
    result = []
    try:
        files = os.listdir(path)
    except:
        return result

    for f in files:
        if finds_match(f, matches):
            result.append(f)
    return result

def setup_files(path):
    suffix = [".h", ".cpp"]

    list = scrape_flat(".", suffix)
    for f in list:
       os.remove(f)

    list = scrape_flat(path, suffix)
    for f in list:
       shutil.copy(os.path.join(path, f), ".")

    return 0

def run_one(path):
    cmds = [ ["make", "clean"], ["make"], ["./bench"], ["./test"], ["./image", "-r", "expected/", "--tolerance", "3"] ]
    if 0 != setup_files(path):
        return -1
    for array in cmds:
        if 0 != spcall(array):
            doprint("Error calling " + array[0])
            return -1
    return 0


def main(argv=None):
    if argv is None:
        argv = sys.argv
    if "--help" == argv[1]:
        print("args: path-to-student-dirs pa#(e.g. 2) <optional: student-dir>")
        return 0
    if len(argv) < 3:
        print("error: need path-to-students and subdir-name args")
        return -1
    if len(argv) > 3:
        students = argv[3:]
    else:
        students = ["aaronmd", "corallo", "dptheodo", "klieth", "nbusby", "reynoldn", "swaldon",
                    "ajpark", "dierk", "dst3708", "fcli", "jskylstr", "pavel", "phsiao", "slove13" ]

    pa_name = argv[2]

    for stud in students:
        path = os.path.join(argv[1], stud, pa_name)
        doprint("Testing " + stud + " from " + path)
        if 0 != run_one(path):
            doprint("error")
        doprint("")    
    return 0

if __name__ == "__main__":
    sys.exit(main())

