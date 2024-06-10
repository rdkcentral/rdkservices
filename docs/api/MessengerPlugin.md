<!-- Generated automatically, DO NOT EDIT! -->
<a name="Messenger_Plugin"></a>
# Messenger Plugin

**Version: [1.1.4](https://github.com/rdkcentral/rdkservices/blob/main/Messenger/CHANGELOG.md)**

A Messenger plugin for Thunder framework.

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

The `Messenger` plugin allows exchanging text messages between users gathered in virtual rooms. The rooms are dynamically created and destroyed based on user attendance. Upon joining a room, the client receives a unique token (room ID) to be used for sending and receiving the messages.

The plugin is designed to be loaded and executed within the Thunder framework. For more information about the framework refer to [[Thunder](#Thunder)].

<a name="Configuration"></a>
# Configuration

The table below lists configuration options of the plugin.

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| callsign | string | Plugin instance name (default: *Messenger*) |
| classname | string | Class name: *Messenger* |
| locator | string | Library name: *libWPEFrameworkMessenger.so* |
| autostart | boolean | Determines if the plugin shall be started automatically along with the framework |

<a name="Methods"></a>
# Methods

The following methods are provided by the Messenger plugin:

Messenger interface methods:

| Method | Description |
| :-------- | :-------- |
| [join](#join) | Joins a messaging room |
| [leave](#leave) | Leaves a messaging room |
| [send](#send) | Sends a message to a room |


<a name="join"></a>
## *join*

Joins a messaging room.

### Description

Use this method to join a room. If the specified room does not exist, then it will be created.

### Events

| Event | Description |
| :-------- | :-------- |
| [userupdate](#userupdate) | Notifies about user status updates. |
### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.user | string | User name to join the room under (must not be empty) |
| params.room | string | Name of the room to join (must not be empty) |
| params?.secure | string | <sup>*(optional)*</sup> Room security (must be one of the following: *insecure*, *secure*) |
| params?.acl | array | <sup>*(optional)*</sup> Access-control list for secure room |
| params?.acl[#] | string | <sup>*(optional)*</sup> URL origin with possible wildcards |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.roomid | string | Unique ID of the room |

### Errors

| Code | Message | Description |
| :-------- | :-------- | :-------- |
| 5 | ```ERROR_ILLEGAL_STATE``` | User name is already taken (i.e. the user has already joined the room) |
| 30 | ```ERROR_BAD_REQUEST``` | User name or room name was invalid |
| 24 | ```ERROR_PRIVILEGED_REQUEST``` | Room security errors |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "Messenger.join",
    "params": {
        "user": "Bob",
        "room": "Lounge",
        "secure": "secure",
        "acl": [
            "https://*.github.io"
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
        "roomid": "1e217990dd1cd4f66124"
    }
}
```

<a name="leave"></a>
## *leave*

Leaves a messaging room.

### Description

Use this method to leave a room. The room ID becomes invalid after this call. If there are no more users, the room will be destroyed and related resources freed.

### Events

| Event | Description |
| :-------- | :-------- |
| [userupdate](#userupdate) | Notifies about user status updates. |
### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.roomid | string | ID of the room to leave |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | null | Always null |

### Errors

| Code | Message | Description |
| :-------- | :-------- | :-------- |
| 22 | ```ERROR_UNKNOWN_KEY``` | The given room ID was invalid |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "Messenger.leave",
    "params": {
        "roomid": "1e217990dd1cd4f66124"
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": null
}
```

<a name="send"></a>
## *send*

Sends a message to a room.

### Description

Use this method to send a message to a room.

### Events

| Event | Description |
| :-------- | :-------- |
| [message](#message) | Notifies about new messages in a room. |
### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.roomid | string | ID of the room to send the message to |
| params.message | string | The message content to send |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | null | Always null |

### Errors

| Code | Message | Description |
| :-------- | :-------- | :-------- |
| 22 | ```ERROR_UNKNOWN_KEY``` | The given room ID was invalid |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "Messenger.send",
    "params": {
        "roomid": "1e217990dd1cd4f66124",
        "message": "Hello!"
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": null
}
```

<a name="Notifications"></a>
# Notifications

Notifications are autonomous events, triggered by the internals of the implementation, and broadcasted via JSON-RPC to all registered observers. Refer to [[Thunder](#Thunder)] for information on how to register for a notification.

The following events are provided by the Messenger plugin:

Messenger interface events:

| Event | Description |
| :-------- | :-------- |
| [roomupdate](#roomupdate) | Notifies about room status updates |
| [userupdate](#userupdate) | Notifies about user status updates |
| [message](#message) | Notifies about new messages in a room |


<a name="roomupdate"></a>
## *roomupdate*

Notifies about room status updates.

### Description

Register to this event to be notified about room status updates. Immediately after registering to this notification the listener will sequentially receive updates of all rooms that have been created so far.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.room | string | Name of the room this notification relates to |
| params?.secure | string | <sup>*(optional)*</sup> Room security (must be one of the following: *insecure*, *secure*) |
| params.action | string | Specifies the room status change, e.g. created or destroyed (must be one of the following: *created*, *destroyed*) |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.roomupdate",
    "params": {
        "room": "Lounge",
        "secure": "secure",
        "action": "created"
    }
}
```

<a name="userupdate"></a>
## *userupdate*

Notifies about user status updates.

### Description

Register to this event to be notified about room status updates. Immediately after registering to this notification the listener will sequentially receive updates of all users that have joined the room so far.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.user | string | Name of the user that has this notification relates to |
| params.action | string | Specifies the user status change, e.g. join or leave a room (must be one of the following: *joined*, *left*) |

> The *room ID* argument shall be passed within the designator, e.g. *1e217990dd1cd4f66124.client.events.1*.

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "1e217990dd1cd4f66124.client.events.userupdate",
    "params": {
        "user": "Bob",
        "action": "joined"
    }
}
```

<a name="message"></a>
## *message*

Notifies about new messages in a room.

### Description

Register to this event to be notified about new messages in a room.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.user | string | Name of the user that has sent the message |
| params.message | string | Content of the message |

> The *room ID* argument shall be passed within the designator, e.g. *1e217990dd1cd4f66124.client.events.1*.

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "1e217990dd1cd4f66124.client.events.message",
    "params": {
        "user": "Bob",
        "message": "Hello!"
    }
}
```

