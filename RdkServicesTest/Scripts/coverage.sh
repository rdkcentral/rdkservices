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

lcov \
  -o coverage.info \
  -c \
  -d ./thunder/build/rdkservices

lcov \
  -r coverage.info \
  '/usr/include/*' \
  '*/RdkServicesTest/thunder/build/rdkservices/_deps/*' \
  '*/RdkServicesTest/thunder/install/*' \
  -o filtered_coverage.info

genhtml \
  -o coverage \
  -t "rdkservices coverage" \
  filtered_coverage.info

rm coverage.info filtered_coverage.info

echo "==== DONE ===="

exit 0
