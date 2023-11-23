/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2022 RDK Management
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

#include "SqliteSeeDbPlabels.h"

#include "pbnj_utils.hpp"
#include <glib.h>

namespace WPEFramework {
namespace Plugin {

    uint32_t SqliteSeeDbPlabels::GenerateKey(std::vector<uint8_t>& key) const
    {
        uint32_t result;

        // NOTE: pbnj_utils stores the nonce under XDG_DATA_HOME/data.
        // If the dir doesn't exist it will fail

        auto path = g_build_filename(g_get_user_data_dir(), "data", nullptr);
        if (!Core::File(string(path)).Exists())
            g_mkdir_with_parents(path, 0755);
        g_free(path);

        if (pbnj_utils::prepareBufferForOrigin(_key.c_str(), [&pKey](const std::vector<uint8_t>& buffer) {
                pKey = buffer;
            })
            == true) {
            TRACE(Trace::Error, (_T("pbnj_utils::prepareBufferForOrigin returned false")));
            result = Core::ERROR_GENERAL;
        } else {
            result = Core::ERROR_NONE;
        }

        return result;
    }

} // namespace Plugin
} // namespace WPEFramework
