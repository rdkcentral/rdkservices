# 3. Plugin Json API Spec

This JSON file provides the API spec for the plugin. This uses JSON Schema [https://json-schema.org/](https://json-schema.org/).

## It contains the following sections:

### a) Schema:
A schema defines the structure of a JSON document, including the type of data that can be stored in it, the names of fields and their types, and constraints on the values that can be stored in those fields. The purpose of a schema is to provide a blueprint for how the data in a JSON document should be organized, so that applications can validate the data before processing it.

The Latest Schema Syntax:
```JSON
"$schema": "https://raw.githubusercontent.com/rdkcentral/rdkservices/main/Tools/json_generator/schemas/interface.schema.json",
```
Also, Jsonrpc version has to be updated which indicates that the spec supports jsonrpc protocol version 2.0.
```JSON
    "jsonrpc": "2.0",
```
### b) Info:
The Info section updates information such as Title, Description, and Class name as mentioned in the below sample.
```JSON

    "info": {
      "title": "SecurityAgent API",
      "class": "SecurityAgent",
      "description": "The `SecurityAgent` plugin is responsible for allowing or blocking access to the Thunder APIs."
    },
```

### c) Definitions:
This section defines global definitions such as key values, data types, etc., for use in the spec as mentioned in the below sample.
```JSON
 "value": {
            "summary": "The key value. Values are capped at 1000 characters in size.",
            "type": "string",
            "example": "value1"
        }
```
### d) Methods:
This section defines the APIs provided by this plugin as mentioned in the below sample.
```JSON
"deleteKey":{
            "summary": "Deletes a key from the specified namespace.",
            "params": {
                "type": "object",
                "properties": {
                    "namespace": {
                        "$ref": "#/definitions/namespace"
                    },
                    "key": {
                        "$ref": "#/definitions/key"
                    }
                },
                "required": [
                    "namespace",
                    "key"
                ]
            },
            "result": {
                "$ref": "#/common/result"
            }
        }
```

- The **key** specifies the **API name**.
- The **"summary"** provides a detailed description about the API, as that description will be useful for the clients of the API. This is also included in the API documentation.
- The **"params"** list all the input parameters for this API and the **"required"** field within the **"params"** lists all the required input parameters only.
- The **"result"** provides details of the output response.

### e) Events:
This section defines the events supported by this plugin as mentioned in the below sample.
```JSON
"onValueChanged": {
            "summary": "Triggered whenever any of the values stored are changed using setValue.",
            "params": {
                "type": "object",
                "properties": {
                    "namespace": {
                        "$ref": "#/definitions/namespace"
                    },
                    "key": {
                        "$ref": "#/definitions/key"
                    },
                    "value": {
                        "$ref": "#/definitions/value"
                    }
                },
                "required": [
                    "namespace",
                    "key",
                    "value"
                ]
            }
        }
```
- The **key** specifies the **event name**.
- The **"summary"** provides a detailed description about the event, as that description will be useful for the clients of the API. This is also included in the API documentation.
- The **"params"** list all the parameters that will be included in the event payload. and the **"required"** field within the **"params"** lists all the required parameters only.

For example, please refer to link [SecurityAgent JSON file](https://github.com/rdkcentral/rdkservices/blob/sprint/23Q1/SecurityAgent/SecurityAgent.json).

