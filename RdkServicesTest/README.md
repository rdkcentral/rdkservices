# RdkServicesTest #

Write unit tests for your plugin and add them to RdkServicesTest.

Supply the CMake options to build your plugin in [build.sh](./Scripts/build.sh).
Read the [GoogleTest User’s Guide](https://google.github.io/googletest/).

## 1. Build ##

```shell script
cd RdkServicesTest
./Scripts/build.sh [-D] [-t]

options:
  -D                   Debug mode (default Release)
  -t                   Build tests only (default everything)
```

This builds Thunder, ThunderInterfaces, Rdkservices and installs them in thunder/install.

Make sure you have packages _python3 python3-pip libsqlite3-dev libcurl4-openssl-dev_.

## 2. Run ##

```shell script
cd RdkServicesTest
./Scripts/run.sh [-d]

options:
  -d                   Run with vgdb. 'target remote' args: "| vgdb"
```

This runs all tests, generates Valgrind report valgrind_log, and coverage info.

Make sure you have package _valgrind_.

## 3. Coverage ##

```shell script
cd RdkServicesTest
./Scripts/coverage.sh
python -m webbrowser coverage/index.html
```

This generates a coverage report in HTML.

Make sure you have package _lcov_.
