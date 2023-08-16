/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2020 RDK Management
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
 
#include "JavaScriptFunctionType.h"
#include "Utils.h"
#include "../Tags.h"

unsigned int amazon_player_deinitialize();

namespace WPEFramework {
namespace JavaScript {
namespace Amazon {

    static char amazonLibrary[] = _T("libamazon_player.so");

    class AmazonPlayer {
    private:
        typedef unsigned int ( *Initialize ) (const char* config );
        typedef unsigned int ( *Deinitialize ) ();
        typedef void ( *MessageListenerType )( const std::string& msg );
        typedef bool ( *RegisterMessageListenerType )( MessageListenerType inMessageListener );
        typedef std::string ( *SendMessageType )( const std::string& );

        // Gets configuration for this handler from WPEWebKitBrowser via synchronous message.
        static std::string Configuration()
        {
            std::string utf8MessageName(string(Tags::Config) + "hawaii");

            WKStringRef jsMessageName = WKStringCreateWithUTF8CString(utf8MessageName.c_str());
            WKMutableArrayRef messageBody = WKMutableArrayCreate();
            WKTypeRef returnData;

            WKBundlePostSynchronousMessage(WebKit::Utils::GetBundle(), jsMessageName, messageBody, &returnData);

            std::string result (WebKit::Utils::WKStringToString(static_cast<WKStringRef>(returnData)));

            WKRelease(returnData);
            WKRelease(messageBody);
            WKRelease(jsMessageName);

            return result;
        }
        static void ListenerCallback (const std::string& msg)
        {
            AmazonPlayer& instance(AmazonPlayer::Instance());

            if (instance._jsContext) {
                //std::cerr << "MESSAGE: " << msg << std::endl;

                JSValueRef passedArgs;

                JSStringRef message = JSStringCreateWithUTF8CString(msg.c_str());
                passedArgs = JSValueMakeString(instance._jsContext, message);

                JSObjectRef globalObject = JSContextGetGlobalObject(instance._jsContext);
                JSObjectCallAsFunction(instance._jsContext, instance._jsListener, globalObject, 1, &passedArgs, NULL);
                JSStringRelease(message);
            }
        }

        AmazonPlayer()
            : library(amazonLibrary)
            , _registerMessageListener(nullptr)
            , _sendMessage(nullptr)
            , _jsContext()
            , _jsListener()
        {
            if (library.IsLoaded() == true) {
                _registerMessageListener = reinterpret_cast<RegisterMessageListenerType>(library.LoadFunction(_T("registerMessageListener")));
                _sendMessage = reinterpret_cast<SendMessageType >(library.LoadFunction(_T("sendMessage")));
            } 

            if ((_registerMessageListener == nullptr) || (_sendMessage == nullptr)) {
                TRACE(Trace::Fatal, (_T("FAILED Library loading: %s"), amazonLibrary));
            }
            else {
                Initialize initializer = reinterpret_cast<Initialize>(library.LoadFunction(_T("amazon_player_initialize")));

                if (initializer != nullptr) {
                    // Time to initialize
                    std::string config = Configuration();
                    
                    // Call the initailze on the library...
                    unsigned int result = initializer(config.c_str());
                }
            }
        }
 
    public:
        AmazonPlayer(const AmazonPlayer&) = delete;
        AmazonPlayer& operator= (const AmazonPlayer&) = delete;

        static AmazonPlayer& Instance() {
            static AmazonPlayer _singleton;
            return (_singleton);
        }

       ~AmazonPlayer() 
        {
            if (_jsListener != nullptr) {
                JSValueUnprotect(_jsContext, _jsListener);

                // Also call the detinitailze on the library...
                Deinitialize deinitializer = reinterpret_cast<Deinitialize>(library.LoadFunction(_T("amazon_player_deinitialize")));
                unsigned int result;

                if ((deinitializer != nullptr) && ( (result = deinitializer()) != 0)) {
                    TRACE(Trace::Fatal, (_T("Could not preperly unload the Hawaii interface. Error: %d"), result));
                }
            }
        }

    public:
        void RegisterMessageListener(JSContextRef& context, JSObjectRef& listener) {
            if ( (_jsListener == nullptr) && (_registerMessageListener != nullptr))  {
                assert (_jsContext == nullptr);

                _jsListener = listener;
                JSValueProtect(context, _jsListener);
                _jsContext  = JSContextGetGlobalContext(context);
                _registerMessageListener(ListenerCallback);
            }
        }
        void SendMessage(const string& message) {
            if (_sendMessage != nullptr) {
                _sendMessage(message);
            }
        }

    private:
        Core::Library library;
        RegisterMessageListenerType _registerMessageListener;
        SendMessageType _sendMessage;
        JSContextRef _jsContext;
        JSObjectRef _jsListener;
    };

    class registerMessageListener {
    public:
        registerMessageListener() = default;
        ~registerMessageListener() = default;

        JSValueRef HandleMessage(JSContextRef context, JSObjectRef,
                                 JSObjectRef, size_t argumentCount, const JSValueRef arguments[], JSValueRef*)
        {
            if (argumentCount != 1) {
                TRACE(Trace::Information, (_T("Hawaii::registerMessageListener expects 1 argument")));
            }
            else {
                JSObjectRef jsListener = JSValueToObject(context, arguments[0], nullptr);
        
                if (!JSObjectIsFunction(context, jsListener)) {
                    TRACE(Trace::Information, (_T("Hawaii::registerMessageListener expects a funtion argument")));
                }
                else {
                    AmazonPlayer::Instance().RegisterMessageListener(context, jsListener);
                }
            }

            return JSValueMakeNull(context);
        }
    };

    class sendMessage {
    public:
        sendMessage() = default;
        ~sendMessage() = default;

        JSValueRef HandleMessage(JSContextRef context, JSObjectRef,
                                 JSObjectRef, size_t argumentCount, const JSValueRef arguments[], JSValueRef*)
        {
            if (argumentCount != 1) {
                TRACE(Trace::Information, (_T("Hawaii::sendMessage expects one argument")));
                std::cerr << "sendMessage expects one argument" << std::endl;
            }
            else if (!JSValueIsString(context, arguments[0])) {
                TRACE(Trace::Information, (_T("Hawaii::sendMessage expects a string argument")));
            }
            else {
                JSStringRef jsString = JSValueToStringCopy(context, arguments[0], nullptr);
                size_t bufferSize = JSStringGetLength(jsString) + 1;
                char stringBuffer[bufferSize];

                JSStringGetUTF8CString(jsString, stringBuffer, bufferSize);

                //TRACE(Trace::Information, (_T("Hawaii::sendMessage(%s)"), stringBuffer));

                AmazonPlayer::Instance().SendMessage(stringBuffer);

                JSStringRelease(jsString);
            }

            return JSValueMakeNull(context);
        }
    };

    static JavaScriptFunctionType<registerMessageListener> _registerInstance(_T("hawaii"));
    static JavaScriptFunctionType<sendMessage> _sendInstance(_T("hawaii"));

} // namespace Amazon
} // namespace JavaScript
} // namespace WPEFramework
