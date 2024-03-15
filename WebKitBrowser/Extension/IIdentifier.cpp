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

#include "Module.h"
#include <plugins/plugins.h>

#include "IIdentifier.h"

namespace WPEFramework {
namespace JavaScript {
namespace IIdentifier {

void InjectJS(WebKitScriptWorld* world, WebKitFrame* frame, Core::ProxyType<RPC::CommunicatorClient>& _comClient)
{
    if (webkit_frame_is_main_frame(frame) == false)
        return;

    JSCContext* jsContext = webkit_frame_get_js_context_for_script_world(frame, world);

    using IIdentifierData = std::tuple<PluginHost::IShell*, const PluginHost::ISubSystem*, const PluginHost::ISubSystem::IIdentifier*>;

    // Register a custom class
    JSCClass* jscClass = jsc_context_register_class(
        jsContext,
        "IIdentifier",
        nullptr,
        nullptr,
        GDestroyNotify(+[](gpointer userData) -> void {
            IIdentifierData& identifierData = *static_cast<IIdentifierData*>(userData);

            PluginHost::IShell* controller = std::get<0>(identifierData);
            ASSERT(controller != nullptr);
            const PluginHost::ISubSystem* subsysInterface = std::get<1>(identifierData);
            ASSERT(subsysInterface != nullptr);
            const PluginHost::ISubSystem::IIdentifier* identifierInterface = std::get<2>(identifierData);
            ASSERT(identifierInterface != nullptr);

            identifierInterface->Release();
            subsysInterface->Release();
            controller->Release();

            delete static_cast<IIdentifierData*>(userData);
        }));

    // Add constructor for the custom class
    JSCValue* constructor = jsc_class_add_constructor(
        jscClass,
        nullptr,
        GCallback(+[](gpointer userData) -> gpointer {
            Core::ProxyType<RPC::CommunicatorClient>& comClient = *static_cast<Core::ProxyType<RPC::CommunicatorClient>*>(userData);
            ASSERT(comClient != nullptr);

            PluginHost::IShell* controller = comClient->Acquire<PluginHost::IShell>(10000, _T("Controller"), ~0);
            ASSERT(controller != nullptr);
            const PluginHost::ISubSystem* subsysInterface = controller->SubSystems();
            ASSERT(subsysInterface != nullptr);
            const PluginHost::ISubSystem::IIdentifier* identifierInterface = subsysInterface->Get<PluginHost::ISubSystem::IIdentifier>();

            auto* identifierData = new IIdentifierData(controller, subsysInterface, identifierInterface);
            return identifierData;
        }),
        (gpointer)&_comClient,
        nullptr,
        G_TYPE_POINTER,
        0,
        G_TYPE_NONE);
    jsc_context_set_value(jsContext, jsc_class_get_name(jscClass), constructor);
    g_object_unref(constructor);

    // Add methods for the custom class
    jsc_class_add_method(
        jscClass,
        "Identifier",
        G_CALLBACK(+[](gpointer userData) -> char* {
            IIdentifierData& identifierData = *static_cast<IIdentifierData*>(userData);
            const PluginHost::ISubSystem::IIdentifier* identifierInterface = std::get<2>(identifierData);
            ASSERT(identifierInterface != nullptr);

            uint8_t buffer[64] = {};
            buffer[0] = identifierInterface->Identifier(sizeof(buffer) - 1, &(buffer[1]));
            string identifier = Core::SystemInfo::Instance().Id(buffer, ~0);
            if (identifier.length() > 0) {
                return g_strndup(reinterpret_cast<const char*>(identifier.c_str()), identifier.length());
            } else {
                return nullptr;
            }
        }),
        (gpointer)&_comClient,
        nullptr,
        G_TYPE_STRING,
        0,
        G_TYPE_NONE);

    jsc_class_add_method(
        jscClass,
        "Architecture",
        G_CALLBACK(+[](gpointer userData) -> char* {
            IIdentifierData& identifierData = *static_cast<IIdentifierData*>(userData);
            const PluginHost::ISubSystem::IIdentifier* identifierInterface = std::get<2>(identifierData);
            ASSERT(identifierInterface != nullptr);

            string architecture = identifierInterface->Architecture();
            if (architecture.length() > 0) {
                return g_strndup(reinterpret_cast<const char*>(architecture.c_str()), architecture.length());
            } else {
                return nullptr;
            }
        }),
        (gpointer)&_comClient,
        nullptr,
        G_TYPE_STRING,
        0,
        G_TYPE_NONE);

    jsc_class_add_method(
        jscClass,
        "Chipset",
        G_CALLBACK(+[](gpointer userData) -> char* {
            IIdentifierData& identifierData = *static_cast<IIdentifierData*>(userData);
            const PluginHost::ISubSystem::IIdentifier* identifierInterface = std::get<2>(identifierData);
            ASSERT(identifierInterface != nullptr);

            string chipset = identifierInterface->Chipset();
            if (chipset.length() > 0) {
                return g_strndup(reinterpret_cast<const char*>(chipset.c_str()), chipset.length());
            } else {
                return nullptr;
            }
        }),
        (gpointer)&_comClient,
        nullptr,
        G_TYPE_STRING,
        0,
        G_TYPE_NONE);

    jsc_class_add_method(
        jscClass,
        "FirmwareVersion",
        G_CALLBACK(+[](gpointer userData) -> char* {
            IIdentifierData& identifierData = *static_cast<IIdentifierData*>(userData);
            const PluginHost::ISubSystem::IIdentifier* identifierInterface = std::get<2>(identifierData);
            ASSERT(identifierInterface != nullptr);

            string firmwareVersion = identifierInterface->FirmwareVersion();
            if (firmwareVersion.length() > 0) {
                return g_strndup(reinterpret_cast<const char*>(firmwareVersion.c_str()), firmwareVersion.length());
            } else {
                return nullptr;
            }
        }),
        (gpointer)&_comClient,
        nullptr,
        G_TYPE_STRING,
        0,
        G_TYPE_NONE);

    g_object_unref(jsContext);
}

}  // IIdentifier
}  // JavaScript
}  // WPEFramework
