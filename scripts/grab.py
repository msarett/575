import sys
import getopt
import string
import shutil
import os

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

def dump_files(root, student, assignment, files):
    for f in files:
        src_file = os.path.join(root, student, assignment, f)
        print src_file

def copy_to_txt(root, student, assignment, files):
    for f in files:
        src_file = os.path.join(root, student, assignment, f)
        new_f = student + "." + assignment + "." + f + ".txt"
        shutil.copy(src_file, new_f)

#
# python this_file.py students_dir assignment_eg_pa4
# 
def main(argv=None):
    if argv is None:
        argv = sys.argv
    students_dir = argv[1]
    assignment = argv[2]

    proc = dump_files
    if len(argv) > 3:
        if argv[3] == "--txt":
            proc = copy_to_txt

    students = get_students(students_dir)
    for stud in students:
        stud_dir = os.path.join(students_dir, stud)
        files = scrape_flat(os.path.join(stud_dir, assignment), [".cpp", ".h"])
        if len(files) > 0:
            proc(students_dir, stud, assignment, files)

if __name__ == "__main__":
    sys.exit(main())

