#!/bin/sh
#
# This script is used to generate all API markdown files in a
# single pass. It calls the JsonGenerator.py script that is
# part of the rdkcentral/Thunder repository. Update the below 
# variables before running the script:
# 
#   * $THUNDER_REPO (The location where the thunder repository
#      is cloned relative to this script.)
#
#   * $RDKSERVICES_REPO (The location where the rdkservices
#     repository is cloned relative to the JsonGenerator.py
#     script)
# 

export THUNDER_REPO="../../../Thunder/Tools/JsonGenerator"
export RDKSERVICES_REPO="../../../rdkservices/"

cd $THUNDER_REPO
 
 command -v ./JsonGenerator.py >/dev/null 2>&1 || { 
     echo >&2 "JsonGenerator.py is not available. Aborting."; exit 1; 
     }
 
 echo "Generating Plugin markdown documentation..."
 pwd
 ./JsonGenerator.py --docs $RDKSERVICES_REPO/*/*Plugin.json  -o ../docs/api --no-interfaces-section --verbose $files

 echo "Generation Complete."