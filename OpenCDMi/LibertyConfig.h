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

#pragma once

#include <condition_variable>
#include <mutex>

#include "Module.h"

#include <asconnector/asconnector.h>
#include <core/core.h>


namespace Lgi {
using JSONString = WPEFramework::Core::JSON::String;

class WidevineConfig : public WPEFramework::Core::JSON::Container {
public:
    WidevineConfig(const WidevineConfig&) = delete;
    WidevineConfig& operator=(const WidevineConfig&) = delete;
    WidevineConfig();
    ~WidevineConfig() = default;

public:
    JSONString Certificate;
    JSONString Product;
    JSONString Company;
    JSONString Model;
    JSONString Device;
    JSONString BuildInfo;
    JSONString CertScope;
};

class ASConfig: public ASConnector::events {
  public:
    ASConfig();
    std::string getCompany();
    std::string getModel();
    std::string getBuildInfo();

  private:
    void OnWatchData(ASConnector::DataSource src, const std::string& alias, json_t* data) override final;

    std::shared_ptr<ASConnector> _ASConnector;
    std::mutex _ConfigMtx;
    std::condition_variable _ASConnCv;
    std::string _Company;
    std::string _Model;
    std::string _BuildInfo;
};

std::string updateWidevineConfig(const std::string& widevineConfigStr, ASConfig& asConfig);
}
