# This script generates the RDK services API reference 
# documentation. The script currently uses the JsonGenerator.py 
# script that is part of the rdkcentral/Thunder repository.

import shutil
import os
import glob
import time
import sys
import site

from git.repo.base import Repo
Repo.clone_from("https://github.com/rdkcentral/Thunder.git", r"../../Thunder")
site.addsitedir(r'../../Thunder/Tools')

# Fetching the JSON files for each plugin to generate md 
# files under the docs/api folder
def convert_json_to_md():
    print()
    print("*****   Generating md files under docs/api   *****")
    print()
    dirname = os.path.dirname(__file__)
    print("Directory path:", dirname)
    print()
    filename = os.path.join(dirname, r"../*/*Plugin.json")
    flist = glob.glob(os.path.join(filename))
    for file in flist:
        os.system(r"../../Thunder/Tools/JsonGenerator/JsonGenerator.py --docs "
                  r"{} -o ../docs/api --no-interfaces-section" .format(file))
    print()
    print("*****   Generated md files under docs/api    *****")

# Replacing the given strings in md files to fix the linking issues
def postprocess_md():
    print("#####    Postprocessing the md files    #####")
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
    print("#####    Postprocessing completed     #####")
def main():
    convert_json_to_md()
    print()
    print()
    postprocess_md()

if __name__ == "__main__":
    main()