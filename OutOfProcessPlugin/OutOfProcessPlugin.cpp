/**
* If not stated otherwise in this file or this component's LICENSE
* file the following copyright and licenses apply:
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
**/

#include "OutOfProcessPlugin.h"

namespace WPEFramework {
namespace Plugin {

	SERVICE_REGISTRATION(OutOfProcessPlugin, 1, 0);

	/* encapsulated class Thread  */
	/* initialize and deinitialize or activate or deactivate the plugin services */
	const string OutOfProcessPlugin::Initialize(PluginHost::IShell *service) {

        	ASSERT (_service == nullptr);
           	ASSERT (service != nullptr);
	        _service = service;
		_implementation = _service->Root<Exchange::IOutOfProcessPlugin>(_connectionId, 2000, _T("OutOfProcessPluginImplementation"));	
  		return (string());
	}

	void OutOfProcessPlugin::Deinitialize(PluginHost::IShell *service) {
	
	}

	string OutOfProcessPlugin::Information() const {
		// No additional info to report.
  		return (string());
	}

	void OutOfProcessPlugin::Inbound(Web::Request &request) {
	}

	Core::ProxyType<Web::Response> OutOfProcessPlugin::Process(const Web::Request &request) {

  		TRACE(Trace::Information, (string(_T("Received OutOfProcessPlugin request"))));
  		Core::ProxyType<Web::Response> result(PluginHost::IFactories::Instance().Response());
  		result->ErrorCode = Web::STATUS_BAD_REQUEST;
  		result->Message = "Unknown error";
  		return result;
	}

	void OutOfProcessPlugin::Deactivated(RPC::IRemoteConnection *connection) {
  		if (connection->Id() == _connectionId) {
    			ASSERT(_service != nullptr);
    			Core::IWorkerPool::Instance().Submit(
      			PluginHost::IShell::Job::Create(_service,
        		PluginHost::IShell::DEACTIVATED,
        		PluginHost::IShell::FAILURE));
  		}
	}

}  // namespace Plugin
}  // namespace WPEFramework
