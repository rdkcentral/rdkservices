# RdkServicesTest #

Write unit tests for your plugin and add them to RdkServicesTest.

Supply the CMake options to build your plugin in [build.sh](./Scripts/build.sh).
Read the [GoogleTest User’s Guide](https://google.github.io/googletest/).

## How to build ##

```shell script
cd RdkServicesTest
./Scripts/build.sh [-j<number>] [-D]

where:
    -j  number of threads (default maximum available threads)
    -D  debug mode (default Release)"
```

This builds Thunder, ThunderInterfaces, Rdkservices and installs them in thunder/install.

Make sure you have packages _python3 python3-pip libsqlite3-dev libcurl4-openssl-dev_.

## How to run ##

```shell script
cd RdkServicesTest
./Scripts/run.sh
```

This runs all tests, generates Valgrind report valgrind_log, and coverage info.

Make sure you have package _valgrind_.

## Coverage ##

```shell script
cd RdkServicesTest
./Scripts/coverage.sh
```

This generates a coverage report in HTML.

Make sure you have package _lcov_.
