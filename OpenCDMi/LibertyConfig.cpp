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

#include "LibertyConfig.h"
#include <rdk_debug.h>

namespace Lgi {
WidevineConfig::WidevineConfig()
    : WPEFramework::Core::JSON::Container()
    , Certificate()
    , Product()
    , Company()
    , Model()
    , Device()
    , BuildInfo()
    , CertScope()
{
    Add(_T("certificate"), &Certificate);
    Add(_T("product"), &Product);
    Add(_T("company"), &Company);
    Add(_T("model"), &Model);
    Add(_T("device"), &Device);
    Add(_T("buildinfo"), &BuildInfo);
    Add(_T("certscope"), &CertScope);
}

ASConfig::ASConfig() {
    rdk_logger_init("/etc/debug.ini");
    const std::string asURI = "ws://127.0.0.1:10415";
    _ASConnector = std::make_shared<ASConnector>(asURI, *this);
    _ASConnector->WatchAPIConfig("Company", "apps.youtube.brandName", "");
    _ASConnector->WatchAPIConfig("Model", "cpe.modelName", "");
    _ASConnector->WatchAPIConfig("BuildInfo", "cpe.firmwareVersion", "");
    _ASConnector->Connect();
}

std::string ASConfig::getCompany()
{
    std::string ret;
    {
        std::unique_lock<std::mutex> lck(_ConfigMtx);
        if(_Company.empty()) {
            _ASConnCv.wait_for(lck, std::chrono::seconds(5), [this](){return !_Company.empty();});
        }
        ret = _Company;
    }
    return ret;
}

std::string ASConfig::getModel()
{
    std::string ret;
    {
        std::unique_lock<std::mutex> lck(_ConfigMtx);
        if(_Model.empty()) {
            _ASConnCv.wait_for(lck, std::chrono::seconds(5), [this](){return !_Model.empty();});
        }
        ret = _Model;
    }
    return ret;
}

std::string ASConfig::getBuildInfo()
{
    std::string ret;
    {
        std::unique_lock<std::mutex> lck(_ConfigMtx);
        if(_BuildInfo.empty()) {
            _ASConnCv.wait_for(lck, std::chrono::seconds(5), [this](){return !_BuildInfo.empty();});
        }
        ret = _BuildInfo;
    }
    return ret;
}

void ASConfig::OnWatchData(ASConnector::DataSource src, const std::string& alias, json_t* data)
{
    if(alias == "Company") {
        if (!json_is_string(data)) {
            return;
        }
        std::lock_guard<std::mutex> lck(_ConfigMtx);
        _Company = json_string_value(data);
    } else if(alias == "Model") {
        if (!json_is_string(data)) {
            return;
        }
        std::lock_guard<std::mutex> lck(_ConfigMtx);
        _Model = json_string_value(data);
    } else if(alias == "BuildInfo") {
        if (!json_is_string(data)) {
            return;
        }
        std::lock_guard<std::mutex> lck(_ConfigMtx);
        _BuildInfo = json_string_value(data);
    }
    _ASConnCv.notify_all();
}

std::string updateWidevineConfig(const std::string& widevineConfigStr, ASConfig& asConfig)
{
    std::string ret;
    WidevineConfig widevineConfig;
    widevineConfig.FromString(widevineConfigStr);
    widevineConfig.BuildInfo = asConfig.getBuildInfo();
    widevineConfig.Model = asConfig.getModel();
    widevineConfig.Company = asConfig.getCompany();
    auto certScope = getenv("COBALT_CERT_SCOPE");
    if(certScope) {
        widevineConfig.CertScope = certScope;
    }
    widevineConfig.ToString(ret);
    return ret;
}
}
