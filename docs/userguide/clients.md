# Overview of Clients

There are several client options that application developers use to access RDK services from their applications. The client technologies discussed here are not exclusive. That is, any client library that support JSON and WebSockets can be used by an application. However, the following technologies are common:

* cURL
* ThunderJS client library
* Thunder C++ client library

## cURL Clients

[cURL](https://curl.se/) is a common command-line tool and library
for transferring data with URLs. The command-line tool is an ideal choice during testing and development and also common in scripts. cURL also provides the libcurl library that can be used directly by applications. Many of the examples in the RDK Services Documentation use cURL command-line examples to demonstrate using services.

## ThunderJS Clients

ThunderJS is a flexible and extensible JavaScript library designed to take advantage of the Thunder framework. It is an isomorphic library, which means it can be used in a browser environment as well as a NodeJS environment. ThunderJS makes it easy to make API calls over a WebSocket connection and can also be used to listen to (and act upon) notifications.

## Thunders C++ Clients

Thunder C++ is a platform abstraction layer for generic functionality. Native C/C++ application use the Thunder C++ client libraries to make in-process API calls that directly use JSON-RPC service interfaces.
