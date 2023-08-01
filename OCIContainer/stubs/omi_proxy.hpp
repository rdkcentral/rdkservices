/**
* If not stated otherwise in this file or this component's LICENSE
* file the following copyright and licenses apply:
*
* Copyright 2021 RDK Management
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

#ifndef OMI_PROXY_HPP_
#define OMI_PROXY_HPP_

#include <i_omi_proxy.hpp>

namespace omi
{

class OmiProxy : public IOmiProxy
{
public:
    OmiProxy() = default;
    virtual ~OmiProxy() = default;

    virtual bool mountCryptedBundle(const std::string& id,
                                       const std::string& rootfs_file_path,
                                       const std::string& config_json_path,
                                       std::string& bundlePath) { return true; }

    virtual bool umountCryptedBundle(const std::string& id) { return true; }

    virtual long unsigned registerListener(const OmiErrorListener &listener, const void* cbParams) { return 0; }

    virtual void unregisterListener(long unsigned tag) {}
};

} // namespace omi

#endif // OMI_PROXY_HPP_
