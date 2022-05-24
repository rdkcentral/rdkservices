#!/usr/bin/env python3
#Generating and postprocessing the md files
#Place the file under rdkservices/docs folder
import shutil
import os
import glob
import time
import sys
import site
import stat
import git
import subprocess
import urllib.request
from pathlib import Path, PureWindowsPath
from git.repo.base import Repo
from sys import platform

# Deleting the existing Thunder Repository from the desired location/ path
def del_rw(action, name, exc):
    os.chmod(name, stat.S_IWRITE)
    os.remove(name)
path = r"../../Thunder"
isdir = os.path.isdir(path)
if (isdir == True):
    print("##### A. Deleting Thunder Folder #####")
    shutil.rmtree(path, onerror=del_rw)
    print("   ##### Thunder Folder is Deleted #####")

# Checking the availability of GitPython in the machine
# Instructing the user to install GitPython if, not found in the machine
def git_install():
    if platform == "linux" or platform == "linux32" :
        os.system('sudo apt-get install python3-pip python-dev')
        os.system('pip install GitPython')

    elif platform == "win32":
        subprocess.run('python -m pip install --upgrade pip')
        p = subprocess.run('pip install GitPython')
        if (p.returncode == 0):
            print("######## GitPython is installed ########")
        else:
            print("######## Please install GitPython ########")

    elif platform == "mac32":
        subprocess.run('python -m pip install --upgrade pip')
        p = subprocess.run('brew install GitPython')
        if (p.returncode == 0):
            print("######## GitPython is installed ########")
        else:
            print("######## Please install GitPython ########")

    else:
        print("######### Invalid platform """"""")

#Fetching the json files from each plugin to generate md files
# under docs/api folder
def convert_json_to_md():
    print()
    print("***********   Generating md files under docs/api  ***********")
    print()
    dirname = os.path.dirname(__file__)
    print("Directory path:", dirname)
    print()
    filename = os.path.join(dirname, r"../*/*Plugin.json")
    flist = glob.glob(os.path.join(filename))
    jsongenpath = Path("../../Thunder/Tools/JsonGenerator/JsonGenerator.py")

    for file in flist:
        os.system(r"{} --docs "
                  r"{} -o ../docs/api --no-interfaces-section".format(jsongenpath, file))
    print()
    print("**********   Generated md files under docs/api   **********")

# Replacing the given strings in md files to fix the linking issues
def postprocess_md():
    print()
    flist = glob.glob(os.path.join(r"../docs/api/*Plugin.md"))

    # Loop and replace the given words
    for file in flist:
        # Open and Read the file
        with open(file, "r") as file_rd:
            rplce_file = file_rd.read()
            rplce_file_Org = rplce_file
            list_rplce = (" [<sup>method</sup>](#head.Methods)",
                          " [<sup>event</sup>](#head.Notifications)",
                          " [<sup>property</sup>](#Properties)",
                          "head.", "method.", "acronym.", "term.", "event.", "ref.", "property.")
            print("postprocessing filename:", file)

           # replace the words
            for word in list_rplce:
                rplce_file = rplce_file.replace(word, "")

            # Replace the file only if, the original file and
            # current file are different
            if rplce_file != rplce_file_Org:
               print("link fixing filename:", file)
               with open(file, "w") as file_wr:
                    file_wr.writelines(rplce_file)
    print()
    print("********************   Postprocessing completed   ********************")

def main():
     start = time.time()
     print("##### B. Checking GitPython #####")
     git_install()
     print("##### C. Checking out Thunder Folder #####")
     request_url = urllib.request.urlopen("https://github.com/rdkcentral/Thunder.git")
     Repo.clone_from("https://github.com/rdkcentral/Thunder.git", r"../../Thunder")
     print("##### D. Thunder Folder is Checked out #####")
     print("##### E. Generating md files #####")
     convert_json_to_md()
     print()
     print()
     print("##### F. Postprocessing md files #####")
     postprocess_md()
     print()
     print()
     end = time.time()
     print("The time taken to execute the above program is :", end - start)
     print()

if __name__ == "__main__":
    main()