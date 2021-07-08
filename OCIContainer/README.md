# OCI Container Manager (Dobby) Thunder Plugin

This Thunder plugin allows for control of OCI containers using the Dobby hypervisor.

It interfaces with Dobby over the existing dbus API

# Build
```
bitbake thunder-plugins
```

# Test
Sample curl commands to use to test the API

**ListContainers**
```
curl -X POST http://127.0.0.1:9998/jsonrpc/ -d '{"jsonrpc": "2.0", "id": 3, "method": "org.rdk.OCIContainer.1.listContainers"}'

```

**Get State**
```
curl -X POST http://127.0.0.1:9998/jsonrpc/ -d '{"jsonrpc": "2.0", "id": 3, "method": "org.rdk.OCIContainer.1.getContainerState", "params": {"containerId": "SleepTest"}}'
```

**Get Info**
```
curl -X POST http://127.0.0.1:9998/jsonrpc/ -d '{"jsonrpc": "2.0", "id": 3, "method": "org.rdk.OCIContainer.1.getContainerInfo", "params": {"containerId": "SleepTest"}}'
```

**Start (Bundle path)**
```
curl -X POST http://127.0.0.1:9998/jsonrpc/ -d '{
   "jsonrpc":"2.0",
   "id":3,
   "method":"org.rdk.OCIContainer.1.startContainer",
   "params":{
      "containerId": "SleepTest",
      "bundlePath": "[-INSERT BUNDLE PATH-]"
   }
}'
```

**Start (Dobby Spec)**
```
curl -X POST http://127.0.0.1:9998/jsonrpc/ -d '{
   "jsonrpc":"2.0",
   "id":3,
   "method":"org.rdk.OCIContainer.1.startContainerFromDobbySpec",
   "params":{
      "containerId": "SleepTest",
      "dobbySpec":{
            "version": "1.0",
            "cwd": "/",
            "args": [
               "sleep",
               "60"
            ],
            "env": [

            ],
            "user": {
               "uid": 1000,
               "gid": 1000
            },
            "console": {
               "limit": 65536,
               "path": "/tmp/container.log"
            },
            "etc": {
               "group": [
                  "root:x:0:"
               ],
               "passwd": [
                  "root::0:0:root:/:/bin/false"
               ]
            },
            "memLimit": 41943040,
            "network": "nat",
            "mounts": [

            ]
      }
   }
}'
```

**Stop Container**
```
curl -X POST http://127.0.0.1:9998/jsonrpc/ -d '{"jsonrpc": "2.0", "id": 3, "method": "org.rdk.OCIContainer.1.stopContainer", "params": {"containerId": "SleepTest"}}'
```

**Pause Container**
```
curl -X POST http://127.0.0.1:9998/jsonrpc/ -d '{"jsonrpc": "2.0", "id": 3, "method": "org.rdk.OCIContainer.1.pauseContainer", "params": {"containerId": "SleepTest"}}'
```

**Resume Container**
```
curl -X POST http://127.0.0.1:9998/jsonrpc/ -d '{"jsonrpc": "2.0", "id": 3, "method": "org.rdk.OCIContainer.1.resumeContainer", "params": {"containerId": "SleepTest"}}'
```

**Execute Command**
```
curl -X POST http://127.0.0.1:9998/jsonrpc/ -d '{
   "jsonrpc": "2.0",
   "id": 3,
   "method": "org.rdk.OCIContainer.1.executeCommand",
   "params": {
      "containerId": "SleepTest",
      "options": "",
      "command": "ls"
   }
}'
```
