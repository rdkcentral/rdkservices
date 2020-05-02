# Security Agent

The security agent of thunder is responsible to allow or block access to the Thunder API. This is to prevent unauthorized access to the Thunder API running on a device. 


## High level


The Thunder generates a token that is exposed through the native code to the application runtime. For example in case of the WPE WebKit browser the tokens are exposed through an injected bundle. Once the application retrieves the token from the native code it can be used on the connection request to Thunder.


The Thunder Security Agent has an access control list that provides access to, or a subset of, Thunder APIs based on application origin. This allows various applications to get different sets of APIs, all APIs or none.


The security token is per [RFC 7519](https://tools.ietf.org/html/rfc7519).

## Access control list

The access control list of the security agent is divided into two sections:

- Origin to group mapping
- Group to access mapping


The origin to group mapping maps a specific origin of an application to a group of applications. In turn the group of applications has a list of APIs that are either allowed or denied to be accessed. 

For an example please see [the following example](https://github.com/WebPlatformForEmbedded/ThunderNanoServices/blob/master/SecurityAgent/data.json).


## Overview

For a simplified overivew please see below:

     __________                __________                __________
    |          |              |          |              |          |
    |   App    | -- Token --> |   App    | -- Token --> |  Thunder |
    |          |    Request   |  Engine  |    Native    |          |
    |__________|              |__________|              |__________|
          |                                                  |
          |                    API request                   |
           ---------------------- Token ----------------------
