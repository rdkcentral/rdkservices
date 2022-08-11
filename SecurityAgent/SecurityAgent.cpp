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
 
#include "SecurityAgent.h"
#include "SecurityContext.h"
#include "TokenFactory.h"

#define API_VERSION_NUMBER_MAJOR 1
#define API_VERSION_NUMBER_MINOR 0
#define API_VERSION_NUMBER_PATCH 0

namespace WPEFramework {

namespace {

    static Plugin::Metadata<Plugin::SecurityAgent> metadata(
        // Version (Major, Minor, Patch)
        API_VERSION_NUMBER_MAJOR, API_VERSION_NUMBER_MINOR, API_VERSION_NUMBER_PATCH,
        // Preconditions
        {},
        // Terminations
        {},
        // Controls
        {}
    );
}

namespace Plugin {

    SERVICE_REGISTRATION(SecurityAgent, API_VERSION_NUMBER_MAJOR, API_VERSION_NUMBER_MINOR, API_VERSION_NUMBER_PATCH);

    static Core::ProxyPoolType<Web::TextBody> textFactory(1);

    class SecurityCallsign : public PluginHost::ISubSystem::ISecurity {
    public:
        SecurityCallsign() = delete;
        SecurityCallsign(const SecurityCallsign&) = delete;
        SecurityCallsign& operator=(const SecurityCallsign&) = delete;

        SecurityCallsign(const string callsign)
            : _callsign(callsign)
        {
        }
        virtual ~SecurityCallsign()
        {
        }

    public:
        // Security information
        virtual string Callsign() const
        {
            return (_callsign);
        }

    private:
        BEGIN_INTERFACE_MAP(SecurityCallsign)
        INTERFACE_ENTRY(PluginHost::ISubSystem::ISecurity)
        END_INTERFACE_MAP

    private:
        const string _callsign;
    };

    SecurityAgent::SecurityAgent()
        : _acl()
        , _dispatcher(nullptr)
        , _engine()
    {
        RegisterAll();
    }

    /* virtual */ SecurityAgent::~SecurityAgent()
    {
        UnregisterAll();
    }

    /* virtual */ const string SecurityAgent::Initialize(PluginHost::IShell* service)
    {
        Config config;
        config.FromString(service->ConfigLine());
        string version = service->Version();

        _skipURL = static_cast<uint8_t>(service->WebPrefix().length());
        Core::File aclFile(service->PersistentPath() + config.ACL.Value(), true);

        PluginHost::ISubSystem* subSystem = service->SubSystems();

        if (aclFile.Exists() == false) {
            aclFile = service->DataPath() + config.ACL.Value();
        }
        if ((aclFile.Exists() == true) && (aclFile.Open(true) == true)) {

            if (_acl.Load(aclFile) == Core::ERROR_INCOMPLETE_CONFIG) {
                AccessControlList::Iterator index(_acl.Unreferenced());
                while (index.Next()) {
                    SYSLOG(Logging::Startup, (_T("Role: %s not referenced"), index.Current().c_str()));
                }
                index = _acl.Undefined();
                while (index.Next()) {
                    SYSLOG(Logging::Startup, (_T("Role: %s is undefined"), index.Current().c_str()));
                }
            }
        }

        ASSERT(_dispatcher == nullptr);
        ASSERT(subSystem != nullptr);

        string connector = config.Connector.Value();

        if (connector.empty() == true) {
            connector = service->VolatilePath() + _T("token");
        }
        _engine = Core::ProxyType<RPC::InvokeServer>::Create(&Core::IWorkerPool::Instance());
        _dispatcher.reset(new TokenDispatcher(Core::NodeId(connector.c_str()), service->ProxyStubPath(), this, _engine));

        if (_dispatcher != nullptr) {

            if (_dispatcher->IsListening() == false) {
                _dispatcher.reset(nullptr);
                _engine.Release();
            } else {
                if (subSystem != nullptr) {
                    Core::SystemInfo::SetEnvironment(_T("SECURITYAGENT_PATH"), _dispatcher->Connector().c_str(), true);
                    Core::Sink<SecurityCallsign> information(service->Callsign());

                    if (subSystem->IsActive(PluginHost::ISubSystem::SECURITY) != false) {
                        SYSLOG(Logging::Startup, (_T("Security is not defined as External !!")));
                    }

                    subSystem->Set(PluginHost::ISubSystem::SECURITY, &information);
                    subSystem->Release();
                }
            }
        }

        // On success return empty, to indicate there is no error text.
        return _T("");
    }

    /* virtual */ void SecurityAgent::Deinitialize(PluginHost::IShell* service)
    {
        PluginHost::ISubSystem* subSystem = service->SubSystems();

        ASSERT(subSystem != nullptr);

        if (subSystem != nullptr) {
            subSystem->Set(PluginHost::ISubSystem::NOT_SECURITY, nullptr);
            subSystem->Release();
        }

        _dispatcher.reset();
        _engine.Release();

        _acl.Clear();
    }

    /* virtual */ string SecurityAgent::Information() const
    {
        // No additional info to report.
        return (string());
    }

    /* virtual */ uint32_t SecurityAgent::CreateToken(const uint16_t length, const uint8_t buffer[], string& token)
    {
        // Generate the token from the buffer coming in...
        auto newToken = JWTFactory::Instance().Element();

        return (newToken->Encode(token, length, buffer) > 0 ? Core::ERROR_NONE : Core::ERROR_UNAVAILABLE);
    }

    /* virtual */ PluginHost::ISecurity* SecurityAgent::Officer(const string& token)
    {
        PluginHost::ISecurity* result = nullptr;

        auto webToken = JWTFactory::Instance().Element();
        uint16_t load = webToken->PayloadLength(token);

        // Validate the token
        if (load != static_cast<uint16_t>(~0)) {
            // It is potentially a valid token, extract the payload.
            uint8_t* payload = reinterpret_cast<uint8_t*>(ALLOCA(load));

            load = webToken->Decode(token, load, payload);

            if (load != static_cast<uint16_t>(~0)) {
                // Seems like we extracted a valid payload, time to create an security context
                result = Core::Service<SecurityContext>::Create<SecurityContext>(&_acl, load, payload);
            }
        }
        return (result);
    }

    /* virtual */ void SecurityAgent::Inbound(Web::Request& request)
    {
        request.Body(textFactory.Element());
    }

    /* virtual */ Core::ProxyType<Web::Response> SecurityAgent::Process(const Web::Request& request)
    {
        Core::ProxyType<Web::Response> result(PluginHost::IFactories::Instance().Response());
        Core::TextSegmentIterator index(Core::TextFragment(request.Path, _skipURL, static_cast<uint32_t>(request.Path.length() - _skipURL)), false, '/');

        result->ErrorCode = Web::STATUS_BAD_REQUEST;
        result->Message = "Unknown error";

        index.Next();

		if (index.Next() == true) {
            // We might be receiving a plugin download request.
            #ifdef SECURITY_TESTING_MODE
            if ((request.Verb == Web::Request::HTTP_PUT) && (request.HasBody() == true)) {
                if (index.Current() == _T("Token")) {
                    Core::ProxyType<const Web::TextBody> data(request.Body<Web::TextBody>());

                    if (data.IsValid() == true) {
                        Core::ProxyType<Web::TextBody> token = textFactory.Element();
                        const string& byteBag(static_cast<const string&>(*data));
                        uint32_t code = CreateToken(static_cast<uint16_t>(byteBag.length()), reinterpret_cast<const uint8_t*>(byteBag.c_str()), static_cast<string&>(*token));

                        if (code == Core::ERROR_NONE) {

                            result->Body(token);
                            result->ContentType = Web::MIMETypes::MIME_TEXT;
                            result->ErrorCode = Web::STATUS_OK;
                            result->Message = "Ok";
                        }
                    }
                }
            } else
            #endif      

            if ( (request.Verb == Web::Request::HTTP_GET) && (index.Current() == _T("Valid")) ) {
                result->ErrorCode = Web::STATUS_FORBIDDEN;
                result->Message = _T("Missing token");

                if (request.WebToken.IsSet()) {
                    auto webToken = JWTFactory::Instance().Element();
                    const string& token = request.WebToken.Value().Token();
                    uint16_t load = webToken->PayloadLength(token);

                    // Validate the token
                    if (load != static_cast<uint16_t>(~0)) {
                        // It is potentially a valid token, extract the payload.
                        uint8_t* payload = reinterpret_cast<uint8_t*>(ALLOCA(load));

                        load = webToken->Decode(token, load, payload);

                        if (load == static_cast<uint16_t>(~0)) {
                            result->ErrorCode = Web::STATUS_FORBIDDEN;
                            result->Message = _T("Invalid token");
                        } else {
                            result->ErrorCode = Web::STATUS_OK;
                            result->Message = _T("Valid token");
                            TRACE(Trace::Information, (_T("Token contents: %s"), reinterpret_cast<const TCHAR*>(payload)));
                        }
                    }
                
				}
            }
        }
		return (result);
    }

} // namespace Plugin
} // namespace WPEFramework
