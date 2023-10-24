<!-- Generated automatically, DO NOT EDIT! -->
<a name="OCIContainer_Plugin"></a>
# OCIContainer Plugin

**Version: [1.0.4](https://github.com/rdkcentral/rdkservices/blob/main/OCIContainer/CHANGELOG.md)**

A org.rdk.OCIContainer plugin for Thunder framework.

### Table of Contents

- [Abbreviation, Acronyms and Terms](#Abbreviation,_Acronyms_and_Terms)
- [Description](#Description)
- [Configuration](#Configuration)
- [Methods](#Methods)
- [Notifications](#Notifications)

<a name="Abbreviation,_Acronyms_and_Terms"></a>
# Abbreviation, Acronyms and Terms

[[Refer to this link](userguide/aat.md)]

<a name="Description"></a>
# Description

The `OCIContainer` plugin allows for control of OCI containers using the Dobby hypervisor.

The plugin is designed to be loaded and executed within the Thunder framework. For more information about the framework refer to [[Thunder](#Thunder)].

<a name="Configuration"></a>
# Configuration

The table below lists configuration options of the plugin.

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| callsign | string | Plugin instance name (default: *org.rdk.OCIContainer*) |
| classname | string | Class name: *org.rdk.OCIContainer* |
| locator | string | Library name: *libWPEFrameworkOCIContainer.so* |
| autostart | boolean | Determines if the plugin shall be started automatically along with the framework |

<a name="Methods"></a>
# Methods

The following methods are provided by the org.rdk.OCIContainer plugin:

OCIContainer interface methods:

| Method | Description |
| :-------- | :-------- |
| [executeCommand](#executeCommand) | Executes a command inside a running container |
| [getContainerInfo](#getContainerInfo) | Gets information about a running container such as CPU, memory, and GPU usage (GPU not supported on Xi6) |
| [getContainerState](#getContainerState) | Gets the state of a currently running container |
| [listContainers](#listContainers) | Lists all running OCI containers Dobby knows about |
| [pauseContainer](#pauseContainer) | Pauses a currently running container |
| [resumeContainer](#resumeContainer) | Resumes a previously paused container |
| [startContainer](#startContainer) | Starts a new container from an existing OCI bundle |
| [startContainerFromDobbySpec](#startContainerFromDobbySpec) | Starts a new container from a legacy Dobby JSON specification |
| [stopContainer](#stopContainer) | Stops a currently running container |


<a name="executeCommand"></a>
## *executeCommand*

Executes a command inside a running container. The path to the executable must resolve within the container's namespace.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.containerId | string | The ID of a container as returned by `listContainers` |
| params?.options | string | <sup>*(optional)*</sup> Global options for crun `exec` command |
| params.command | string | Command to execute |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.OCIContainer.executeCommand",
    "params": {
        "containerId": "com.bskyb.epgui",
        "options": "--cwd=PATH",
        "command": "command"
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "success": true
    }
}
```

<a name="getContainerInfo"></a>
## *getContainerInfo*

Gets information about a running container such as CPU, memory, and GPU usage (GPU not supported on Xi6).

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.containerId | string | The ID of a container as returned by `listContainers` |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.info | object | Information about the specified container |
| result.info.cpu | object | CPU information |
| result.info.cpu.usage | object | The amount of CPU usage |
| result.info.cpu.usage.percpu | array | The usage for each CPU |
| result.info.cpu.usage.percpu[#] | integer |  |
| result.info.cpu.usage.total | integer | The combined usage for all CPUs |
| result.info.id | string | The ID of a container as returned by `listContainers` |
| result.info.memory | object | The amount of memory being used by the container |
| result.info.memory.user | object | User memory statistics |
| result.info.memory.user.failcnt | integer | The fail count; the number of times that the usage counter hit its limit |
| result.info.memory.user.limit | integer | The memory limit |
| result.info.memory.user.max | integer | The maximum amount of memory used |
| result.info.memory.user.usage | integer | The current memory being used |
| result.info.pids | array | A list of container process IDs |
| result.info.pids[#] | integer |  |
| result.info.state | string | The container state (must be one of the following: *Invalid*, *Starting*, *Running*, *Stopped*, *Paused*) |
| result.info.timestamp | integer | The timestamp for container information |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.OCIContainer.getContainerInfo",
    "params": {
        "containerId": "com.bskyb.epgui"
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "info": {
            "cpu": {
                "usage": {
                    "percpu": [
                        83134464184
                    ],
                    "total": 224025108679
                }
            },
            "id": "com.bskyb.epgui",
            "memory": {
                "user": {
                    "failcnt": 0,
                    "limit": 230686720,
                    "max": 38555648,
                    "usage": 28655616
                }
            },
            "pids": [
                7644
            ],
            "state": "Running",
            "timestamp": 6849968158125
        },
        "success": true
    }
}
```

<a name="getContainerState"></a>
## *getContainerState*

Gets the state of a currently running container.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.containerId | string | The ID of a container as returned by `listContainers` |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.containerId | string | The ID of a container as returned by `listContainers` |
| result.state | string | The container state (must be one of the following: *Invalid*, *Starting*, *Running*, *Stopped*, *Paused*) |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.OCIContainer.getContainerState",
    "params": {
        "containerId": "com.bskyb.epgui"
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "containerId": "com.bskyb.epgui",
        "state": "Running",
        "success": true
    }
}
```

<a name="listContainers"></a>
## *listContainers*

Lists all running OCI containers Dobby knows about.

### Events

No Events

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.containers | array | A list of running containers |
| result.containers[#] | object |  |
| result.containers[#].Descriptor | integer | The container descriptor |
| result.containers[#].Id | string | The container Id |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.OCIContainer.listContainers"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "containers": [
            {
                "Descriptor": 91,
                "Id": "com.bskyb.epgui"
            }
        ],
        "success": true
    }
}
```

<a name="pauseContainer"></a>
## *pauseContainer*

Pauses a currently running container.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.containerId | string | The ID of a container as returned by `listContainers` |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.OCIContainer.pauseContainer",
    "params": {
        "containerId": "com.bskyb.epgui"
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "success": true
    }
}
```

<a name="resumeContainer"></a>
## *resumeContainer*

Resumes a previously paused container.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.containerId | string | The ID of a container as returned by `listContainers` |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.OCIContainer.resumeContainer",
    "params": {
        "containerId": "com.bskyb.epgui"
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "success": true
    }
}
```

<a name="startContainer"></a>
## *startContainer*

Starts a new container from an existing OCI bundle.

### Events

| Event | Description |
| :-------- | :-------- |
| [onContainerStarted](#onContainerStarted) | Triggers when a new container starts running. |
### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.containerId | string | The ID of a container as returned by `listContainers` |
| params.bundlePath | string | Path to the OCI bundle containing the rootfs and config to use to create the container |
| params?.command | string | <sup>*(optional)*</sup> Command to execute |
| params?.westerosSocket | string | <sup>*(optional)*</sup> Path to a Westeros socket to mount inside the container |
| params?.envvar | array | <sup>*(optional)*</sup> A list of environment variables to add to the container |
| params?.envvar[#] | string | <sup>*(optional)*</sup>  |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.descriptor | integer | The container descriptor |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.OCIContainer.startContainer",
    "params": {
        "containerId": "com.bskyb.epgui",
        "bundlePath": "/containers/myBundle",
        "command": "command",
        "westerosSocket": "/usr/mySocket",
        "envvar": [
            "FOO=BAR"
        ]
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "descriptor": 91,
        "success": true
    }
}
```

<a name="startContainerFromDobbySpec"></a>
## *startContainerFromDobbySpec*

Starts a new container from a legacy Dobby JSON specification.

### Events

| Event | Description |
| :-------- | :-------- |
| [onContainerStarted](#onContainerStarted) | Triggers when a new container starts running. |
### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.containerId | string | The ID of a container as returned by `listContainers` |
| params.dobbySpec | string | Dobby specification to use for the container |
| params?.command | string | <sup>*(optional)*</sup> Command to execute |
| params?.westerosSocket | string | <sup>*(optional)*</sup> Path to a Westeros socket to mount inside the container |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.descriptor | integer | The container descriptor |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.OCIContainer.startContainerFromDobbySpec",
    "params": {
        "containerId": "com.bskyb.epgui",
        "dobbySpec": "/containers/dobbySpec",
        "command": "command",
        "westerosSocket": "/usr/mySocket"
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "descriptor": 91,
        "success": true
    }
}
```

<a name="stopContainer"></a>
## *stopContainer*

Stops a currently running container.

### Events

| Event | Description |
| :-------- | :-------- |
| [onContainerStopped](#onContainerStopped) | Triggers when the container stops running. |
### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.containerId | string | The ID of a container as returned by `listContainers` |
| params?.force | boolean | <sup>*(optional)*</sup> If `true`, force stop the container using the `SIGKILL` signal). Otherwise, use the `SIGTERM` signal. The default value if no value is specified is `false` |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.OCIContainer.stopContainer",
    "params": {
        "containerId": "com.bskyb.epgui",
        "force": true
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "success": true
    }
}
```

<a name="Notifications"></a>
# Notifications

Notifications are autonomous events, triggered by the internals of the implementation, and broadcasted via JSON-RPC to all registered observers. Refer to [[Thunder](#Thunder)] for information on how to register for a notification.

The following events are provided by the org.rdk.OCIContainer plugin:

OCIContainer interface events:

| Event | Description |
| :-------- | :-------- |
| [onContainerStarted](#onContainerStarted) | Triggered when a new container has started running |
| [onContainerStopped](#onContainerStopped) | Triggered when the container has stopped running |


<a name="onContainerStarted"></a>
## *onContainerStarted*

Triggered when a new container has started running.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.descriptor | integer | The container descriptor |
| params.name | string | Name of the Container |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.onContainerStarted",
    "params": {
        "descriptor": 91,
        "name": "com.bskyb.epgui"
    }
}
```

<a name="onContainerStopped"></a>
## *onContainerStopped*

Triggered when the container has stopped running.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.descriptor | integer | The container descriptor |
| params.name | string | Name of the Container |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.onContainerStopped",
    "params": {
        "descriptor": 91,
        "name": "com.bskyb.epgui"
    }
}
```

