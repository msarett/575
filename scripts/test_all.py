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

def clean_one():
    cmds = [ ["make", "clean"] ]
    for array in cmds:
        if 0 != spcall(array):
            doprint("Error calling " + array[0])
            return -1
    return 0

def run_one():
    cmds = [ ["make", "clean"], ["make"], ["./image"] ]
    for array in cmds:
        if 0 != spcall(array):
            doprint("Error calling " + array[0])
            return -1
    return 0

def get_students(dir):
    for root, list, files in os.walk(dir):
        return list

#
# python this_file.py
# 
def main(argv=None):
    if argv is None:
        argv = sys.argv

    be_silent = None
    proc = run_one
    if len(argv) == 2 and argv[1] == "--clean":
        proc = clean_one
        be_silent = True

    students = get_students(".")
    results = []
    for stud in students:
        if not be_silent:
            doprint(stud)
        os.chdir(stud)
        results.append(proc())
        os.chdir("..");

    if not be_silent:
        doprint("")
        doprint("Results")
        for i in range(len(students)):
            doprint(students[i] + ": " + str(results[i]))

if __name__ == "__main__":
    sys.exit(main())

