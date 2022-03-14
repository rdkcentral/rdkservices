# Creating Native Clients

The sample discussed here is taken from a test client for the SystemService service and is included in the rdkservices project. For the complete test code sample, see [rdkservices](https://github.com/rdkcentral/rdkservices/tree/master/SystemServices/TestClient).

Applications that require the best possible performance can choose to directly access RDK services in the same process as the Thunder Framework. These applications are often implemented as native applications.

Native application are written in C or C++ and use core Thunder C++ libraries to access JSON-RPC service interfaces over WebSockets. In particular, applications make use of functionality accessible through the core.h and WebSocket.h interfaces. Make sure to include them in your application.

Native applications access RDK services using the Thunder instance address and port. The default address is 127.0.0.1 and the default port is 9998. Set the address and port prior to invoking an RDK service. For example:

```c++
Core::SystemInfo::SetEnvironment(_T("THUNDER_ACCESS"), (_T("127.0.0.1:9998")));
```

A best practice is to create a reusable function for invoking remote JSON-RPC services and their methods. Besides calling the remote object, the function needs to pass any method parameters and must also capture the service response. For example:

```c++
bool invokeJSONRPC(JSONRPC::LinkType<Core::JSON::IElement> *remoteObject, std::string method, JsonObject &param, JsonObject &result)
{
    bool ret = false;
    uint32_t retStatus = Core::ERROR_GENERAL;
    std::string response;
    assert(remoteObject != NULL);
    param.ToString(response);
    printf("\ninvokeJSONRPC '%s' with param: '%s'\n", method.c_str(), response.c_str());
    retStatus = remoteObject->Invoke<JsonObject, JsonObject>(5000, _T(method), param, result);
    if (Core::ERROR_NONE != retStatus) {
        printf("\nremoteObject->Invoke '%s' failed [retStatus: 0x%x]\n", method.c_str(), retStatus);
    } else {
        ret = true;
    }
    return ret;
}
```

The SystemService service has many methods that your application can use. For each method you wish to call, create a respective function and handle the JSON response. The response always includes a result JSON Object. The following example is for the `getSystemVersions` method which returns set-top device information:

```c++
void getSystemVersions(std::string methodName, JSONRPC::LinkType<Core::JSON::IElement> *remoteObject)
{
    printf("[%llu] Inside (%s)\n", TimeStamp(), __FUNCTION__);
    JsonObject parameters, response;
    std::string result;
    if (invokeJSONRPC(remoteObject, methodName, parameters, response)) {
        response.ToString(result);
        printf("\nResponse: '%s'\n", result.c_str());
    }
}
```

You can now call the getSystemVersions method on the remote object:

```c++
getSystemVersions(getMethodName((getSystemVersions)retStatus), remoteObject);
```