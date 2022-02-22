# RdkServicesTest #

Write unit tests for your plugin and add them to RdkServicesTest.

## How to build ##

```shell script
cd RdkServicesTest
./Scripts/build.sh
```

This builds Thunder, ThunderInterfaces, Rdkservices and installs them in thunder/install.
Make sure you have packages python3 python3-pip libglib2.0-dev libsqlite3-dev, curl libcurl4-openssl-dev

## How to run ##
```shell script
cd RdkServicesTest
./Scripts/run.sh
```

## How to make debug build and run test via gdb
```shell script
cd RdkServicesTest
./Scripts/build.sh -D && ./Scripts/run.sh -D
```

## Use specific number of threads
```shell script
cd RdkServicesTest
./Scripts/build.sh -j10
```

where 10 is number of threads