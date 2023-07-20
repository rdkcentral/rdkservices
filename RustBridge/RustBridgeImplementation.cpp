/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2022 Metrological
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

#include "RustBridge.h"

namespace WPEFramework {

	namespace Plugin {

		namespace Rust {
			struct RequestContext {
				uint32_t    id;
				const char* auth_token;
			};

			struct PluginContext;

			struct Plugin;
		}
	}
}

extern "C" void wpe_send_to(uint32_t id, const char* json, WPEFramework::Plugin::Rust::PluginContext* p_ctx);

namespace WPEFramework {

	namespace Plugin {

		class RustBridgeImplementation :  public Exchange::IRustBridge {
		private:
			class Config : public Core::JSON::Container {
			public:
				Config(const Config&) = delete;
				Config& operator=(const Config&) = delete;
				Config() 
					: Core::JSON::Container()
					, ModuleName()
				{
					Add(_T("module"), &ModuleName);
				}
				~Config() override = default;
				
			public:
				Core::JSON::String ModuleName;
			};
			
			class Connector {
			private:
				using fn_SendTo = void (*)(uint32_t, const char *, Rust::PluginContext *);
				using fn_Create = Rust::Plugin *(*)(const char *name, fn_SendTo send_to, Rust::PluginContext *, void *);
				using fn_Destroy = void (*)(Rust::Plugin *p);
				using fn_Init = void (*)(Rust::Plugin *p, const char *json);
				using fn_Invoke = void (*)(Rust::Plugin *p, const char *json_req, Rust::RequestContext req_ctx);

				// TODO: Discuss this since I think these are not needed. I assume it is to send events using the 
				// fn_SendTo function, propse to make it an explicit function called events or so and signal this 
				// pluginthat all interested ones should recive that event, with that parameter
				using fn_OnClientConnect = void (*)(Rust::Plugin *p, uint32_t channel_id);
				using fn_OnClientDisconnect = void (*)(Rust::Plugin *p, uint32_t channel_id);
							
			public:
				Connector() = delete;
				Connector(const Connector&) = delete;
				Connector& operator= (const Connector&) = delete;

				Connector(RustBridgeImplementation& parent)
					: _parent(parent)
					, _library()
					, _fn_create(nullptr)
					, _fn_destroy(nullptr)
					, _fn_init(nullptr)
					, _fn_invoke(nullptr)
					, _fn_on_client_connect(nullptr)
					, _fn_on_client_disconnect(nullptr) {
				}
				~Connector() = default;
				
			public:
				bool IsValid() const {
					return (_library.IsLoaded() == true);
				}
				uint32_t Initialize(const std::vector<string>& thunderPaths, const string& locator, const string& className, const string& configLine) {
					
					uint32_t result = Load(thunderPaths, locator);
					
					if (result == Core::ERROR_NONE) {
						ASSERT(_library.IsLoaded() == true);
						ASSERT(_fn_create != nullptr);
						ASSERT(_fn_init != nullptr);
						
						// TODO: Discuss why we want to pass back a function durint the create that is already part of the 
						// rust library where we are sending the information. I guess we can drop it (_fn_service_metadata)
						_plugin = _fn_create(className.c_str(), &wpe_send_to, reinterpret_cast<Rust::PluginContext*>(&_parent), _fn_service_metadata);

						if (_plugin != nullptr) {
							// TODO: indeed the initialize seems a bit redundant. Can we drop it and pass the configuration during the create?
							_fn_init(_plugin, configLine.c_str());
						}
						else {
							// Oops could not create a plugin. Send out a message
							TRACE(Trace::Fatal, (Core::Format(_T("Creating the %s RUST plugin failed"), locator.c_str())));
							result = Core::ERROR_BAD_REQUEST;
						}
					}					
					
					return (result);
				}
				uint32_t Deinitialize() {
					uint32_t result = Core::ERROR_NONE;
					
					if (_library.IsLoaded() == true) {
						ASSERT(_fn_destroy != nullptr);
						ASSERT(_plugin != nullptr);

						_fn_destroy(_plugin);
						
						_fn_create               = nullptr;
						_fn_init                 = nullptr;
						_fn_destroy              = nullptr;
						_fn_invoke               = nullptr;
						_fn_on_client_connect    = nullptr;
						_fn_on_client_disconnect = nullptr;
						_library = Core::Library();
					}		

					return (result);
				}
				void Request(const uint32_t id, const string& token, const string& method, const string& parameters) {
					Core::JSONRPC::Message message;

					message.Id = id;
					message.Designator = method;
					message.Parameters = parameters;
			
					string result;
					message.ToString(result);

					Rust::RequestContext context;

					context.auth_token = token.c_str();
					context.id = id;

					// TODO: As invoke suggests a synchronous call, which it actually is not ;-) can we rename it to Request ?
					_fn_invoke(_plugin, result.c_str(), context);
				}

			private:
				uint32_t LoadLibrary(const string& name) {
					uint32_t result = Core::ERROR_NOT_EXIST;
					
					Core::File libraryToLoad(name);

					if (libraryToLoad.Exists() == true) {
						Core::Library myLib(name.c_str());
						
						if (myLib.IsLoaded() == false) {
							TRACE(Trace::Fatal, (Core::Format(_T("Could not load the library: %s, error: %s"), name.c_str(), myLib.Error().c_str())));
							result = Core::ERROR_OPENING_FAILED;
						}
						else {
							_fn_create = reinterpret_cast<fn_Create>(myLib.LoadFunction(_T("wpe_rust_plugin_create")));
							_fn_init = reinterpret_cast<fn_Init>(myLib.LoadFunction(_T("wpe_rust_plugin_init")));
							_fn_destroy = reinterpret_cast<fn_Destroy>(myLib.LoadFunction(_T("wpe_rust_plugin_destroy")));
							_fn_invoke = reinterpret_cast<fn_Invoke>(myLib.LoadFunction(_T("wpe_rust_plugin_invoke")));
							_fn_on_client_connect = reinterpret_cast<fn_OnClientConnect>(myLib.LoadFunction(_T("wpe_rust_plugin_on_client_connect")));
							_fn_on_client_disconnect = reinterpret_cast<fn_OnClientDisconnect>(myLib.LoadFunction(_T("wpe_rust_plugin_on_client_disconnect")));
							_fn_service_metadata = myLib.LoadFunction(_T("thunder_service_metadata"));
							
							if ( (_fn_create               != nullptr) &&
								 (_fn_init                 != nullptr) &&
								 (_fn_destroy              != nullptr) &&
								 (_fn_invoke               != nullptr) &&
								 (_fn_on_client_connect    != nullptr) &&
								 (_fn_on_client_disconnect != nullptr) ) {
								_library = myLib;
								result = Core::ERROR_NONE;
							}
							else {
								TRACE(Trace::Fatal, (Core::Format(_T("Could not load all the symbols from library: %s"), name.c_str())));
								
								_fn_create               = nullptr;
								_fn_init                 = nullptr;
								_fn_destroy              = nullptr;
								_fn_invoke               = nullptr;
								_fn_on_client_connect    = nullptr;
								_fn_on_client_disconnect = nullptr;
								_fn_service_metadata     = nullptr;
								
								result = Core::ERROR_UNAVAILABLE;
							}
						}
					}
					return (result);
				}
				uint32_t Load(const std::vector<string>& thunderPaths, const string& locator) {
					
					uint32_t result = Core::ERROR_NOT_EXIST;
					
					ASSERT (locator.empty() == false);
					
					// By definitions we start in the usual locations for the plugins..
					std::vector<string>::const_iterator index(thunderPaths.cbegin());
					while ((index != thunderPaths.cend()) && ((result = LoadLibrary(*index + locator)) != Core::ERROR_NONE)) {
						++index;
					}	
					
					if (result != Core::ERROR_NONE) {
						
						string value;
						
						// As this is RUST, maybe we can find it in the LD_LIBRARY_PATH fromt he linux system..
						if (Core::SystemInfo::GetEnvironment(_T("LD_LIBRARY_PATH"), value) == true) {
							Core::TextSegmentIterator iterator (Core::TextFragment(value), true, _T(":"));

							while ( (iterator.Next() == true) && (result != Core::ERROR_NONE) ) {
								result = LoadLibrary(iterator.Current().Text() + locator);
							}
						}
					}
					
					return (result);
				}
				
			private:
				RustBridgeImplementation&	_parent;
				Rust::Plugin*				_plugin;
				Core::Library				_library;
				fn_Create					_fn_create;
				fn_Destroy					_fn_destroy;
				fn_Init						_fn_init;
				fn_Invoke					_fn_invoke;
				fn_OnClientConnect			_fn_on_client_connect;
				fn_OnClientDisconnect		_fn_on_client_disconnect;
				void*						_fn_service_metadata;
			};
			

		public:
			RustBridgeImplementation(const RustBridgeImplementation&) = delete;
			RustBridgeImplementation& operator= (const RustBridgeImplementation&) = delete;
			
			RustBridgeImplementation()
				: _service(nullptr)
				, _connector(*this)
				, _callback(nullptr) {
			}
			~RustBridgeImplementation() {
				_connector.Deinitialize();
				if (_callback != nullptr) {
					_callback->Release();
					_callback = nullptr;
				}

				if (_service != nullptr) {
					_service->Release();
					_service = nullptr;
				}
			}

		public:
			uint32_t Configure(PluginHost::IShell* framework, ICallback* callback) override {
				uint32_t result = Core::ERROR_INCOMPLETE_CONFIG;
				Config config;
				
				config.FromString(framework->ConfigLine());
				
				if (callback != nullptr) {
					string rustModule;
					std::vector<string> searchPath;
					string className(framework->ClassName());

					if (config.ModuleName.Value().empty() == true) {
						rustModule = framework->Callsign() + _T(".so");
					}
					else {
						rustModule = config.ModuleName.Value();
					}
					
					searchPath.push_back(framework->DataPath());
					searchPath.push_back(framework->PersistentPath());
					searchPath.push_back(framework->SystemRootPath());
					searchPath.push_back(framework->VolatilePath());
					
					_callback = callback;

					result = _connector.Initialize(searchPath, rustModule, className, framework->ConfigLine());
					
					if (result != Core::ERROR_NONE) {
						_callback = nullptr;
					}
					else {
						_callback->AddRef();		
						_service = framework;
						_service->AddRef();
					}
				}
				
				return (result);
			}
			
			// ALLOW THUNDER -> RUST (Invoke and Event)
			// The synchronous Invoke from a JSONRPC perspective has been splitup into an 
			// a-synchronous communication to RUST. First we send a Request with an id,
			// Than it is up to RUST to send a response (with the same id) to complete
			// the a-synchronous request, RUST calls the Response() moethod or signals an
			// error on this Request, using the Error() method.
			void Request(const uint32_t id, const string& context, const string& method, const string& parameters) override {
				ASSERT(_connector.IsValid() == true);

				_connector.Request(id, context, method, parameters);
			}
			
			// Allow THUNDER to send an event to the interested subscribers in the RUST
			// world.
			void Event(const string& event, const string& parmeters) override {
				_callback->Event(event, parmeters);
			}

			void Response(const uint32_t id, const string& response, const int32_t errorCode) {
				ASSERT(_callback != nullptr);
				_callback->Response(id, response, errorCode);
			}
			

			BEGIN_INTERFACE_MAP(RustBridgeImplementation)
				INTERFACE_ENTRY(Exchange::IRustBridge)
			END_INTERFACE_MAP

		private:
			PluginHost::IShell* _service;
			Connector _connector;
			ICallback* _callback;
		};
		
		SERVICE_REGISTRATION(RustBridgeImplementation, 1, 0);
	}
} // namespace WPEFramework

using namespace WPEFramework;

// this function is passed into Rust as a pointer
extern "C" void wpe_send_to(uint32_t id, const char* json, Plugin::Rust::PluginContext * plugin)
{
	Plugin::RustBridgeImplementation* implementation = reinterpret_cast<Plugin::RustBridgeImplementation*>(plugin);

	if (implementation != nullptr) {
		if ((id == 0) && (json[0] == '[')) {
			// TODO: Can we make an explicit call for this or return the methods on the fn_create ?
			// Hacky way to get a JSON list of the supported methods in the class
			implementation->Event(_T("registerjsonrpcmethods"), json);
		}
		else {
			Core::JSONRPC::Message convert;
			convert.FromString(json);

			if (convert.Error.IsSet() == true) {
				implementation->Response(id, Core::emptyString, convert.Error.Code.Value());
			}
			else if (convert.Result.Value().empty() == true) {
				implementation->Response(id, string(Core::JSON::IElement::NullTag), 0);
			}
			else {
				implementation->Response(id, convert.Result.Value(), 0);
			}
		}
	}
}
