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

#pragma once

#include <mutex>
#include <vector>

#include "Module.h"

#if defined(PLATFORM_AMLOGIC)
#include <interfaces/ICapture.h>
#endif

namespace WPEFramework {

    namespace Plugin {

        class ScreenCapture;

#if defined(PLATFORM_AMLOGIC)
        struct ScreenCaptureStore: public Exchange::ICapture::IStore {
            ScreenCaptureStore(WPEFramework::Plugin::ScreenCapture* sc) : m_screenCapture(sc) {}
            ScreenCaptureStore(const ScreenCaptureStore& copy) : m_screenCapture(copy.m_screenCapture) { }
            ~ScreenCaptureStore() {}
            virtual bool R8_G8_B8_A8(const unsigned char* buffer, const unsigned int width, const unsigned int height) override;
        private:
            ScreenCaptureStore() = delete;
            WPEFramework::Plugin::ScreenCapture* m_screenCapture;
        };
#else
        class ScreenShotJob
        {
        private:
            ScreenShotJob() = delete;
            ScreenShotJob& operator=(const ScreenShotJob& RHS) = delete;

        public:
            ScreenShotJob(WPEFramework::Plugin::ScreenCapture* tpt) : m_screenCapture(tpt) { }
            ScreenShotJob(const ScreenShotJob& copy) : m_screenCapture(copy.m_screenCapture) { }
            ~ScreenShotJob() {}

            inline bool operator==(const ScreenShotJob& RHS) const
            {
                return(m_screenCapture == RHS.m_screenCapture);
            }

        public:
            uint64_t Timed(const uint64_t scheduledTime);

        private:
            WPEFramework::Plugin::ScreenCapture* m_screenCapture;
        };
#endif
        // This is a server for a JSONRPC communication channel.
        // For a plugin to be capable to handle JSONRPC, inherit from PluginHost::JSONRPC.
        // By inheriting from this class, the plugin realizes the interface PluginHost::IDispatcher.
        // This realization of this interface implements, by default, the following methods on this plugin
        // - exists
        // - register
        // - unregister
        // Any other methood to be handled by this plugin  can be added can be added by using the
        // templated methods Register on the PluginHost::JSONRPC class.
        // As the registration/unregistration of notifications is realized by the class PluginHost::JSONRPC,
        // this class exposes a public method called, Notify(), using this methods, all subscribed clients
        // will receive a JSONRPC message as a notification, in case this method is called.
        class ScreenCapture : public PluginHost::IPlugin, public PluginHost::JSONRPC {
        private:

            // We do not allow this plugin to be copied !!
            ScreenCapture(const ScreenCapture&) = delete;
            ScreenCapture& operator=(const ScreenCapture&) = delete;
         
            //Begin methods
            uint32_t uploadScreenCapture(const JsonObject& parameters, JsonObject& response);
            //End methods

            #ifdef PLATFORM_BROADCOM
            bool getScreenshotNexus(std::vector<unsigned char> &png_data);
            bool joinNexus();
            #endif

            #ifdef PLATFORM_INTEL
            bool getScreenshotIntel(std::vector<unsigned char> &png_data);
            #endif

            #ifdef HAS_FRAMEBUFFER_API_HEADER
            bool getScreenshotRealtek(std::vector<unsigned char> &png_data);
            #endif

            bool saveToPng(unsigned char *bytes, int w, int h, std::vector<unsigned char> &png_out_data);
            bool uploadDataToUrl(const std::vector<unsigned char> &data, const char *url, std::string &error_str);
            bool getScreenShot();
            bool doUploadScreenCapture(const std::vector<unsigned char> &png_data, bool got_screenshot);

        public:
            ScreenCapture();
            virtual ~ScreenCapture();
            virtual const string Initialize(PluginHost::IShell*) override;
            virtual void Deinitialize(PluginHost::IShell* service) override;
            virtual string Information() const override { return {}; }

            BEGIN_INTERFACE_MAP(ScreenCapture)
            INTERFACE_ENTRY(PluginHost::IPlugin)
            INTERFACE_ENTRY(PluginHost::IDispatcher)
            END_INTERFACE_MAP

#if defined(PLATFORM_AMLOGIC)
            void onScreenCaptureData(const unsigned char* buffer, const unsigned int width, const unsigned int height);
#endif
        private:
            std::mutex m_callMutex;

#if defined(PLATFORM_AMLOGIC)
            PluginHost::IPlugin *m_RDKShellRef;
            Exchange::ICapture *m_captureRef;
            ScreenCaptureStore m_screenCaptureStore;
#else
            WPEFramework::Core::TimerType<ScreenShotJob> *screenShotDispatcher;
#endif
            std::string url;
            std::string callGUID;

            #ifdef PLATFORM_BROADCOM
            bool inNexus;
            #endif

            friend class ScreenShotJob;
        };

    } // namespace Plugin
} // namespace WPEFramework
