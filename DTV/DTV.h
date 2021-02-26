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
#include <interfaces/json/JsonData_DTV.h>

extern "C"
{
   // DVB include files
   #include <techtype.h>

   #include <stbdpc.h>
};


namespace WPEFramework
{
   namespace Plugin
   {
      using namespace JsonData::DTV;

      class DTV: public PluginHost::IPlugin,
                 public PluginHost::IWeb,
                 public PluginHost::JSONRPC
      {
         private:
            // Disallow copying as we only want a single instance of the DVB stack
            DTV(const DTV&) = delete;
            DTV& operator=(const DTV&) = delete;

            class Notification: public RPC::IRemoteConnection::INotification
            {
               private:
                  Notification();
                  Notification(const Notification&);
                  Notification& operator=(const Notification&);

               public:
                  explicit Notification(DTV *parent) : _parent(*parent)
                  {
                      ASSERT(parent != nullptr);
                  }

                  ~Notification()
                  {
                  }

                  BEGIN_INTERFACE_MAP (Notification)
                     INTERFACE_ENTRY (RPC::IRemoteConnection::INotification)
                  END_INTERFACE_MAP

                  virtual void Activated(RPC::IRemoteConnection*)
                  {
                  }
                  virtual void Deactivated(RPC::IRemoteConnection *connection)
                  {
                     _parent.Deactivated(connection);
                  }

               private:
                  DTV& _parent;
            };

         class Config : public Core::JSON::Container
         {
            private:
               Config(const Config&);
               Config& operator=(const Config&);

            public:
               Config() : Core::JSON::Container(),
                  SubtitleProcessing(false),
                  TeletextProcessing(false),
                  OutOfProcess(true)
               {
                   Add(_T("subtitleprocessing"), &SubtitleProcessing);
                   Add(_T("teletextprocessing"), &TeletextProcessing);
                   Add(_T("outofprocess"), &OutOfProcess);
               }

               ~Config()
               {
               }

            public:
               Core::JSON::Boolean SubtitleProcessing;
               Core::JSON::Boolean TeletextProcessing;
               Core::JSON::Boolean OutOfProcess;
         };

         public:
            class Data: public Core::JSON::Container
            {
               private:
                  Data(const Data&) = delete;
                  Data& operator=(const Data&) = delete;

               public:
                  Data() : Core::JSON::Container(), NumberOfCountries(0), NumberOfServices(0)
                  {
                     Add(_T("numberOfCountries"), &NumberOfCountries);
                     Add(_T("countryList"), &CountryList);
                     Add(_T("countrycode"), &CountryCode);
                     Add(_T("numberOfServices"), &NumberOfServices);
                     Add(_T("serviceList"), &ServiceList);
                  }

                  ~Data()
                  {
                  }

               public:
                  Core::JSON::DecUInt8 NumberOfCountries;
                  Core::JSON::ArrayType<CountryconfigData> CountryList;
                  Core::JSON::DecUInt32 CountryCode;
                  Core::JSON::DecUInt16 NumberOfServices;
                  Core::JSON::ArrayType<ServiceData> ServiceList;
            };

         public:
            DTV() : _skipURL(0), _service(nullptr), _connectionId(0), _dtv(nullptr), _notification(this)
            {
               DTV::instance(this);
               RegisterAll();
            }

            virtual ~DTV()
            {
               UnregisterAll();
            }

            static DTV* instance(DTV *dtv = nullptr)
            {
               static DTV *dtv_instance = nullptr;

               if (dtv != nullptr)
               {
                  dtv_instance = dtv;
               }

               return(dtv_instance);
            }

         public:
            BEGIN_INTERFACE_MAP (DTV)
            INTERFACE_ENTRY (PluginHost::IPlugin)
            INTERFACE_ENTRY (PluginHost::IWeb)
            INTERFACE_ENTRY (PluginHost::IDispatcher)
            END_INTERFACE_MAP

         public:
            //  IPlugin methods
            // -------------------------------------------------------------------------------------------------------
            virtual const string Initialize(PluginHost::IShell* service);
            virtual void Deinitialize(PluginHost::IShell *service);
            virtual string Information() const;

            //  IWeb methods
            // -------------------------------------------------------------------------------------------------------
            virtual void Inbound(Web::Request &request);
            virtual Core::ProxyType<Web::Response> Process(const Web::Request &request);

         private:
            void Deactivated(RPC::IRemoteConnection *connection);

            void NotifySearchStatus(void);

            // JsonRpc
            void RegisterAll();
            void UnregisterAll();
            uint32_t GetNumberOfCountries(Core::JSON::DecUInt8 &response) const;
            uint32_t GetCountryList(Core::JSON::ArrayType<CountryconfigData>& response) const;
            uint32_t GetCountry(Core::JSON::DecUInt32 &response) const;
            uint32_t SetCountry(Core::JSON::DecUInt32 code) const;
            uint32_t GetLnbList(Core::JSON::ArrayType<LnbsettingsInfo>& response) const;
            uint32_t GetSatelliteList(Core::JSON::ArrayType<SatellitesettingsInfo>& response) const;
            uint32_t GetNumberOfServices(Core::JSON::DecUInt16 &response) const;
            uint32_t GetServiceList(const string& tuner_type, Core::JSON::ArrayType<ServiceData>& response) const;
            uint32_t GetNowNextEvents(const string& service_uri, NowNextEventsData& response) const;
            uint32_t GetScheduleEvents(const string& index, Core::JSON::ArrayType<EiteventInfo>& response) const;
            uint32_t GetStatus(const string& index, StatusData& response) const;

            uint32_t AddLnb(const LnbsettingsInfo& lnb_settings, Core::JSON::Boolean& response);
            uint32_t AddSatellite(const SatellitesettingsInfo& sat_settings, Core::JSON::Boolean& response);

            uint32_t StartServiceSearch(const StartServiceSearchParamsData& search_params, Core::JSON::Boolean& response);
            uint32_t FinishServiceSearch(const FinishServiceSearchParamsData& search_params, Core::JSON::Boolean& response);
            uint32_t StartPlaying(const StartPlayingParamsData& play_params, Core::JSON::DecSInt32& play_handle);
            uint32_t StopPlaying(Core::JSON::DecSInt32 play_handle);

            void EventSearchStatus(SearchstatusParamsData& params);

         private:
            Core::ProxyType<Web::Response> GetMethod(Core::TextSegmentIterator& index);
            Core::ProxyType<Web::Response> PutMethod(Core::TextSegmentIterator& index, const Web::Request& request);
            Core::ProxyType<Web::Response> PostMethod(Core::TextSegmentIterator& index, const Web::Request& request);
#if 0
            Core::ProxyType<Web::Response> DeleteMethod(Core::TextSegmentIterator& index, const Web::Request& request);
#endif

         private:
            uint8_t _skipURL;
            uint32_t _connectionId;
            Core::IUnknown *_dtv;
            PluginHost::IShell *_service;
            Core::Sink<Notification> _notification;

         private:
            static void DvbEventHandler(U32BIT event, void *event_data, U32BIT data_size);

            Core::JSON::EnumType<LnbtypeType> GetJsonLnbType(E_STB_DP_LNB_TYPE type) const;
            Core::JSON::EnumType<LnbpowerType> GetJsonLnbPower(E_STB_DP_LNB_POWER power) const;
            Core::JSON::EnumType<Diseqc_toneType> GetJsonDiseqcTone(E_STB_DP_DISEQC_TONE tone) const;
            Core::JSON::EnumType<Diseqc_cswitchType> GetJsonDiseqcCSwitch(E_STB_DP_DISEQC_CSWITCH cswitch) const;

            E_STB_DP_LNB_TYPE GetDvbLnbType(Core::JSON::EnumType<LnbtypeType> lnb_type) const;
            E_STB_DP_LNB_POWER GetDvbLnbPower(Core::JSON::EnumType<LnbpowerType> lnb_power) const;
            E_STB_DP_DISEQC_TONE GetDvbDiseqcTone(Core::JSON::EnumType<Diseqc_toneType> diseqc_tone) const;
            E_STB_DP_DISEQC_CSWITCH GetDvbDiseqcCSwitch(Core::JSON::EnumType<Diseqc_cswitchType> diseqc_cswitch) const;

            E_STB_DP_SIGNAL_TYPE GetDvbSignalType(TunertypeType tuner_type) const;
            E_STB_DP_POLARITY GetDvbPolarity(StartServiceSearchParamsData::DvbstuningparamsData::PolarityType polarity_type) const;
            E_STB_DP_FEC GetDvbsFEC(StartServiceSearchParamsData::DvbstuningparamsData::FecType fec_type) const;
            E_STB_DP_MODULATION GetDvbsModulation(StartServiceSearchParamsData::DvbstuningparamsData::DvbsmodulationType modulation_type) const;
            E_STB_DP_CMODE GetDvbcModulation(StartServiceSearchParamsData::DvbctuningparamsData::DvbcmodulationType modulation_type) const;

            void* FindSatellite(const char *satellite_name) const;

            void ExtractDvbEventInfo(void *src_event, EiteventInfo& dest_event) const;
            void SetJsonString(U8BIT *src_string, Core::JSON::String& out_string, bool free_src = false) const;
      };
   }
}

