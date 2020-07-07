# OCI Container Manager (Dobby) Thunder Plugin

This Thunder plugin allows for control of OCI containers using the Dobby hypervisor.

It interfaces with Dobby over the existing dbus API

# APIs
## listContainers
List all running OCI containers Dobby knows about
### Parameters
None
### Result
```json
{
   "jsonrpc":"2.0",
   "id":3,
   "result":{
      "containers":[
         {
            "Descriptor":91,
            "Id":"com.bskyb.epgui"
         }
      ],
      "success":true
   }
}
```

## getContainerState
Get the state of a currently running container. Possible states include:
* Invalid
* Starting
* Running
* Stopped
* Paused

### Params
| Name        | Type   | Description                             |
| ----------- | ------ | --------------------------------------- |
| containerId | string | ID of the container to get the state of |

### Result
```json
{
   "jsonrpc":"2.0",
   "id":3,
   "result":{
      "containerId":"com.bskyb.epgui",
      "state":"Running",
      "success":true
   }
}
```

---

## getContainerInfo
Gets information about a running container such as CPU, memory and GPU uage (GPU not supported on Xi6)

### Params
| Name        | Type   | Description                                  |
| ----------- | ------ | -------------------------------------------- |
| containerId | string | ID of the container to get information about |

### Result
```json
{
   "jsonrpc":"2.0",
   "id":3,
   "result":{
      "info":{
         "cpu":{
            "usage":{
               "percpu":[
                  83134464184,
                  95791749503,
                  31057322312,
                  14041572680
               ],
               "total":224025108679
            }
         },
         "id":"com.bskyb.epgui",
         "memory":{
            "user":{
               "failcnt":0,
               "limit":230686720,
               "max":38555648,
               "usage":28655616
            }
         },
         "pids":[
            7644,
            7736
         ],
         "state":"running",
         "timestamp":6849968158125
      },
      "success":true
   }
}
```
---
## startContainer

Starts a new container from an existing OCI bundle.

### Params
| Name        | Type   | Description                                                                            |
| ----------- | ------ | -------------------------------------------------------------------------------------- |
| containerId | string | ID for the new container                                                               |
| bundlePath  | string | Path to the OCI bundle containing the rootfs and config to use to create the container |
| command     | string | Custom command to run inside the container, overriding the command in the container config |
---

### Response
```json
{
   "jsonrpc":"2.0",
   "id":3,
   "result":{
      "descriptor":257,
      "success":true
   }
}
```

## startContainerFromDobbySpec

Starts a new container from a legacy Dobby json specification

### Params
| Name        | Type   | Description                                                                            |
| ----------- | ------ | -------------------------------------------------------------------------------------- |
| containerId | string | ID for the new container                                                               |
| dobbySpec  | object | Dobby specification to use for the container |
| command     | string | Custom command to run inside the container, overriding the command in the Dobby spec |

### Response
```json
{
   "jsonrpc":"2.0",
   "id":3,
   "result":{
      "descriptor":257,
      "success":true
   }
}
```
---
## stopContainer
Stop a currently running container

### Params
| Name        | Type    | Description                                                                                           |
| ----------- | ------- | ----------------------------------------------------------------------------------------------------- |
| containerId | string  | ID of the container to stop                                                                           |
| force       | boolean | [Optional - defaults to false] If true, force stop the container (SIGKILL). Otherwise SIGTERM is used |

### Response
```json
{
   "jsonrpc":"2.0",
   "id":3,
   "result":{
      "success":true
   }
}
```
---
## execCommamd

Execute a command inside a running container. The path to the executable must resolve within the container's namespace.

### Params
| Name        | Type   | Description                               |
| ----------- | ------ | ----------------------------------------- |
| containerId | string | ID of the container to run the command in |
| options     | string | Global options for crun's exec command    |
| command     | string | Command to execute                        |

### Response
```json
{
   "jsonrpc":"2.0",
   "id":3,
   "result":{
      "success":true
   }
}
```

---
## deleteContainer
**NOT YET IMPLEMENTED**

Delete a container bundle

### Params
**TODO:: Decide on parameters**

### Response
**NOT YET IMPLEMENTED**

---
## pauseContainer

Pause a currently running container

### Params
| Name        | Type   | Description                  |
| ----------- | ------ | ---------------------------- |
| containerId | string | ID of the container to pause |

### Response
```json
{
   "jsonrpc":"2.0",
   "id":3,
   "result":{
      "success":true
   }
}
```
---
## resumeContainer

Resume a previously paused container

### Params
| Name        | Type   | Description                                                       |
| ----------- | ------ | ----------------------------------------------------------------- |
| containerId | string | ID of the container to resume (must already be in a paused state) |

### Response
```json
{
   "jsonrpc":"2.0",
   "id":3,
   "result":{
      "success":true
   }
}
```
---
---
# Build
```
bitbake thunder-plugins
```

# Test
Sample curl commands to use to test the API

**ListContainers**
```
curl -X POST http://127.0.0.1:9998/Service/ -d '{"jsonrpc": "2.0", "id": 3, "method": "org.rdk.OCIContainer.1.listContainers"}'

```

**Get State**
```
curl -X POST http://127.0.0.1:9998/Service/ -d '{"jsonrpc": "2.0", "id": 3, "method": "org.rdk.OCIContainer.1.getContainerState", "params": {"containerId": "SleepTest"}}'
```

**Get Info**
```
curl -X POST http://127.0.0.1:9998/Service/ -d '{"jsonrpc": "2.0", "id": 3, "method": "org.rdk.OCIContainer.1.getContainerInfo", "params": {"containerId": "SleepTest"}}'
```

**Start (Bundle path)**
```
curl -X POST http://127.0.0.1:9998/Service/ -d '{
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
curl -X POST http://127.0.0.1:9998/Service/ -d '{
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
curl -X POST http://127.0.0.1:9998/Service/ -d '{"jsonrpc": "2.0", "id": 3, "method": "org.rdk.OCIContainer.1.stopContainer", "params": {"containerId": "SleepTest"}}'
```

**Pause Container**
```
curl -X POST http://127.0.0.1:9998/Service/ -d '{"jsonrpc": "2.0", "id": 3, "method": "org.rdk.OCIContainer.1.pauseContainer", "params": {"containerId": "SleepTest"}}'
```

**Resume Container**
```
curl -X POST http://127.0.0.1:9998/Service/ -d '{"jsonrpc": "2.0", "id": 3, "method": "org.rdk.OCIContainer.1.resumeContainer", "params": {"containerId": "SleepTest"}}'
```

**Execute Command**
```
curl -X POST http://127.0.0.1:9998/Service/ -d '{
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
