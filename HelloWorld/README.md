Below are details for developing a minimal helloworld thunder plugin which basically maintains some user text message and returns it on ask. It also has functionality to print the message to console and send the message as echo event.

Prepare interface:

1,Define a new interface file IHelloWorld.h inside wpeframework-interfaces module within git/interfaces directory (Reference seen in ThunderInterfaceChanges folder)

In below class,

@text means the text user has to use to access a method via json-rpc.
@json means the json-rpc support is provided for this method/class. We can see corresponding registration for json-rpc is present in JSON<Interface>.h file after build.
@json:omit means to omit the json-rpc support is provided for this method.
@stubgen:stub means to omit the com-rpc support for this method.
@event means the event handler/sink class.

  // @json @text:keep
struct EXTERNAL IHelloWorld : virtual public Core::IUnknown {
  enum { ID = ID_HELLO_WORLD };

  // @event
  struct EXTERNAL INotification : virtual public Core::IUnknown {
    enum { ID = ID_HELLO_WORLD_NOTIFICATION };

    // @text onEcho
    // @brief Triggered when echo() is called
    // @param message: the message echoed
    virtual void onEcho(const string& message) = 0;
  };


  virtual uint32_t Register(Exchange::IHelloWorld ::INotification* notification /* @in */) = 0;
  virtual uint32_t Unregister(Exchange::IHelloWorld ::INotification* notification /* @in */) = 0;

  // @json @text getMessage
  virtual Core::hresult GetHelloWorldMessage(string& result /* @out */) = 0;

  // @json @text setMessage
  virtual Core::hresult SetHelloWorldMessage(const string& message /* @in */) = 0;

  // @json @text logHelloWorldMessage
  virtual Core::hresult LogHelloWorldMessage() = 0;

  // @json:omit @text logHelloWorldMessageComRpcOnly
  virtual Core::hresult LogHelloWorldMessageComRpcOnly() = 0;

  // @json @stubgen:stub @text logHelloWorldMessageJsonRpcOnly
  virtual Core::hresult LogHelloWorldMessageJsonRpcOnly() = 0;

  // @json @text echo
  virtual Core::hresult Echo(const string& message /* @in */) = 0;
};

2, Define the values for ID_HELLO_WORLD, ID_HELLO_WORLD_NOTIFICATION in interfaces/Ids.h file.
3, Build wpeframework-interfaces component.
This generates below files:

build/interfaces/generated/ProxyStubs_HelloWorld.cpp
- Holds COM-RPC interface for all function calls.
- Will have a stub implementation for api if mentioned not supported with @stubgen:omit in interface file.

build/definitions/generated/json_IHelloWorld.h
- Auto generated and has contents for its own puprose

build/definitions/generated/JsonData_HelloWorld.h
- Auto generated and has contents for its own puprose to convert between json and primitive types

build/definitions/generated/JHelloWorld.h
- Have functionalities to register and unregister json-rpc calls with user strings with actual function names.
- This won't have register and unregister lines for api's mentioned as @json:omit

Prepare plugin:
1, Define two classes HelloWorld and HelloWorldImplementation.
2, Details of HelloWorld:

a, Includes JsonData_HelloWorld.h and JHelloWorld.h.
b, Implements IHelloWorld::INotification. This is a notification sink and receives event details to be sent to external world. This will further invoke JHelloWorld's version of event to send this to external world.
c, On initialize, performs below things:
- Instantiates HelloWorldImplementation.
- Register above HelloWorldImplementation handle with JHelloWorld, as a indicator for the handler for IHelloWorld interface calls.
- Register itself with HelloWorldImplementation as notification handler.

d, On Deinitialize, performs below things:
- Unregister itself as notification handler from HelloWorldImplementation.
- Unregister HelloWorldImplementation handle from JHelloWorld.
- Deinstantiate HelloWorldImplementation.

e, This class always live in space of WPEFramework process.
f, This class will be built in name of libWPEFrameworkHelloWorld.so


3,Details of HelloWorldImplementation:

a, Implements IHelloWorld
b, Maintains notification handle (HelloWorld instance above) to send any notifications
c, Handles all IHelloWorld function calls called via JSON-RPC or COM-RPC
d, This class may or many not live in space of WPEFramework process based on running as in-process or out-ofprocess plugin
e, This class will be built in name of libWPEFrameworkHelloWorldImplementation.so

4, There will be HelloWorld.json defined which holds config details of plugin. We need to mention locator as libWPEFrameworkHelloWorldImplementation.so in config.
5, Build rdkservices component with this plugin.

