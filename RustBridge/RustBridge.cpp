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

		namespace {

			static Metadata<RustBridge> metadata(
				// Version
				1, 0, 0,
				// Preconditions
				{},
				// Terminations
				{},
				// Controls
				{}
			);
		}

		class Registration : public Core::JSON::Container {
		public:
			Registration(const Registration&) = delete;
			Registration& operator=(const Registration&) = delete;

			Registration()
				: Core::JSON::Container()
				, Event()
				, Callsign()
			{
				Add(_T("event"), &Event);
				Add(_T("id"), &Callsign);
			}
			~Registration() override = default;

		public:
			Core::JSON::String Event;
			Core::JSON::String Callsign;
		};

		// -------------------------------------------------------------------------------------------------------
		//   IPluginExtended methods
		// -------------------------------------------------------------------------------------------------------
		const string RustBridge::Initialize(PluginHost::IShell* service) /* override */
		{
			string message;

			ASSERT(_module == nullptr);
			ASSERT(_service == nullptr);
			ASSERT(service != nullptr);
			ASSERT(_connectionId == 0);

			// Setup skip URL for right offset.
			_service = service;
			_service->AddRef();

			Config config;
			config.FromString(service->ConfigLine());
			_skipURL = static_cast<uint8_t>(service->WebPrefix().length());
			_callsign = service->Callsign();
			_service = service;
			_timeOut = (config.TimeOut.Value() * Core::Time::TicksPerMillisecond);

			// Register the Process::Notification stuff. The Remote process might die before we get a
			// change to "register" the sink for these events !!! So do it ahead of instantiation.
			_service->Register(&_notification);

			_module = _service->Root<Exchange::IRustBridge>(_connectionId, 2000, _T("RustBridgeImplementation"));

			if (_module == nullptr) {
				message = _T("RustBridge could not be instantiated.");
			}
			else {
				uint32_t result = _module->Configure(_service, &_callback);

				if (result != Core::ERROR_NONE) {
					message = _T("RustBridge could not be configured, error: ") + Core::NumberType<uint32_t>(result).Text();
				}
			}

			if (message.length() != 0) {
				Deinitialize(service);
			}

			// On success return empty, to indicate there is no error text.
			return (message);
		}

		void RustBridge::Deinitialize(PluginHost::IShell* service) /* override */
		{
			ASSERT(_service == _service);

			_service->Unregister(&_notification);

			if (_module != nullptr) {

				RPC::IRemoteConnection* connection(_service->RemoteConnection(_connectionId));

				// Stop processing of the rustbridge:
				VARIABLE_IS_NOT_USED uint32_t result = _module->Release();
				_module = nullptr;

				// It should have been the last reference we are releasing,
				// so it should endup in a DESTRUCTION_SUCCEEDED, if not we
				// are leaking...
				ASSERT(result == Core::ERROR_DESTRUCTION_SUCCEEDED);

				// The process can disappear in the meantime...
				if (connection != nullptr) {
					// But if it did not dissapear in the meantime, forcefully terminate it. Shoot to kill :-)
					connection->Terminate();
					connection->Release();
				}

			}

			_service->Release();
			_service = nullptr;
			_connectionId = 0;
		}

		string RustBridge::Information() const /* override */
		{
			// No additional info to report.
			return (string());
		}

		bool RustBridge::Attach(PluginHost::Channel& channel) /* override */ {
			bool assigned = false;

			// The expectation is that the JavaScript service opens up a connection to us, so we can forward the 
			// incomming requests, to be handled by the Service.
			if (_javascriptService == 0) {
				if (channel.Protocol().find("json") != string::npos) {
					_javascriptService = channel.Id();
					assigned = true;
				}
			}
			return(assigned);
		}

		void RustBridge::Detach(PluginHost::Channel& channel) /* override */ {
			// Hopefull this does not happen as than we are loosing the actual service :-) We could do proper error handling
			// if this happens :-)
			_javascriptService = 0;
		}

		// -------------------------------------------------------------------------------------------------------
		//   IDispatcher methods
		// -------------------------------------------------------------------------------------------------------
		Core::ProxyType<Core::JSONRPC::Message> RustBridge::Invoke(const string& token, const uint32_t channelId, const Core::JSONRPC::Message& inbound) /* override */
		{
			string method;
			Registration info;

			Core::ProxyType<Core::JSONRPC::Message> message(PluginHost::IFactories::Instance().JSONRPC());
			string designator(inbound.Designator.Value());

			if (inbound.Id.IsSet() == true) {
				message->JSONRPC = Core::JSONRPC::Message::DefaultVersion;
				message->Id = inbound.Id.Value();
			}

			switch (Destination(designator, method)) {
			case state::STATE_INCORRECT_HANDLER:
				message->Error.SetError(Core::ERROR_INVALID_DESIGNATOR);
				message->Error.Text = _T("Destined invoke failed.");
				break;
			case state::STATE_INCORRECT_VERSION:
				message->Error.SetError(Core::ERROR_INVALID_SIGNATURE);
				message->Error.Text = _T("Requested version is not supported.");
				break;
			case state::STATE_UNKNOWN_METHOD:
				message->Error.SetError(Core::ERROR_UNKNOWN_KEY);
				message->Error.Text = _T("Unknown method.");
				break;
			case state::STATE_REGISTRATION:
				info.FromString(inbound.Parameters.Value());
				Subscribe(channelId, info.Event.Value(), info.Callsign.Value(), *message);
				break;
			case state::STATE_UNREGISTRATION:
				info.FromString(inbound.Parameters.Value());
				Unsubscribe(channelId, info.Event.Value(), info.Callsign.Value(), *message);
				break;
			case state::STATE_EXISTS:
				message->Result = Core::NumberType<uint32_t>(Core::ERROR_UNKNOWN_KEY).Text();
				break;
			case state::STATE_NONE_EXISTING:
				message->Result = Core::NumberType<uint32_t>(Core::ERROR_NONE).Text();
				break;
			case state::STATE_CUSTOM:
				// Let's on behalf of the request forward it and update 
				uint32_t newId = Core::InterlockedIncrement(_sequenceId);
				Core::Time waitTill = Core::Time::Now() + _timeOut;

				_pendingRequests.emplace(std::piecewise_construct,
					std::forward_as_tuple(newId),
					std::forward_as_tuple(channelId, message->Id.Value(), waitTill));

				TRACE(Trace::Information, (_T("Request: [%d] from [%d], method: [%s]"), newId, channelId, method.c_str()));

				// Time to fire of the request to RUST ->
				_module->Request(newId, token, method, inbound.Parameters.Value());

				// Wait for ID to return, we can not report anything back yet...
				message.Release();

				if (_timeOut != 0) {
					_cleaner.Reschedule(waitTill);
				}

				break;
			}

			return (Core::ProxyType<Core::JSONRPC::Message>(message));
		}

		void RustBridge::Activate(PluginHost::IShell* /* service */) /* override */ {
			// We did what we needed to do in the Intialize.
		}

		void RustBridge::Deactivate() /* override */ {
			// We did what we needed to do in the Deintialize.
		}

		void RustBridge::Close(const uint32_t channelId) /* override */ {

			_adminLock.Lock();

			ObserverMap::iterator index = _observers.begin();

			while (index != _observers.end()) {
				ObserverList::iterator loop(index->second.begin());

				while (loop != index->second.end()) {
					if (loop->Id() != channelId) {
						loop++;
					}
					else {
						loop = index->second.erase(loop);
					}
				}

				index++;
			}

			_adminLock.Unlock();
		}

		// -------------------------------------------------------------------------------------------------------
		//   Private methods
		// -------------------------------------------------------------------------------------------------------
		void RustBridge::Cleanup() {
			// Lets see if there are still any pending request we should report Missing In Action :-)
			Core::Time now(Core::Time::Now());
			Core::Time nextSlot;

			_adminLock.Lock();
			PendingMap::iterator index(_pendingRequests.begin());
			while (index != _pendingRequests.end()) {
				if (now >= index->second.Issued()) {
					// Send and Error to the requester..
					Core::ProxyType<Core::JSONRPC::Message> message(PluginHost::IFactories::Instance().JSONRPC());
					message->Error.SetError(Core::ERROR_TIMEDOUT);
					message->Error.Text = _T("There is no response form the server within time!!!");
					message->Id = index->second.SequenceId();

					TRACE(Trace::Warning, (_T("Got a timeout on channelId [%d] for request [%d]"), index->second.ChannelId(), message->Id.Value()));

					_service->Submit(index->second.ChannelId(), Core::ProxyType<Core::JSON::IElement>(message));
					index = _pendingRequests.erase(index);
				}
				else {
					if ((nextSlot.IsValid() == false) || (nextSlot > index->second.Issued())) {
						nextSlot = index->second.Issued();
					}
					index++;
				}
			}
			_adminLock.Unlock();

			if (nextSlot.IsValid()) {
				_cleaner.Reschedule(nextSlot);
			}
		}

		bool RustBridge::InternalMessage(const string& eventName, const string& parameters) {
			bool result = false;

			if (eventName == _T("registerjsonrpcmethods")) {
				result = true;
				Core::JSON::ArrayType<Core::JSON::String> parameter;
				parameter.FromString(parameters);
				Core::JSON::ArrayType<Core::JSON::String>::Iterator index(parameter.Elements());

				_supportedVersions.clear();

				while (index.Next() == true) {
					string entry = index.Current().Value();
					uint8_t version = Core::JSONRPC::Message::Version(entry);
					string method = Core::JSONRPC::Message::Method(entry);
					VersionMap::iterator placement = _supportedVersions.find(version);

					if (placement == _supportedVersions.end()) {
						auto newEntry = _supportedVersions.emplace(std::piecewise_construct,
							std::forward_as_tuple(version),
							std::forward_as_tuple());

						newEntry.first->second.push_back(method);
					}
					else if (std::find(placement->second.begin(), placement->second.end(), method) == placement->second.end()) {
						// Check if this label does not already exist
						placement->second.push_back(method);
					}
				}
			}

			return (result);
		}

		void RustBridge::Deactivated(RPC::IRemoteConnection* connection)
		{
			// This can potentially be called on a socket thread, so the deactivation (wich in turn kills this object) must be done
			// on a seperate thread. Also make sure this call-stack can be unwound before we are totally destructed.
			if (_connectionId == connection->Id()) {

				ASSERT(_service != nullptr);

				Core::IWorkerPool::Instance().Submit(PluginHost::IShell::Job::Create(_service, PluginHost::IShell::DEACTIVATED, PluginHost::IShell::FAILURE));
			}
		}

		void RustBridge::RustInvoke(const string& context, const string& method, const string& parmeters, string& response, uint32_t& result) {

		}

		void RustBridge::RustEvent(const string& eventName, const string& parameters) {

			// Check for control messages between server and us..
			if (InternalMessage(eventName, parameters) == false) {

				// This is an event, we need event handling..
				_adminLock.Lock();

				ObserverMap::iterator index = _observers.find(eventName);

				if (index != _observers.end()) {
					for (const Observer& entry : index->second) {
						Core::ProxyType<Core::JSONRPC::Message> outbound(PluginHost::IFactories::Instance().JSONRPC());
						outbound->Designator = (entry.Designator().empty() == false ? entry.Designator() + '.' + eventName : eventName);
						outbound->Parameters = parameters;

						_service->Submit(entry.Id(), Core::ProxyType<Core::JSON::IElement>(outbound));
					}
				}

				_adminLock.Unlock();
			}
		}

		void RustBridge::RustResponse(const uint32_t id, const string& response, const uint32_t error) {

			uint32_t requestId, channelId = 0;

			// This is the response to an invoked method, Let's see who should get this repsonse :-)
			_adminLock.Lock();
			PendingMap::iterator index = _pendingRequests.find(id);
			if (index != _pendingRequests.end()) {
				channelId = index->second.ChannelId();
				requestId = index->second.SequenceId();
				_pendingRequests.erase(index);
			}
			_adminLock.Unlock();

			if (channelId != 0) {
				Core::ProxyType<PluginHost::JSONRPCMessage> message = PluginHost::IFactories::Instance().JSONRPC();

				TRACE(Trace::Information, (_T("Response: [%d] to [%d]"), requestId, channelId));

				// Oke, there is someone waiting for a response!
				message->Id = requestId;

				if (response.empty() == true) {
					message->Error.Code = error;
					message->Error.Data = _T("FAILED");
				}
				else {
					message->Result = response;
				}

				_service->Submit(channelId, Core::ProxyType<Core::JSON::IElement>(message));
			}
		}


	} // namespace Plugin
} // namespace WPEFramework
