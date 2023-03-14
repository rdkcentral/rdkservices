# 2. Plugin Json API Spec

Create `FooPlugin.json`. This JSON file provides the API spec for `FooPlugin`.

This API spec is used to generate the JSON data and enum classes used for JSON-RPC.

This API spec is also used to generate plugin documentation that is hosted on the [RDK Services Documentation wiki](https://rdkcentral.github.io/rdkservices/#/README).

Example:
```JSON
{
    "$schema": "https://raw.githubusercontent.com/rdkcentral/rdkservices/main/Tools/json_generator/schemas/interface.schema.json",
    "jsonrpc": "2.0",
    "info": {
        "title": "FooPlugin API",
        "class": "FooPlugin",
        "description": "The `FooPlugin` plugin allows you to persist key/value pairs"
    },
    "common": {
        "$ref": "../common/common.json"
    },
    "definitions": {
        "key": {
            "summary": "The key name as a valid UTF-8 string",
            "type": "string",
            "example": "key1"
        },
        "value": {
            "summary": "The key value. Values are capped at 1000 characters in size.",
            "type": "string",
            "example": "value1"
        }
    },
    "methods": {
        "setValue": {
            "summary": "Sets the value of a given key",
            "events": {
                "onValueChanged": "Triggered whenever any of the values stored are changed using setValue"
            },
            "params": {
                "type": "object",
                "properties": {
                    "key": {
                        "$ref": "#/definitions/key"
                    },
                    "value": {
                        "$ref": "#/definitions/value"
                    }
                },
                "required": [
                    "key",
                    "value"
                ]
            },
            "result": {
                "$ref": "#/common/result"
            }
        }
    },
    "events": {
        "onValueChanged": {
            "summary": "Triggered whenever any of the values stored are changed using setValue",
            "params": {
                "type": "object",
                "properties": {
                    "key": {
                        "$ref": "#/definitions/key"
                    },
                    "value": {
                        "$ref": "#/definitions/value"
                    }
                },
                "required": [
                    "key",
                    "value"
                ]
            }
        }
    }
}
```

### It contains the following sections:

#### a) Schema:
A schema defines the structure of a JSON document, including the type of data that can be stored in it, the names of fields and their types, and constraints on the values that can be stored in those fields. The purpose of a schema is to provide a blueprint for how the data in a JSON document should be organized, so that applications can validate the data before processing it.

RDK Services API specs use [JSON Schema](https://json-schema.org/).

Following lines should be added to `FooPlugin.json` to point to the latest schema used.

```JSON
"$schema": "https://raw.githubusercontent.com/rdkcentral/rdkservices/main/Tools/json_generator/schemas/interface.schema.json",
```
Also, Add the following to indicate that spec supports JSON-RPC protocol version 2.0
```JSON
"jsonrpc": "2.0",
```
#### b) Info:
The Info section updates information such as Title, Description, and Class name as mentioned in the below sample.
```JSON
"info": {
    "title": "FooPlugin API",
    "class": "FooPlugin",
    "description": "The `FooPlugin` plugin allows you to set a value and notify changes to the value"
    },
```

#### c) Definitions:
This section defines global definitions such as key values, data types, etc., for use in the spec as mentioned in the below sample.

```JSON
"definitions": {
        "key": {
            "summary": "The key name as a valid UTF-8 string",
            "type": "string",
            "example": "key1"
        },
        "value": {
            "summary": "The key value. Values are capped at 1000 characters in size.",
            "type": "string",
            "example": "value1"
        }
    },
```
#### d) Methods:
This section defines the APIs provided by this plugin as mentioned in the below sample.

```JSON
"setValue": {
    "summary": "Sets the value of a given key",
    "events": {
        "onValueChanged": "Triggered whenever any of the values stored are changed using setValue"
    },
    "params": {
        "type": "object",
        "properties": {
            "key": {
                "$ref": "#/definitions/key"
            },
            "value": {
                "$ref": "#/definitions/value"
            }
        },
        "required": [
            "key",
            "value"
        ]
    }  
    "result": {
        "type": "object",
        "properties": {
            "success": {
                "$ref": "#/success"
            }
        },
        "required": [
            "success"
        ]
    },
}
```

- The `key` specifies the API name `setValue`. APIs names follow camelCase starting with lower case.
- The `summary` provides a detailed description about the API, as that description will be useful for the clients of the API. This is also included in the API documentation.
- The `params` list all the input parameters for this API and the `required` field within the `params` lists all the required input parameters only.
- The `result` provides details of the output response.

#### e) Events:
This section defines the events supported by this plugin as mentioned in the below sample.

```JSON
"onValueChanged": {
    "summary": "Triggered whenever any of the values stored are changed using setValue",
    "params": {
        "type": "object",
        "properties": {
            "key": {
                "$ref": "#/definitions/key"
            },
            "value": {
                "$ref": "#/definitions/value"
            }
        },
        "required": [
            "key",
            "value"
        ]
    }
}
```
- The `key` specifies the event name `onValueChanged`. Event names follow camelcase starting with lower case.
- The `summary` provides a detailed description about the event, as that description will be useful for the clients of the API. This is also included in the API documentation.
- The `params` list all the parameters that will be included in the event payload and the `required` field within the `params` lists all the required parameters only.
