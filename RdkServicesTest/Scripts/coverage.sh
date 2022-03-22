#!/bin/sh

set -e

checkInstalled() {
  command -v "$1"
  case "$?" in
  0) true ;;
  *) false ;;
  esac
}

if ! checkInstalled "lcov"; then
  echo "lcov should be installed"
  exit 1
fi

lcov -o coverage.info -c -d ./thunder/build/rdkservices
genhtml -o coverage coverage.info
rm coverage.info

python -m webbrowser coverage/index.html

echo "==== DONE ===="

exit 0
