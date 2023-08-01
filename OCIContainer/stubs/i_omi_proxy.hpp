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

#ifndef I_OMI_PROXY_HPP_
#define I_OMI_PROXY_HPP_

#include <string>
#include <functional>

namespace omi
{

/**
 *  @interface IOmiProxy
 *  @brief Wrapper around an omi_dbus_api that provides simpler method
 *  calls and give possibility to register/unregister for incoming signals.
 */
class IOmiProxy
{
public:
    enum class ErrorType { verityFailed };

    typedef std::function<void(const std::string&, ErrorType, const void*)> OmiErrorListener;

    // Mount crypted bundle
    // id               [IN]  - Container ID in reverse domain name notation
    // rootfs_file_path [IN]  - Absolute pathname for filesystem image
    // config_json_path [IN]  - Absolute pathname for config.json.jwt
    // bundlePath       [OUT] - Absolute pathname for decrypted config.json payload
    // Returns TRUE on success, FALSE on error
    virtual bool mountCryptedBundle(const std::string& id,
                                    const std::string& rootfs_file_path,
                                    const std::string& config_json_path,
                                    std::string& bundlePath /*out parameter*/) = 0;

    // Unmount crypted bundle
    // id               [IN]  - Container ID in reverse domain name notation
    // Returns TRUE on success, FALSE on error
    virtual bool umountCryptedBundle(const std::string& id) = 0;

    // Register listener
    // listener         [IN]  - OMI error listener which will be called on error event occurrence
    // Returns listener ID (0<) or 0 on error
    virtual long unsigned registerListener(const OmiErrorListener &listener, const void* cbParams) = 0;

    // Unregister listener
    // tag           [IN]  - ID which defines listener to be unregistered
    virtual void unregisterListener(long unsigned tag) = 0;

};
} // namespace omi
#endif // #ifndef I_OMI_PROXY_HPP_
