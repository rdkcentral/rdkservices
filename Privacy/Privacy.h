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

#include "Module.h"

#include <interfaces/IPrivacy.h>
#include <interfaces/json/JsonData_Privacy.h>
#include <interfaces/json/JPrivacy.h>
#include "UtilsLogging.h"


namespace WPEFramework {

    namespace Plugin {


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
        class Privacy : public PluginHost::IPlugin, public PluginHost::JSONRPC {
        private:
            // We do not allow this plugin to be copied !!
            Privacy(const Privacy&) = delete;
            Privacy& operator=(const Privacy&) = delete;

        public:
            Privacy();
            virtual ~Privacy();
            virtual const string Initialize(PluginHost::IShell* shell) override;
            virtual void Deinitialize(PluginHost::IShell* service) override;
            virtual string Information() const override { return {}; }

            BEGIN_INTERFACE_MAP(Privacy)
            INTERFACE_ENTRY(PluginHost::IPlugin)
            INTERFACE_ENTRY(PluginHost::IDispatcher)
            INTERFACE_AGGREGATE(Exchange::IPrivacy, mPrivacy)
            END_INTERFACE_MAP

        private:
            class Notification : public RPC::IRemoteConnection::INotification,
                                 public Exchange::IPrivacy::INotification
            {
            private:
                Notification() = delete;
                Notification(const Notification &) = delete;
                Notification &operator=(const Notification &) = delete;

            public:
                explicit Notification(Privacy *parent) : mParent(*parent)
                {
                    ASSERT(parent != nullptr);
                }

                virtual ~Notification() override
                {
                }

                BEGIN_INTERFACE_MAP(Notification)
                INTERFACE_ENTRY(RPC::IRemoteConnection::INotification)
                INTERFACE_ENTRY(Exchange::IPrivacy::INotification)
                END_INTERFACE_MAP

                void Activated(RPC::IRemoteConnection *) override
                {
                    LOGINFO("Privacy Notification Activated");
                }

                void Deactivated(RPC::IRemoteConnection *connection) override
                {
                    LOGINFO("Privacy Notification Deactivated");
                    mParent.Deactivated(connection);
                }

                void OnAllowACRCollectionChanged(const bool allow) override
                {
                    LOGINFO("OnAllowACRCollectionChanged: %d\n", allow);
                    Exchange::JPrivacy::Event::OnAllowACRCollectionChanged(mParent, allow);
                }

                void OnAllowAppContentAdTargetingChanged(const bool allow) override
                {
                    LOGINFO("OnAllowAppContentAdTargetingChanged: %d\n", allow);
                    Exchange::JPrivacy::Event::OnAllowAppContentAdTargetingChanged(mParent, allow);
                }

                void OnAllowCameraAnalyticsChanged(const bool allow) override
                {
                    LOGINFO("OnAllowCameraAnalyticsChanged: %d\n", allow);
                    Exchange::JPrivacy::Event::OnAllowCameraAnalyticsChanged(mParent, allow);
                }

                void OnAllowPersonalizationChanged(const bool allow) override
                {
                    LOGINFO("OnAllowPersonalizationChanged: %d\n", allow);
                    Exchange::JPrivacy::Event::OnAllowPersonalizationChanged(mParent, allow);
                }

                void OnAllowPrimaryBrowseAdTargetingChanged(const bool allow) override
                {
                    LOGINFO("OnAllowPrimaryBrowseAdTargetingChanged: %d\n", allow);
                    Exchange::JPrivacy::Event::OnAllowPrimaryBrowseAdTargetingChanged(mParent, allow);
                }

                void OnAllowPrimaryContentAdTargetingChanged(const bool allow) override
                {
                    LOGINFO("OnAllowPrimaryContentAdTargetingChanged: %d\n", allow);
                    Exchange::JPrivacy::Event::OnAllowPrimaryContentAdTargetingChanged(mParent, allow);
                }

                void OnAllowProductAnalyticsChanged(const bool allow) override
                {
                    LOGINFO("OnAllowProductAnalyticsChanged: %d\n", allow);
                    Exchange::JPrivacy::Event::OnAllowProductAnalyticsChanged(mParent, allow);
                }

                void OnAllowRemoteDiagnosticsChanged(const bool allow) override
                {
                    LOGINFO("OnAllowRemoteDiagnosticsChanged: %d\n", allow);
                    Exchange::JPrivacy::Event::OnAllowRemoteDiagnosticsChanged(mParent, allow);
                }

                void OnAllowResumePointsChanged(const bool allow) override
                {
                    LOGINFO("OnAllowResumePointsChanged: %d\n", allow);
                    Exchange::JPrivacy::Event::OnAllowResumePointsChanged(mParent, allow);
                }

                void OnAllowUnentitledPersonalizationChanged(const bool allow) override
                {
                    LOGINFO("OnAllowUnentitledPersonalizationChanged: %d\n", allow);
                    Exchange::JPrivacy::Event::OnAllowUnentitledPersonalizationChanged(mParent, allow);
                }

                void OnAllowUnentitledResumePointsChanged(const bool allow) override
                {
                    LOGINFO("OnAllowUnentitledResumePointsChanged: %d\n", allow);
                    Exchange::JPrivacy::Event::OnAllowUnentitledResumePointsChanged(mParent, allow);
                }

                void OnAllowWatchHistoryChanged(const bool allow) override
                {
                    LOGINFO("OnAllowWatchHistoryChanged: %d\n", allow);
                    Exchange::JPrivacy::Event::OnAllowWatchHistoryChanged(mParent, allow);
                }

                void OnAllowContinueWatchingOptOutChanged(const bool allow) override
                {
                    LOGINFO("OnAllowContinueWatchingOptOutChanged: %d\n", allow);
                    Exchange::JPrivacy::Event::OnAllowContinueWatchingOptOutChanged(mParent, allow);
                }

                void OnAllowPersonalisedRecsOptOutChanged(const bool allow) override
                {
                    LOGINFO("OnAllowPersonalisedRecsOptOutChanged: %d\n", allow);
                    Exchange::JPrivacy::Event::OnAllowPersonalisedRecsOptOutChanged(mParent, allow);
                }

                void OnAllowProductAnalyticsOptOutChanged(const bool allow) override
                {
                    LOGINFO("OnAllowProductAnalyticsOptOutChanged: %d\n", allow);
                    Exchange::JPrivacy::Event::OnAllowProductAnalyticsOptOutChanged(mParent, allow);
                }

            private:
                Privacy &mParent;
            };

            void Deactivated(RPC::IRemoteConnection* connection);

        private:
            PluginHost::IShell* mService;
            uint32_t mConnectionId;
            Exchange::IPrivacy* mPrivacy;
            Core::Sink<Notification> mNotification;
        };
	} // namespace Plugin
} // namespace WPEFramework
