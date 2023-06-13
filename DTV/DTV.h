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

#include "Module.h"
#include <interfaces/IDTV.h>
#include <interfaces/json/JsonData_DTV.h>
#include "UtilsJsonRpc.h"

namespace WPEFramework
{
   namespace Plugin
   {
      using namespace JsonData::DTV;
      using namespace Exchange;

      class DTV: public PluginHost::IPlugin,
                 public PluginHost::JSONRPC
      {
         private:
            class Notification: public RPC::IRemoteConnection::INotification,
                                public Exchange::IDTV::INotification
            {
               private:
                  Notification() = delete;
                  Notification(const Notification&) = delete;
                  Notification& operator=(const Notification&) = delete;

               public:
                  explicit Notification(DTV *parent) : m_parent(*parent)
                  {
                      ASSERT(parent != nullptr);
                  }

                  virtual ~Notification() override
                  {
                  }

                  BEGIN_INTERFACE_MAP (Notification)
                     INTERFACE_ENTRY (Exchange::IDTV::INotification)
                     INTERFACE_ENTRY (RPC::IRemoteConnection::INotification)
                  END_INTERFACE_MAP

                  void Activated(RPC::IRemoteConnection*) override
                  {
                  }

                  void Deactivated(RPC::IRemoteConnection *connection) override
                  {
                     m_parent.Deactivated(connection);
                  }

                  void SearchEvent(const uint8_t handle, const bool finished, const uint8_t progress) override
                  {
                     SYSLOG(Logging::Notification, (_T("Notification::SearchEvent: handle=%u, finished=%u, progress=%u"),
                        handle, finished, progress));

                     Core::JSON::EnumType<EventtypeType> json_event_type = EventtypeType::SERVICESEARCHSTATUS;

                     JsonObject params;

                     params["eventtype"] = json_event_type.Data();
                     params["handle"] = handle;
                     params["finished"] = finished;
                     params["progress"] = progress;

                     m_parent.Notify(_T("searchstatus"), params);

                     std::string json;
                     params.ToString(json);

                     m_parent.m_service->Notify(json.c_str());
                  }

                  void ServiceEvent(const ServiceEventType event_type, const IDTV::IService *service,
                     const IDTV::IEitEvent *eit_event) override
                  {
                     uint16_t lcn;
                     std::string dvburi;

                     service->Lcn(lcn);
                     service->Dvburi(dvburi);

                     SYSLOG(Logging::Notification, (_T("Notification::ServiceEvent: lcn=%u, dvburi=%s"),
                        lcn, dvburi.c_str()));

                     ServiceupdatedParamsInfo service_params;
                     Core::JSON::EnumType<EventtypeType> json_event_type;

                     switch(event_type)
                     {
                        case IDTV::INotification::ServiceEventType::SERVICE_UPDATED:
                           json_event_type = EventtypeType::SERVICEUPDATED;
                           break;
                        case IDTV::INotification::ServiceEventType::SERVICE_ADDED:
                           json_event_type = EventtypeType::SERVICEADDED;
                           break;
                        case IDTV::INotification::ServiceEventType::SERVICE_DELETED:
                           json_event_type = EventtypeType::SERVICEDELETED;
                           break;
                        case IDTV::INotification::ServiceEventType::VIDEO_CHANGED:
                           json_event_type = EventtypeType::VIDEOCHANGED;
                           break;
                        case IDTV::INotification::ServiceEventType::AUDIO_CHANGED:
                           json_event_type = EventtypeType::AUDIOCHANGED;
                           break;
                        case IDTV::INotification::ServiceEventType::SUBTITLES_CHANGED:
                           json_event_type = EventtypeType::SUBTITLESCHANGED;
                           break;
                        case IDTV::INotification::ServiceEventType::NOW_EVENT_CHANGED:
                           json_event_type = EventtypeType::EVENTCHANGED;
                           break;
                     }

                     JsonObject params;

                     params["eventtype"] = json_event_type.Data();
                     params["service"] = m_parent.CreateJsonForService(service);

                     if (eit_event != nullptr)
                     {
                        params["event"] = m_parent.CreateJsonForEITEvent(eit_event);
                     }

                     m_parent.Notify(_T("serviceupdated"), params);

                     std::string json;
                     params.ToString(json);

                     m_parent.m_service->Notify(json.c_str());
                  }

               private:
                  DTV& m_parent;
            };

         public:
            DTV(const DTV&) = delete;
            DTV& operator=(const DTV&) = delete;

            DTV();
            ~DTV() override;

            BEGIN_INTERFACE_MAP (DTV)
               INTERFACE_ENTRY (PluginHost::IPlugin)
               INTERFACE_ENTRY (PluginHost::IDispatcher)
               INTERFACE_AGGREGATE(Exchange::IDTV, m_dtv)
            END_INTERFACE_MAP

            //  IPlugin methods
            // -------------------------------------------------------------------------------------------------------
            const string Initialize(PluginHost::IShell* service) override;
            void Deinitialize(PluginHost::IShell *service) override;
            string Information() const override;

         private:
            void Deactivated(RPC::IRemoteConnection *connection);

            void RegisterAll();
            void UnregisterAll();

            uint32_t GetNumberOfCountries(Core::JSON::DecUInt8 &response) const;
            uint32_t GetCountryList(Core::JSON::ArrayType<CountryconfigData>& response) const;
            uint32_t GetCountry(Core::JSON::DecUInt32 &response) const;
            uint32_t SetCountry(Core::JSON::DecUInt32 code) const;

            uint32_t GetLnbList(Core::JSON::ArrayType<LnbsettingsInfo>& response) const;
            uint32_t GetSatelliteList(Core::JSON::ArrayType<SatellitesettingsInfo>& response) const;
            uint32_t GetNumberOfServices(Core::JSON::DecUInt16 &response) const;
            uint32_t GetServiceList(const string& index, Core::JSON::ArrayType<ServiceInfo>& response) const;
            uint32_t GetNowNextEvents(const string& service_uri, NowNextEventsData& response) const;
            uint32_t GetScheduleEvents(const string& index, Core::JSON::ArrayType<EiteventInfo>& response) const;
            uint32_t GetStatus(const string& index, StatusData& response) const;
            uint32_t GetServiceInfo(const string& index, ServiceInfo& response) const;
            uint32_t GetServiceComponents(const string& index, Core::JSON::ArrayType<ComponentData>& response) const;
            uint32_t GetTransportInfo(const string& index, TransportInfo& response) const;
            uint32_t GetExtendedEventInfo(const string& index, ExtendedeventinfoData& response) const;
            uint32_t GetSignalInfo(const string& index, SignalInfoData& response) const;

            uint32_t AddLnb(const LnbsettingsInfo& lnb_settings, Core::JSON::Boolean& response);
            uint32_t AddSatellite(const SatellitesettingsInfo& sat_settings, Core::JSON::Boolean& response);

            uint32_t StartServiceSearch(const StartServiceSearchParamsData& search_params, Core::JSON::Boolean& response);
            uint32_t FinishServiceSearch(const FinishServiceSearchParamsData& search_params, Core::JSON::Boolean& response);

            uint32_t StartPlaying(const StartPlayingParamsData& play_params, Core::JSON::DecSInt32& play_handle);
            uint32_t StopPlaying(Core::JSON::DecSInt32 play_handle);

            Core::JSON::EnumType<ServicetypeType> GetJsonServiceType(const IDTV::IService::ServiceType type) const;
            Core::JSON::EnumType<RunningstatusType> GetJsonRunningStatus(const IDTV::IService::RunState run_state) const;
            Core::JSON::EnumType<LnbtypeType> GetJsonLnbType(const IDTV::ILnb::LnbType lnb_type) const;
            Core::JSON::EnumType<LnbpowerType> GetJsonLnbPower(const IDTV::ILnb::LnbPower lnb_power) const;
            Core::JSON::EnumType<Diseqc_toneType> GetJsonDiseqcTone(const IDTV::ILnb::DiseqcTone diseqc_tone) const;
            Core::JSON::EnumType<Diseqc_cswitchType> GetJsonDiseqcCSwitch(const IDTV::ILnb::DiseqcCSwitch diseqc_cswitch) const;
            Core::JSON::EnumType<ComponentData::TypeType> GetJsonComponentType(const IDTV::IComponent::ComponentType type) const;
            Core::JSON::EnumType<CodecType> GetJsonCodecType(const IDTV::IComponent::CodecType type) const;
            Core::JSON::EnumType<ComponentData::AudioData::TypeType> GetJsonAudioType(const IDTV::IComponent::AudType type) const;
            Core::JSON::EnumType<ComponentData::AudioData::ModeType> GetJsonAudioMode(const IDTV::IComponent::AudMode mode) const;
            Core::JSON::EnumType<ComponentData::SubtitlesData::FormatType> GetJsonSubtitleFormat(const IDTV::IComponent::SubFormat type) const;
            Core::JSON::EnumType<TunertypeType> GetJsonTunerType(const IDTV::TunerType tuner_type) const;
            Core::JSON::EnumType<PolarityType> GetJsonPolarity(const IDTV::IDvbsTuningParams::PolarityType polarity) const;
            Core::JSON::EnumType<FecType> GetJsonFec(const IDTV::IDvbsTuningParams::FecType fec) const;
            Core::JSON::EnumType<DvbsmodulationType> GetJsonDvbsModulation(const IDTV::IDvbsTuningParams::ModulationType modulation) const;
            Core::JSON::EnumType<DvbcmodulationType> GetJsonDvbcModulation(const IDTV::IDvbcTuningParams::ModulationType modulation) const;
            Core::JSON::EnumType<DvbtbandwidthType> GetJsonBandwidth(const IDTV::IDvbtTuningParams::BandwidthType bandwidth) const;
            Core::JSON::EnumType<OfdmmodeType> GetJsonOfdmMode(const IDTV::IDvbtTuningParams::OfdmModeType mode) const;

            IDTV::TunerType GetTunerType(Core::JSON::EnumType<TunertypeType> tuner_type) const;
            IDTV::ILnb::LnbType GetLnbType(Core::JSON::EnumType<LnbtypeType> lnb_type) const;
            IDTV::ILnb::LnbPower GetLnbPower(Core::JSON::EnumType<LnbpowerType> lnb_power) const;
            IDTV::ILnb::DiseqcTone GetDiseqcTone(Core::JSON::EnumType<Diseqc_toneType> tone_type) const;
            IDTV::ILnb::DiseqcCSwitch GetDiseqcCSwitch(Core::JSON::EnumType<Diseqc_cswitchType> switch_type) const;
            IDTV::IDvbsTuningParams::PolarityType GetDvbsPolarity(Core::JSON::EnumType<PolarityType> polarity_type) const;
            IDTV::IDvbsTuningParams::FecType GetDvbsFEC(Core::JSON::EnumType<FecType> fec_type) const;
            IDTV::IDvbsTuningParams::ModulationType GetDvbsModulation(Core::JSON::EnumType<DvbsmodulationType> modulation_type) const;
            IDTV::IDvbtTuningParams::BandwidthType GetDvbtBandwidth(Core::JSON::EnumType<DvbtbandwidthType> bandwidth_type) const;
            IDTV::IDvbtTuningParams::OfdmModeType GetDvbtOfdmMode(Core::JSON::EnumType<OfdmmodeType> mode_type) const;
            IDTV::IDvbcTuningParams::ModulationType GetDvbcModulation(Core::JSON::EnumType<DvbcmodulationType> modulation_type) const;

            JsonObject CreateJsonForService(const IDTV::IService *service) const;
            JsonObject CreateJsonForEITEvent(const IDTV::IEitEvent *event) const;

            void ExtractDvbServiceInfo(ServiceInfo& info, const IDTV::IService *service) const;
            void ExtractDvbEventInfo(EiteventInfo& info, const IDTV::IEitEvent *event) const;
            void ExtractDvbStreamInfo(ComponentData& info, const IDTV::IComponent *component) const;
            void ExtractDvbTransportInfo(TransportInfo& info, const IDTV::ITransport *transport) const;
            void ExtractDvbsTuningParams(DvbstuningparamsInfo& tuning_params, const IDTV::ITransport *transport) const;
            void ExtractDvbcTuningParams(DvbctuningparamsInfo& tuning_params, const IDTV::ITransport *transport) const;
            void ExtractDvbtTuningParams(DvbttuningparamsInfo& tuning_params, const IDTV::ITransport *transport) const;

         private:
            uint32_t m_connectionId;
            Exchange::IDTV *m_dtv;
            PluginHost::IShell *m_service;
            Core::Sink<Notification> m_notification;
      };
   }
}

