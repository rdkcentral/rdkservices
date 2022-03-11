# RdkServicesTest #

Write unit tests for your plugin and add them to RdkServicesTest.

## How to build ##

```shell script
cd RdkServicesTest
./Scripts/build.sh
```

This builds Thunder, ThunderInterfaces, Rdkservices and installs them in thunder/install.
Make sure you have packages python3 python3-pip libsqlite3-dev.

## How to run ##

```shell script
cd RdkServicesTest
./Scripts/run.sh
```

This runs all tests, generates valgrind report valgrind_log, and coverage info.
Make sure you have package valgrind.

## Coverage ##

```shell script
cd RdkServicesTest
./Scripts/coverage.sh
```

This generates coverage report in html.
Make sure you have package lcov.
