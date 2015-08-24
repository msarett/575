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

def get_students(dir):
    for root, list, files in os.walk(dir):
        return list

#
# python this_file.py students_dir code_dir pa?
# 
def main(argv=None):
    if argv is None:
        argv = sys.argv

    students_dir = argv[1]
    if students_dir == "--help" or students_dir == "-h":
        doprint("Creates a set of directories (here), one for each student that has the corresponding")
        doprint("pa# directory. Each directory will have a copy of code/pa# and all of the .cpp/.h")
        doprint("files found in the student's pa# top-level")
        doprint("> grab dir_for_student_users dir_for_code pa#")
        return

    assignment = argv[3]
    code_dir = os.path.join(argv[2], assignment);

    students = get_students(students_dir)
    for stud in students:
        stud_src_dir = os.path.join(students_dir, stud, assignment)
        if os.path.exists(stud_src_dir):
            # copy the code files first, creating the stud directory at the same time.
            # code stores all of its .cpp and .h files in sub dirs (e.g. src or include)
            shutil.copytree(code_dir, stud)
            # now copy just the top-level cpp/h files from the student.
            files = scrape_flat(stud_src_dir, [".cpp", ".h"])
            for f in files:
                src_f = os.path.join(stud_src_dir, f)
                try:
                    shutil.copyfile(src_f, os.path.join(stud, f))
                except:
                    doprint(stud + ": couldn't copy file " + src_f)
        else:
            doprint(stud + ": does not have " + assignment)


if __name__ == "__main__":
    sys.exit(main())

