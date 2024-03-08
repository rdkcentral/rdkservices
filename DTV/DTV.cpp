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

#include "DTV.h"

#define API_VERSION_NUMBER_MAJOR 1
#define API_VERSION_NUMBER_MINOR 0
#define API_VERSION_NUMBER_PATCH 3

namespace WPEFramework
{
   namespace {

        static Plugin::Metadata<Plugin::DTV> metadata(
            // Version (Major, Minor, Patch)
            API_VERSION_NUMBER_MAJOR, API_VERSION_NUMBER_MINOR, API_VERSION_NUMBER_PATCH,
            // Preconditions
            {},
            // Terminations
            {},
            // Controls
            {}
        );
   }

   namespace Plugin
   {
      using namespace JsonData::DTV;
      using namespace Exchange;

      SERVICE_REGISTRATION(DTV, API_VERSION_NUMBER_MAJOR, API_VERSION_NUMBER_MINOR, API_VERSION_NUMBER_PATCH);

      DTV::DTV() : m_service(nullptr), m_connectionId(0), m_dtv(nullptr), m_notification(this)
      {
      }

      DTV::~DTV()
      {
      }

      const string DTV::Initialize(PluginHost::IShell *service)
      {
         ASSERT(service != nullptr);
         ASSERT(m_dtv == nullptr);
         ASSERT(m_service == nullptr);
         ASSERT(m_connectionId == 0);

         string message = "";

         SYSLOG(Logging::Startup, (_T("DTV::Initialize: PID=%u"), getpid()));

         m_service = service;
         m_service->AddRef();

         Config config;
         config.FromString(m_service->ConfigLine());

         m_service->Register(&m_notification);

         m_dtv = service->Root<Exchange::IDTV>(m_connectionId, 2000, _T("DTVImpl"));
         if (m_dtv != nullptr)
         {
            // Initialise the out-of-process plugin
            m_dtv->Initialize(service);

            // Register for notifications
            m_dtv->Register(&m_notification);

            // Register the JSONRPC APIs
//            Exchange::JDTV::Register(*this, m_dtv);
            RegisterAll();
         }
         else
         {
            SYSLOG(Logging::Startup, (_T("DTV::Initialize: Failed to initialise DTV plugin")));
            message = _T("DTV plugin could not be initialised");
         }

         if (message.length() != 0)
         {
            Deinitialize(service);
         }

         return message;
      }

      void DTV::Deinitialize(PluginHost::IShell *service)
      {
         ASSERT(m_service == service);

         SYSLOG(Logging::Shutdown, (string(_T("DTV::Deinitialize"))));

         // Make sure the Activated and Deactivated are no longer called before we start cleaning up..
         m_service->Unregister(&m_notification);

         if (m_dtv != nullptr)
         {
            // Unregister all the JSONRPC APIs
            UnregisterAll();

            m_dtv->Unregister(&m_notification);

            // Stop processing:
            RPC::IRemoteConnection* connection = service->RemoteConnection(m_connectionId);

            VARIABLE_IS_NOT_USED uint32_t result = m_dtv->Release();
            m_dtv = nullptr;

            // It should have been the last reference we are releasing,
            // so it should endup in a DESTRUCTION_SUCCEEDED, if not we
            // are leaking...
            ASSERT(result == Core::ERROR_DESTRUCTION_SUCCEEDED);

            // If this was running in a (container) process...
            if (connection != nullptr)
            {
               // Lets trigger the cleanup sequence for
               // out-of-process code. Which will guard
               // that unwilling processes, get shot if
               // not stopped friendly :-)
               connection->Terminate();
               connection->Release();
            }
         }

         m_connectionId = 0;
         m_service->Release();
         m_service = nullptr;
         SYSLOG(Logging::Shutdown, (string(_T("DTV de-initialised"))));
      }

      string DTV::Information() const
      {
         // No additional info to report
         return (string());
      }

      void DTV::Deactivated(RPC::IRemoteConnection *connection)
      {
         if (connection->Id() == m_connectionId)
         {
            ASSERT(m_service != nullptr);
            PluginHost::WorkerPool::Instance().Submit(PluginHost::IShell::Job::Create(m_service,
               PluginHost::IShell::DEACTIVATED, PluginHost::IShell::FAILURE));
         }
      }

      void DTV::RegisterAll()
      {
         // Version 1 properties, though some include additional info in version 2
         JSONRPC::Property<Core::JSON::DecUInt8>(_T("numberOfCountries"), &DTV::GetNumberOfCountries, nullptr, this);
         JSONRPC::Property<Core::JSON::ArrayType<CountryconfigData>>(_T("countryList"), &DTV::GetCountryList, nullptr, this);
         JSONRPC::Property<Core::JSON::DecUInt32>(_T("country"), &DTV::GetCountry, &DTV::SetCountry, this);
         JSONRPC::Property<Core::JSON::ArrayType<LnbsettingsInfo>>(_T("lnbList"), &DTV::GetLnbList, nullptr, this);
         JSONRPC::Property<Core::JSON::ArrayType<SatellitesettingsInfo>>(_T("satelliteList"), &DTV::GetSatelliteList, nullptr, this);
         JSONRPC::Property<Core::JSON::DecUInt16>(_T("numberOfServices"), &DTV::GetNumberOfServices, nullptr, this);
         JSONRPC::Property<Core::JSON::ArrayType<ServiceInfo>>(_T("serviceList"), &DTV::GetServiceList, nullptr, this);
         JSONRPC::Property<NowNextEventsData>(_T("nowNextEvents"), &DTV::GetNowNextEvents, nullptr, this);
         JSONRPC::Property<Core::JSON::ArrayType<EiteventInfo>>(_T("scheduleEvents"), &DTV::GetScheduleEvents, nullptr, this);
         JSONRPC::Property<StatusData>(_T("status"), &DTV::GetStatus, nullptr, this);

         // Version 2 properties
         JSONRPC::Property<ServiceInfo>(_T("serviceInfo"), &DTV::GetServiceInfo, nullptr, this);
         JSONRPC::Property<Core::JSON::ArrayType<ComponentData>>(_T("serviceComponents"), &DTV::GetServiceComponents, nullptr, this);
         JSONRPC::Property<TransportInfo>(_T("transportInfo"), &DTV::GetTransportInfo, nullptr, this);
         JSONRPC::Property<ExtendedeventinfoData>(_T("extendedEventInfo"), &DTV::GetExtendedEventInfo, nullptr, this);
         JSONRPC::Property<SignalInfoData>(_T("signalInfo"), &DTV::GetSignalInfo, nullptr, this);

         // Version 1 methods
         JSONRPC::Register<LnbsettingsInfo, Core::JSON::Boolean>(_T("addLnb"), &DTV::AddLnb, this);
         JSONRPC::Register<SatellitesettingsInfo, Core::JSON::Boolean>(_T("addSatellite"), &DTV::AddSatellite, this);

         JSONRPC::Register<StartServiceSearchParamsData, Core::JSON::Boolean>(_T("startServiceSearch"), &DTV::StartServiceSearch, this);
         JSONRPC::Register<FinishServiceSearchParamsData, Core::JSON::Boolean>(_T("finishServiceSearch"), &DTV::FinishServiceSearch, this);
         JSONRPC::Register<StartPlayingParamsData, Core::JSON::DecSInt32>(_T("startPlaying"), &DTV::StartPlaying, this);
         JSONRPC::Register<Core::JSON::DecSInt32, void>(_T("stopPlaying"), &DTV::StopPlaying, this);

         // Version 2 methods
      }

      void DTV::UnregisterAll()
      {
         // Properties
         JSONRPC::Unregister(_T("numberOfCountries"));
         JSONRPC::Unregister(_T("countryList"));
         JSONRPC::Unregister(_T("country"));
         JSONRPC::Unregister(_T("lnbList"));
         JSONRPC::Unregister(_T("satelliteList"));
         JSONRPC::Unregister(_T("numberOfServices"));
         JSONRPC::Unregister(_T("serviceList"));
         JSONRPC::Unregister(_T("nowNextEvents"));
         JSONRPC::Unregister(_T("scheduleEvents"));
         JSONRPC::Unregister(_T("status"));
         JSONRPC::Unregister(_T("serviceInfo"));
         JSONRPC::Unregister(_T("serviceComponents"));
         JSONRPC::Unregister(_T("transportInfo"));
         JSONRPC::Unregister(_T("extendedEventInfo"));
         JSONRPC::Unregister(_T("signalInfo"));

         // Methods
         JSONRPC::Unregister(_T("addLnb"));
         JSONRPC::Unregister(_T("addSatellite"));
         JSONRPC::Unregister(_T("startServiceSearch"));
         JSONRPC::Unregister(_T("finishServiceSearch"));
         JSONRPC::Unregister(_T("startPlaying"));
         JSONRPC::Unregister(_T("stopPlaying"));
      }

      uint32_t DTV::GetNumberOfCountries(Core::JSON::DecUInt8 &response) const
      {
         uint8_t count;
         uint32_t result = m_dtv->GetNumberOfCountries(count);
         if (result == Core::ERROR_NONE)
         {
            response = count;
         }

         return result;
      }

      uint32_t DTV::GetCountryList(Core::JSON::ArrayType<CountryconfigData>& response) const
      {
         IDTV::ICountry::IIterator* iterator;

         uint32_t result = m_dtv->GetCountryList(iterator);
         if (result == Core::ERROR_NONE)
         {
            IDTV::ICountry* country;

            while (iterator->Current(country) == Core::ERROR_NONE)
            {
               CountryconfigData config;
               std::string name;
               uint32_t code;

               if (country->Name(name) == Core::ERROR_NONE)
               {
                  if (country->Code(code) == Core::ERROR_NONE)
                  {
                     //config.Name = Core::ToString(name.c_str());
                     config.Name = name.c_str();
                     config.Code = code;

                     response.Add(config);
                  }
               }

               iterator->Next();
            }
         }

         return result;
      }

      uint32_t DTV::GetCountry(Core::JSON::DecUInt32 &response) const
      {
         uint32_t country;

         uint32_t result = m_dtv->GetCountry(country);
         if (result == Core::ERROR_NONE)
         {
            response = country;
         }

         return result;
      }

      uint32_t DTV::SetCountry(Core::JSON::DecUInt32 code) const
      {
         return (m_dtv->SetCountry(code));
      }

      uint32_t DTV::GetLnbList(Core::JSON::ArrayType<LnbsettingsInfo>& response) const
      {
         IDTV::ILnb::IIterator *iterator;

         uint32_t result = m_dtv->GetLnbList(iterator);
         if (result == Core::ERROR_NONE)
         {
            IDTV::ILnb *lnb;
            LnbsettingsInfo info;
            std::string str;
            IDTV::ILnb::LnbType type;
            IDTV::ILnb::LnbPower power;
            IDTV::ILnb::DiseqcTone tone;
            IDTV::ILnb::DiseqcCSwitch cswitch;
            bool bvalue;
            uint8_t value8;
            uint32_t value32;

            while (iterator->Current(lnb) == Core::ERROR_NONE)
            {
               lnb->Name(str);
               info.Name = str.c_str();

               lnb->Type(type);
               info.Type = GetJsonLnbType(type);

               lnb->Power(power);
               info.Power = GetJsonLnbPower(power);

               lnb->Tone(tone);
               info.Diseqc_tone = GetJsonDiseqcTone(tone);

               lnb->CSwitch(cswitch);
               info.Diseqc_cswitch = GetJsonDiseqcCSwitch(cswitch);

               lnb->Is22kHz(bvalue);
               info.Is22k = bvalue;

               lnb->Is12V(bvalue);
               info.Is12v = bvalue;

               lnb->IsPulsePosition(bvalue);
               info.Ispulseposition = bvalue;

               lnb->IsDiseqcPosition(bvalue);
               info.Isdiseqcposition = bvalue;

               lnb->IsSmatv(bvalue);
               info.Issmatv = bvalue;

               lnb->DiseqcRepeats(value8);
               info.Diseqcrepeats = value8;

               lnb->USwitch(value8);
               info.U_switch = value8;

               lnb->UnicableChannel(value8);
               info.Unicablechannel = value8;

               lnb->UnicableFreq(value32);
               info.Unicablefreq = value32;

               response.Add(info);

               iterator->Next();
            }
         }

         return result;
      }

      uint32_t DTV::GetSatelliteList(Core::JSON::ArrayType<SatellitesettingsInfo>& response) const
      {
         IDTV::ISatellite::IIterator *iterator;

         uint32_t result = m_dtv->GetSatelliteList(iterator);
         if (result == Core::ERROR_NONE)
         {
            IDTV::ISatellite *sat;
            SatellitesettingsInfo info;
            std::string str;
            int16_t value16;

            while (iterator->Current(sat) == Core::ERROR_NONE)
            {
               sat->Name(str);
               info.Name = str.c_str();

               sat->Longitude(value16);
               info.Longitude = value16;

               sat->Lnb(str);
               info.Lnb = str.c_str();

               response.Add(info);

               iterator->Next();
            }
         }

         return result;
      }

      uint32_t DTV::GetNumberOfServices(Core::JSON::DecUInt16 &response) const
      {
         uint16_t num_services;

         uint32_t result = m_dtv->GetNumberOfServices(num_services);
         if (result == Core::ERROR_NONE)
         {
            response = num_services;
         }

         return result;
      }

      uint32_t DTV::GetServiceList(const string& index, Core::JSON::ArrayType<ServiceInfo>& response) const
      {
         IDTV::IService::IIterator* iterator;
         uint16_t onet_id, trans_id;
         uint32_t result;

         // The index may be a doublet specifying a transport
         if (std::sscanf(index.c_str(), "%hu.%hu", &onet_id, &trans_id) == 2)
         {
            result = m_dtv->GetServiceList(onet_id, trans_id, iterator);
         }
         else
         {
            IDTV::TunerType tuner_type;

            // Check for a tuner type, otherwise all services will be returned
            switch(Core::EnumerateType<TunertypeType>(index.c_str()).Value())
            {
               case TunertypeType::DVBS:
                  tuner_type = IDTV::TunerType::DVBS;
                  break;
               case TunertypeType::DVBT:
                  tuner_type = IDTV::TunerType::DVBT;
                  break;
               case TunertypeType::DVBC:
                  tuner_type = IDTV::TunerType::DVBC;
                  break;
               default:
                  tuner_type = IDTV::TunerType::NONE;
                  break;
            }

            result = m_dtv->GetServiceList(tuner_type, iterator);
         }

         if (result == Core::ERROR_NONE)
         {
            IDTV::IService *service;
            ServiceInfo info;

            while (iterator->Current(service) == Core::ERROR_NONE)
            {
               ExtractDvbServiceInfo(info, service);

               response.Add(info);

               iterator->Next();
            }
         }

         return result;
      }

      uint32_t DTV::GetNowNextEvents(const string& service_uri, NowNextEventsData& response) const
      {
         uint32_t result = Core::ERROR_BAD_REQUEST;

         SYSLOG(Logging::Notification, (_T("DTV::GetNowNextEvents: %s"), service_uri.c_str()));

         if (service_uri.length() != 0)
         {
            uint16_t onet_id, trans_id, serv_id;

            if (std::sscanf(service_uri.c_str(), "%hu.%hu.%hu", &onet_id, &trans_id, &serv_id) == 3)
            {
               IDTV::IEitEvent *now_event;
               IDTV::IEitEvent *next_event;

               result = m_dtv->GetNowNextEvents(onet_id, trans_id, serv_id, now_event, next_event);
               if (result == Core::ERROR_NONE)
               {
                  if (now_event != nullptr)
                  {
                     ExtractDvbEventInfo(response.Now, now_event);
                  }
                  else
                  {
                     response.Now.Starttime = 0;
                  }

                  if (next_event != nullptr)
                  {
                     ExtractDvbEventInfo(response.Next, next_event);
                  }
                  else
                  {
                     response.Next.Starttime = 0;
                  }
               }
            }
         }

         return (result);
      }

      uint32_t DTV::GetScheduleEvents(const string& index, Core::JSON::ArrayType<EiteventInfo>& response) const
      {
         uint32_t result = Core::ERROR_BAD_REQUEST;

         if (index.length() != 0)
         {
            uint16_t onet_id, trans_id, serv_id;
            uint32_t start_utc = 0;
            uint32_t end_utc = 0xffffffff;

            int num_args = std::sscanf(index.c_str(), "%hu.%hu.%hu:%u,%u", &onet_id, &trans_id, &serv_id,
               &start_utc, &end_utc);

            if (num_args >= 3)
            {
               IDTV::IEitEvent::IIterator* iterator;

               result = m_dtv->GetScheduleEvents(onet_id, trans_id, serv_id, start_utc, end_utc, iterator);
               if (result == Core::ERROR_NONE)
               {
                  IDTV::IEitEvent *event;

                  while (iterator->Current(event) == Core::ERROR_NONE)
                  {
                     EiteventInfo info;

                     ExtractDvbEventInfo(info, event);

                     response.Add(info);

                     iterator->Next();
                  }
               }
            }
         }

         return (result);
      }

      uint32_t DTV::GetStatus(const string& index, StatusData& response) const
      {
         uint32_t result = Core::ERROR_BAD_REQUEST;

         if (index.length() != 0)
         {
            IDTV::IStatus *status;
            int32_t handle = atoi(index.c_str());

            result = m_dtv->GetStatus(handle, status);
            if (result == Core::ERROR_NONE)
            {
               uint8_t value8;
               uint16_t value16;
               std::string dvburi;

               status->Tuner(value8);
               response.Tuner = value8;

               status->Demux(value8);
               response.Demux = value8;

               status->PmtPid(value16);
               response.Pmtpid = value16;

               status->Dvburi(dvburi);
               response.Dvburi = dvburi;

               status->Lcn(value16);
               response.Lcn = value16;
            }
         }

         return (result);
      }

      uint32_t DTV::GetServiceInfo(const string& index, ServiceInfo& response) const
      {
         uint32_t result = Core::ERROR_BAD_REQUEST;
         uint16_t onet_id, trans_id, serv_id;

         // The index must be a service's DVB triplet
         if (std::sscanf(index.c_str(), "%hu.%hu.%hu", &onet_id, &trans_id, &serv_id) == 3)
         {
            IDTV::IService *info;

            result = m_dtv->GetServiceInfo(onet_id, trans_id, serv_id, info);
            if (result == Core::ERROR_NONE)
            {
               ExtractDvbServiceInfo(response, info);
            }
         }

         return result;
      }

      uint32_t DTV::GetServiceComponents(const string& index, Core::JSON::ArrayType<ComponentData>& response) const
      {
         uint32_t result = Core::ERROR_BAD_REQUEST;
         uint16_t onet_id, trans_id, serv_id;

         // The index must be a service's DVB triplet
         if (std::sscanf(index.c_str(), "%hu.%hu.%hu", &onet_id, &trans_id, &serv_id) == 3)
         {
            IDTV::IComponent::IIterator *iterator;

            result = m_dtv->GetServiceComponents(onet_id, trans_id, serv_id, iterator);
            if (result == Core::ERROR_NONE)
            {
               IDTV::IComponent *component;

               while (iterator->Current(component) == Core::ERROR_NONE)
               {
                  ComponentData info;

                  ExtractDvbStreamInfo(info, component);

                  response.Add(info);

                  iterator->Next();
               }
            }
         }

         return (result);
      }

      uint32_t DTV::GetTransportInfo(const string& index, TransportInfo& response) const
      {
         uint32_t result = Core::ERROR_BAD_REQUEST;
         uint16_t onet_id, trans_id;

         // The index must be a transport's DVB doublet
         int num_args = std::sscanf(index.c_str(), "%hu.%hu", &onet_id, &trans_id);
         if (num_args == 2)
         {
            IDTV::ITransport *transport;

            result = m_dtv->GetTransportInfo(onet_id, trans_id, transport);
            if (result == Core::ERROR_NONE)
            {
               ExtractDvbTransportInfo(response, transport);
            }
         }

         return (result);
      }

      uint32_t DTV::GetExtendedEventInfo(const string& index, ExtendedeventinfoData& response) const
      {
         uint32_t result = Core::ERROR_BAD_REQUEST;
         uint16_t onet_id, trans_id, serv_id, event_id;

         // The index must be a service's DVB triplet followed by the event ID
         int num_args = std::sscanf(index.c_str(), "%hu.%hu.%hu:%hu", &onet_id, &trans_id,
            &serv_id, &event_id);
         if (num_args == 4)
         {
            IDTV::IEitExtendedEvent *extended_info;

            result = m_dtv->GetExtendedEventInfo(onet_id, trans_id, serv_id, event_id, extended_info);
            if (result == Core::ERROR_NONE)
            {
               std::string str;
               IDTV::IEitExtendedEvent::IEitExtendedEventItem::IIterator *iterator;

               extended_info->Description(str);
               response.Description = str.c_str();

               if (extended_info->Items(iterator) == Core::ERROR_NONE)
               {
                  IDTV::IEitExtendedEvent::IEitExtendedEventItem *item;
                  ExtendedeventinfoData::ExtendedeventitemData ex_item;

                  while (iterator->Current(item) == Core::ERROR_NONE)
                  {
                     item->Description(str);
                     ex_item.Description = str.c_str();

                     item->Item(str);
                     ex_item.Item = str.c_str();

                     response.Items.Add(ex_item);

                     iterator->Next();
                  }
               }
            }
         }

         return(result);
      }

      uint32_t DTV::GetSignalInfo(const string& index, SignalInfoData& response) const
      {
         uint32_t result = Core::ERROR_BAD_REQUEST;

         if (index.length() != 0)
         {
            IDTV::ISignalInfo *info;
            int32_t handle = atoi(index.c_str());

            result = m_dtv->GetSignalInfo(handle, info);
            if (result == Core::ERROR_NONE)
            {
               bool locked;
               uint8_t strength, quality;

               info->Locked(locked);
               response.Locked = locked;

               info->Strength(strength);
               response.Strength = strength;

               info->Quality(quality);
               response.Quality = quality;
            }
         }

         return(result);
      }

      uint32_t DTV::AddLnb(const LnbsettingsInfo& lnb_settings, Core::JSON::Boolean& response)
      {
         uint32_t result = m_dtv->AddLnb(lnb_settings.Name.Value(), GetLnbType(lnb_settings.Type),
            GetLnbPower(lnb_settings.Power), GetDiseqcTone(lnb_settings.Diseqc_tone),
            GetDiseqcCSwitch(lnb_settings.Diseqc_cswitch), lnb_settings.Is22k, lnb_settings.Is12v,
            lnb_settings.Ispulseposition, lnb_settings.Isdiseqcposition, lnb_settings.Issmatv,
            lnb_settings.Diseqcrepeats, lnb_settings.U_switch, lnb_settings.Unicablechannel,
            lnb_settings.Unicablefreq);

         if (result == Core::ERROR_NONE)
         {
            response = true;
         }
         else
         {
            response = false;
         }

         return(result);
      }

      uint32_t DTV::AddSatellite(const SatellitesettingsInfo& sat_settings, Core::JSON::Boolean& response)
      {
         uint32_t result = m_dtv->AddSatellite(sat_settings.Name.Value(), sat_settings.Longitude,
            sat_settings.Lnb.Value());
         if (result == Core::ERROR_NONE)
         {
            response = true;
         }
         else
         {
            response = false;
         }

         return(result);
      }

      uint32_t DTV::StartServiceSearch(const StartServiceSearchParamsData& start_search, Core::JSON::Boolean& response)
      {
         IDTV::TunerType tuner_type = GetTunerType(start_search.Tunertype);
         uint32_t result;

         response = false;

         if (tuner_type != IDTV::TunerType::NONE)
         {
            IDTV::ServiceSearchType search_type;
            bool retune = (start_search.Retune ? true : false);

            if (start_search.Searchtype == StartServiceSearchParamsData::SearchtypeType::FREQUENCY)
            {
               SYSLOG(Logging::Notification, (_T("DTV::StartServiceSearch: frequency")));
               search_type = IDTV::ServiceSearchType::FREQUENCY;
            }
            else
            {
               SYSLOG(Logging::Notification, (_T("DTV::StartServiceSearch: network")));
               search_type = IDTV::ServiceSearchType::NETWORK;
            }

            if (start_search.Usetuningparams)
            {
               switch(tuner_type)
               {
                  case IDTV::TunerType::DVBS:
                     SYSLOG(Logging::Notification, (_T("DTV::StartServiceSearch: manual DVB-S")));
                     result = m_dtv->StartServiceSearch(search_type, retune,
                        start_search.Dvbstuningparams.Satellite.Value(),
                        (uint32_t)start_search.Dvbstuningparams.Frequency.Value(),
                        GetDvbsPolarity(start_search.Dvbstuningparams.Polarity),
                        (uint16_t)start_search.Dvbstuningparams.Symbolrate.Value(),
                        GetDvbsFEC(start_search.Dvbstuningparams.Fec),
                        GetDvbsModulation(start_search.Dvbstuningparams.Modulation),
                        start_search.Dvbstuningparams.Dvbs2);
                     break;

                  case IDTV::TunerType::DVBT:
                     SYSLOG(Logging::Notification, (_T("DTV::StartServiceSearch: manual DVB-T")));
                     result = m_dtv->StartServiceSearch(search_type, retune,
                        (uint32_t)start_search.Dvbttuningparams.Frequency.Value(),
                        GetDvbtBandwidth(start_search.Dvbttuningparams.Bandwidth),
                        GetDvbtOfdmMode(start_search.Dvbttuningparams.Mode),
                        start_search.Dvbttuningparams.Dvbt2,
                        (uint8_t)start_search.Dvbttuningparams.Plpid);
                     break;

                  case IDTV::TunerType::DVBC:
                     SYSLOG(Logging::Notification, (_T("DTV::StartServiceSearch: manual DVB-C")));
                     result = m_dtv->StartServiceSearch(search_type, retune,
                        start_search.Dvbctuningparams.Frequency.Value(),
                        (uint16_t)start_search.Dvbctuningparams.Symbolrate.Value(),
                        GetDvbcModulation(start_search.Dvbctuningparams.Modulation));
                     break;

                  default:
                     result = Core::ERROR_BAD_REQUEST;
                     break;
               }
            }
            else
            {
               SYSLOG(Logging::Notification, (_T("DTV::StartServiceSearch: auto")));

               result = m_dtv->StartServiceSearch(tuner_type, search_type, retune);
            }

            if (result == Core::ERROR_NONE)
            {
               response = true;
            }
         }
         else
         {
            result = Core::ERROR_BAD_REQUEST;
         }

         return(result);
      }

      uint32_t DTV::FinishServiceSearch(const FinishServiceSearchParamsData& finish_search, Core::JSON::Boolean& response)
      {
         uint32_t result;
         IDTV::TunerType tuner_type = GetTunerType(finish_search.Tunertype);

         if (tuner_type != IDTV::TunerType::NONE)
         {
            result = m_dtv->FinishServiceSearch(tuner_type, (finish_search.Savechanges ? true : false));
            response = true;
         }
         else
         {
            result = Core::ERROR_BAD_REQUEST;
            response = false;
         }

         return(result);
      }

      uint32_t DTV::StartPlaying(const StartPlayingParamsData& play_params, Core::JSON::DecSInt32& play_handle)
      {
         uint32_t result = Core::ERROR_BAD_REQUEST;
         bool monitor_only = false;
         int32_t decode_path = -1;

         if (play_params.Monitoronly.IsSet())
         {
            monitor_only = play_params.Monitoronly;
            SYSLOG(Logging::Notification, (_T("DTV::StartPlaying: monitor=%u"), monitor_only));
         }

         if (play_params.Dvburi.IsSet())
         {
            SYSLOG(Logging::Notification, (_T("DTV::StartPlaying: uri=%s"), play_params.Dvburi.Value().c_str()));
            result = m_dtv->StartPlaying(play_params.Dvburi.Value(), monitor_only, decode_path);
         }
         else if (play_params.Lcn.IsSet())
         {
            SYSLOG(Logging::Notification, (_T("DTV::StartPlaying: lcn=%u"), play_params.Lcn.Value()));
            result = m_dtv->StartPlaying(play_params.Lcn.Value(), monitor_only, decode_path);
         }

         if (result == Core::ERROR_NONE)
         {
            play_handle = decode_path;
            SYSLOG(Logging::Notification, (_T("DTV::StartPlaying: %d"), decode_path));
         }

         return(result);
      }

      uint32_t DTV::StopPlaying(Core::JSON::DecSInt32 play_handle)
      {
         SYSLOG(Logging::Notification, (_T("DTV::StopPlaying: %d"), play_handle.Value()));
         return (m_dtv->StopPlaying(play_handle.Value()));
      }

      Core::JSON::EnumType<ServicetypeType> DTV::GetJsonServiceType(const IDTV::IService::ServiceType type) const
      {
         Core::JSON::EnumType<ServicetypeType> serv_type;

         switch(type)
         {
            case IDTV::IService::ServiceType::TV:
               serv_type = ServicetypeType::TV;
               break;
            case IDTV::IService::ServiceType::RADIO:
               serv_type = ServicetypeType::RADIO;
               break;
            case IDTV::IService::ServiceType::TELETEXT:
               serv_type = ServicetypeType::TELETEXT;
               break;
            case IDTV::IService::ServiceType::NVOD:
               serv_type = ServicetypeType::NVOD;
               break;
            case IDTV::IService::ServiceType::NVOD_TIMESHIFT:
               serv_type = ServicetypeType::NVOD_TIMESHIFT;
               break;
            case IDTV::IService::ServiceType::MOSAIC:
               serv_type = ServicetypeType::MOSAIC;
               break;
            case IDTV::IService::ServiceType::AVC_RADIO:
               serv_type = ServicetypeType::AVC_RADIO;
               break;
            case IDTV::IService::ServiceType::AVC_MOSAIC:
               serv_type = ServicetypeType::AVC_MOSAIC;
               break;
            case IDTV::IService::ServiceType::DATA:
               serv_type = ServicetypeType::DATA;
               break;
            case IDTV::IService::ServiceType::MPEG2_HD:
               serv_type = ServicetypeType::MPEG2_HD;
               break;
            case IDTV::IService::ServiceType::AVC_SD_TV:
               serv_type = ServicetypeType::AVC_SD_TV;
               break;
            case IDTV::IService::ServiceType::AVC_SD_NVOD_TIMESHIFT:
               serv_type = ServicetypeType::AVC_SD_NVOD_TIMESHIFT;
               break;
            case IDTV::IService::ServiceType::AVC_SD_NVOD:
               serv_type = ServicetypeType::AVC_SD_NVOD;
               break;
            case IDTV::IService::ServiceType::HD_TV:
               serv_type = ServicetypeType::HD_TV;
               break;
            case IDTV::IService::ServiceType::AVC_HD_NVOD_TIMESHIFT:
               serv_type = ServicetypeType::AVC_HD_NVOD_TIMESHIFT;
               break;
            case IDTV::IService::ServiceType::AVC_HD_NVOD:
               serv_type = ServicetypeType::AVC_HD_NVOD;
               break;
            case IDTV::IService::ServiceType::UHD_TV:
               serv_type = ServicetypeType::UHD_TV;
               break;
            default:
               serv_type = ServicetypeType::UNKNOWN;
               break;
         }

         return(serv_type);
      }

      Core::JSON::EnumType<RunningstatusType> DTV::GetJsonRunningStatus(const IDTV::IService::RunState run_state) const
      {
         Core::JSON::EnumType<RunningstatusType> status;

         switch(run_state)
         {
            case IDTV::IService::RunState::RUN_STATE_NOT_RUNNING:
               status = RunningstatusType::NOTRUNNING;
               break;
            case IDTV::IService::RunState::RUN_STATE_STARTS_SOON:
               status = RunningstatusType::STARTSSOON;
               break;
            case IDTV::IService::RunState::RUN_STATE_PAUSING:
               status = RunningstatusType::PAUSING;
               break;
            case IDTV::IService::RunState::RUN_STATE_RUNNING:
               status = RunningstatusType::RUNNING;
               break;
            case IDTV::IService::RunState::RUN_STATE_OFF_AIR:
               status = RunningstatusType::OFFAIR;
               break;
            case IDTV::IService::RunState::RUN_STATE_UNDEFINED:
            default:
               status = RunningstatusType::UNDEFINED;
               break;
         }

         return(status);
      }

      Core::JSON::EnumType<LnbtypeType> DTV::GetJsonLnbType(const IDTV::ILnb::LnbType lnb_type) const
      {
         Core::JSON::EnumType<LnbtypeType> type;

         switch (lnb_type)
         {
            case IDTV::ILnb::LnbType::SINGLE:
               type = LnbtypeType::SINGLE;
               break;
            case IDTV::ILnb::LnbType::UNICABLE:
               type = LnbtypeType::UNICABLE;
               break;
            case IDTV::ILnb::LnbType::UNIVERSAL:
            default:
               type = LnbtypeType::UNIVERSAL;
               break;
         }

         return type;
      }

      Core::JSON::EnumType<LnbpowerType> DTV::GetJsonLnbPower(const IDTV::ILnb::LnbPower lnb_power) const
      {
         Core::JSON::EnumType<LnbpowerType> power;

         switch (lnb_power)
         {
            case IDTV::ILnb::LnbPower::POWER_OFF:
               power = LnbpowerType::OFF;
               break;
            case IDTV::ILnb::LnbPower::POWER_ON:
               power = LnbpowerType::ON;
               break;
            case IDTV::ILnb::LnbPower::POWER_AUTO:
            default:
               power = LnbpowerType::AUTO;
               break;
         }

         return power;
      }

      Core::JSON::EnumType<Diseqc_toneType> DTV::GetJsonDiseqcTone(const IDTV::ILnb::DiseqcTone diseqc_tone) const
      {
         Core::JSON::EnumType<Diseqc_toneType> tone;

         switch (diseqc_tone)
         {
            case IDTV::ILnb::DiseqcTone::TONE_A:
               tone = Diseqc_toneType::A;
               break;
            case IDTV::ILnb::DiseqcTone::TONE_B:
               tone = Diseqc_toneType::B;
               break;
            case IDTV::ILnb::DiseqcTone::TONE_OFF:
            default:
               tone = Diseqc_toneType::OFF;
               break;
         }

         return tone;
      }

      Core::JSON::EnumType<Diseqc_cswitchType> DTV::GetJsonDiseqcCSwitch(const IDTV::ILnb::DiseqcCSwitch diseqc_cswitch) const
      {
         Core::JSON::EnumType<Diseqc_cswitchType> cswitch;

         switch (diseqc_cswitch)
         {
            case IDTV::ILnb::DiseqcCSwitch::CS_A:
               cswitch = Diseqc_cswitchType::A;
               break;
            case IDTV::ILnb::DiseqcCSwitch::CS_B:
               cswitch = Diseqc_cswitchType::B;
               break;
            case IDTV::ILnb::DiseqcCSwitch::CS_C:
               cswitch = Diseqc_cswitchType::C;
               break;
            case IDTV::ILnb::DiseqcCSwitch::CS_D:
               cswitch = Diseqc_cswitchType::D;
               break;
            case IDTV::ILnb::DiseqcCSwitch::CS_OFF:
            default:
               cswitch = Diseqc_cswitchType::OFF;
               break;
         }

         return cswitch;
      }

      Core::JSON::EnumType<ComponentData::TypeType> DTV::GetJsonComponentType(const IDTV::IComponent::ComponentType type) const
      {
         Core::JSON::EnumType<ComponentData::TypeType> comp_type;

         switch (type)
         {
            case IDTV::IComponent::ComponentType::VIDEO:
               comp_type = ComponentData::TypeType::VIDEO;
               break;
            case IDTV::IComponent::ComponentType::AUDIO:
               comp_type = ComponentData::TypeType::AUDIO;
               break;
            case IDTV::IComponent::ComponentType::SUBTITLES:
               comp_type = ComponentData::TypeType::SUBTITLES;
               break;
            case IDTV::IComponent::ComponentType::TELETEXT:
               comp_type = ComponentData::TypeType::TELETEXT;
               break;
            case IDTV::IComponent::ComponentType::DATA:
            default:
               comp_type = ComponentData::TypeType::DATA;
               break;
         }

         return(comp_type);
      }

      Core::JSON::EnumType<CodecType> DTV::GetJsonCodecType(const IDTV::IComponent::CodecType type) const
      {
         Core::JSON::EnumType<CodecType> codec;

         switch (type)
         {
            case IDTV::IComponent::CodecType::MPEG2:
               codec = CodecType::MPEG2;
               break;
            case IDTV::IComponent::CodecType::H264:
               codec = CodecType::H264;
               break;
            case IDTV::IComponent::CodecType::H265:
               codec = CodecType::H265;
               break;
            case IDTV::IComponent::CodecType::AVS:
               codec = CodecType::AVS;
               break;
            case IDTV::IComponent::CodecType::MPEG1:
               codec = CodecType::MPEG1;
               break;
            case IDTV::IComponent::CodecType::AAC:
               codec = CodecType::AAC;
               break;
            case IDTV::IComponent::CodecType::HEAAC:
               codec = CodecType::HEAAC;
               break;
            case IDTV::IComponent::CodecType::HEAACV2:
               codec = CodecType::HEAACV2;
               break;
            case IDTV::IComponent::CodecType::AC3:
               codec = CodecType::AC3;
               break;
            case IDTV::IComponent::CodecType::EAC3:
               codec = CodecType::EAC3;
               break;
         }

         return(codec);
      }

      Core::JSON::EnumType<ComponentData::AudioData::TypeType> DTV::GetJsonAudioType(const IDTV::IComponent::AudType type) const
      {
         Core::JSON::EnumType<ComponentData::AudioData::TypeType> audio_type;

         switch (type)
         {
            case IDTV::IComponent::AudType::AUDIOTYPE_UNDEFINED:
               audio_type = ComponentData::AudioData::TypeType::UNDEFINED;
               break;
            case IDTV::IComponent::AudType::AUDIOTYPE_CLEAN:
               audio_type = ComponentData::AudioData::TypeType::CLEAN;
               break;
            case IDTV::IComponent::AudType::AUDIOTYPE_HEARINGIMPAIRED:
               audio_type = ComponentData::AudioData::TypeType::HEARINGIMPAIRED;
               break;
            case IDTV::IComponent::AudType::AUDIOTYPE_VISUALLYIMPAIRED:
               audio_type = ComponentData::AudioData::TypeType::VISUALLYIMPAIRED;
               break;
            default:
               audio_type = ComponentData::AudioData::TypeType::UNKNOWN;
               break;
         }

         return(audio_type);
      }

      Core::JSON::EnumType<ComponentData::AudioData::ModeType> DTV::GetJsonAudioMode(const IDTV::IComponent::AudMode mode) const
      {
         Core::JSON::EnumType<ComponentData::AudioData::ModeType> audio_mode;

         switch (mode)
         {
            case IDTV::IComponent::AudMode::AUDIOMODE_STEREO:
               audio_mode = ComponentData::AudioData::ModeType::STEREO;
               break;
            case IDTV::IComponent::AudMode::AUDIOMODE_LEFT:
               audio_mode = ComponentData::AudioData::ModeType::LEFT;
               break;
            case IDTV::IComponent::AudMode::AUDIOMODE_RIGHT:
               audio_mode = ComponentData::AudioData::ModeType::RIGHT;
               break;
            case IDTV::IComponent::AudMode::AUDIOMODE_MONO:
               audio_mode = ComponentData::AudioData::ModeType::MONO;
               break;
            case IDTV::IComponent::AudMode::AUDIOMODE_MULTICHANNEL:
               audio_mode = ComponentData::AudioData::ModeType::MULTICHANNEL;
               break;
            case IDTV::IComponent::AudMode::AUDIOMODE_UNDEFINED:
            default:
               audio_mode = ComponentData::AudioData::ModeType::UNDEFINED;
               break;
         }

         return(audio_mode);
      }

      Core::JSON::EnumType<ComponentData::SubtitlesData::FormatType> DTV::GetJsonSubtitleFormat(const IDTV::IComponent::SubFormat type) const
      {
         Core::JSON::EnumType<ComponentData::SubtitlesData::FormatType> format;

         switch (type)
         {
            case IDTV::IComponent::SubFormat::FORMAT_DEFAULT:
               format = ComponentData::SubtitlesData::FormatType::DEFAULT;
               break;
            case IDTV::IComponent::SubFormat::FORMAT_4_3:
               format = ComponentData::SubtitlesData::FormatType::E4_3;
               break;
            case IDTV::IComponent::SubFormat::FORMAT_16_9:
               format = ComponentData::SubtitlesData::FormatType::E16_9;
               break;
            case IDTV::IComponent::SubFormat::FORMAT_221_1:
               format = ComponentData::SubtitlesData::FormatType::E221_1;
               break;
            case IDTV::IComponent::SubFormat::FORMAT_HD:
               format = ComponentData::SubtitlesData::FormatType::HD;
               break;
            case IDTV::IComponent::SubFormat::FORMAT_HARDOFHEARING:
               format = ComponentData::SubtitlesData::FormatType::HARDOFHEARING;
               break;
            case IDTV::IComponent::SubFormat::FORMAT_HARDOFHEARING4_3:
               format = ComponentData::SubtitlesData::FormatType::HARDOFHEARING4_3;
               break;
            case IDTV::IComponent::SubFormat::FORMAT_HARDOFHEARING16_9:
               format = ComponentData::SubtitlesData::FormatType::HARDOFHEARING16_9;
               break;
            case IDTV::IComponent::SubFormat::FORMAT_HARDOFHEARING221_1:
               format = ComponentData::SubtitlesData::FormatType::HARDOFHEARING221_1;
               break;
            case IDTV::IComponent::SubFormat::FORMAT_HARDOFHEARINGHD:
               format = ComponentData::SubtitlesData::FormatType::HARDOFHEARINGHD;
               break;
         }

         return(format);
      }

      Core::JSON::EnumType<TunertypeType> DTV::GetJsonTunerType(const IDTV::TunerType tuner_type) const
      {
         Core::JSON::EnumType<TunertypeType> json_type;

         switch (tuner_type)
         {
            case IDTV::TunerType::DVBS:
               json_type = TunertypeType::DVBS;
               break;
            case IDTV::TunerType::DVBT:
               json_type = TunertypeType::DVBT;
               break;
            case IDTV::TunerType::DVBC:
               json_type = TunertypeType::DVBC;
               break;
            default:
               json_type = TunertypeType::NONE;
               break;
         }

         return(json_type);
      }

      Core::JSON::EnumType<PolarityType> DTV::GetJsonPolarity(const IDTV::IDvbsTuningParams::PolarityType polarity) const
      {
         Core::JSON::EnumType<PolarityType> json_polarity;

         switch (polarity)
         {
            case IDTV::IDvbsTuningParams::PolarityType::HORIZONTAL:
               json_polarity = PolarityType::HORIZONTAL;
               break;
            case IDTV::IDvbsTuningParams::PolarityType::VERTICAL:
               json_polarity = PolarityType::VERTICAL;
               break;
            case IDTV::IDvbsTuningParams::PolarityType::LEFT:
               json_polarity = PolarityType::LEFT;
               break;
            case IDTV::IDvbsTuningParams::PolarityType::RIGHT:
               json_polarity = PolarityType::RIGHT;
               break;
         }

         return(json_polarity);
      }

      Core::JSON::EnumType<FecType> DTV::GetJsonFec(const IDTV::IDvbsTuningParams::FecType fec) const
      {
         Core::JSON::EnumType<FecType> json_fec;

         switch (fec)
         {
            case IDTV::IDvbsTuningParams::FecType::FEC_AUTO:
               json_fec = FecType::FECAUTO;
               break;
            case IDTV::IDvbsTuningParams::FecType::FEC1_2:
               json_fec = FecType::FEC1_2;
               break;
            case IDTV::IDvbsTuningParams::FecType::FEC2_3:
               json_fec = FecType::FEC2_3;
               break;
            case IDTV::IDvbsTuningParams::FecType::FEC3_4:
               json_fec = FecType::FEC3_4;
               break;
            case IDTV::IDvbsTuningParams::FecType::FEC5_6:
               json_fec = FecType::FEC5_6;
               break;
            case IDTV::IDvbsTuningParams::FecType::FEC7_8:
               json_fec = FecType::FEC7_8;
               break;
            case IDTV::IDvbsTuningParams::FecType::FEC1_4:
               json_fec = FecType::FEC1_4;
               break;
            case IDTV::IDvbsTuningParams::FecType::FEC1_3:
               json_fec = FecType::FEC1_3;
               break;
            case IDTV::IDvbsTuningParams::FecType::FEC2_5:
               json_fec = FecType::FEC2_5;
               break;
            case IDTV::IDvbsTuningParams::FecType::FEC8_9:
               json_fec = FecType::FEC8_9;
               break;
            case IDTV::IDvbsTuningParams::FecType::FEC9_10:
               json_fec = FecType::FEC9_10;
               break;
            case IDTV::IDvbsTuningParams::FecType::FEC3_5:
               json_fec = FecType::FEC3_5;
               break;
            case IDTV::IDvbsTuningParams::FecType::FEC4_5:
               json_fec = FecType::FEC4_5;
               break;
         }

         return(json_fec);
      }

      Core::JSON::EnumType<DvbsmodulationType> DTV::GetJsonDvbsModulation(const IDTV::IDvbsTuningParams::ModulationType modulation) const
      {
         Core::JSON::EnumType<DvbsmodulationType> json_modulation;

         switch (modulation)
         {
            case IDTV::IDvbsTuningParams::ModulationType::MOD_QPSK:
               json_modulation = DvbsmodulationType::QPSK;
               break;
            case IDTV::IDvbsTuningParams::ModulationType::MOD_8PSK:
               json_modulation = DvbsmodulationType::E8PSK;
               break;
            case IDTV::IDvbsTuningParams::ModulationType::MOD_16QAM:
               json_modulation = DvbsmodulationType::E16QAM;
               break;
            case IDTV::IDvbsTuningParams::ModulationType::MOD_AUTO:
            default:
               json_modulation = DvbsmodulationType::AUTO;
               break;
         }

         return(json_modulation);
      }

      Core::JSON::EnumType<DvbcmodulationType> DTV::GetJsonDvbcModulation(const IDTV::IDvbcTuningParams::ModulationType modulation) const
      {
         Core::JSON::EnumType<DvbcmodulationType> json_modulation;

         switch (modulation)
         {
            case IDTV::IDvbcTuningParams::ModulationType::MOD_AUTO:
               json_modulation = DvbcmodulationType::AUTO;
               break;
            case IDTV::IDvbcTuningParams::ModulationType::MOD_4QAM:
               json_modulation = DvbcmodulationType::E4QAM;
               break;
            case IDTV::IDvbcTuningParams::ModulationType::MOD_8QAM:
               json_modulation = DvbcmodulationType::E8QAM;
               break;
            case IDTV::IDvbcTuningParams::ModulationType::MOD_16QAM:
               json_modulation = DvbcmodulationType::E16QAM;
               break;
            case IDTV::IDvbcTuningParams::ModulationType::MOD_32QAM:
               json_modulation = DvbcmodulationType::E32QAM;
               break;
            case IDTV::IDvbcTuningParams::ModulationType::MOD_64QAM:
               json_modulation = DvbcmodulationType::E64QAM;
               break;
            case IDTV::IDvbcTuningParams::ModulationType::MOD_128QAM:
               json_modulation = DvbcmodulationType::E128QAM;
               break;
            case IDTV::IDvbcTuningParams::ModulationType::MOD_256QAM:
               json_modulation = DvbcmodulationType::E256QAM;
               break;
         }

         return(json_modulation);
      }

      Core::JSON::EnumType<DvbtbandwidthType> DTV::GetJsonBandwidth(const IDTV::IDvbtTuningParams::BandwidthType bandwidth) const
      {
         Core::JSON::EnumType<DvbtbandwidthType> json_bandwidth;

         switch (bandwidth)
         {
            case IDTV::IDvbtTuningParams::BandwidthType::BW_5MHZ:
               json_bandwidth = DvbtbandwidthType::E5MHZ;
               break;
            case IDTV::IDvbtTuningParams::BandwidthType::BW_6MHZ:
               json_bandwidth = DvbtbandwidthType::E6MHZ;
               break;
            case IDTV::IDvbtTuningParams::BandwidthType::BW_7MHZ:
               json_bandwidth = DvbtbandwidthType::E7MHZ;
               break;
            case IDTV::IDvbtTuningParams::BandwidthType::BW_8MHZ:
               json_bandwidth = DvbtbandwidthType::E8MHZ;
               break;
            case IDTV::IDvbtTuningParams::BandwidthType::BW_10MHZ:
               json_bandwidth = DvbtbandwidthType::E10MHZ;
               break;
            case IDTV::IDvbtTuningParams::BandwidthType::BW_UNDEFINED:
            default:
               json_bandwidth = DvbtbandwidthType::UNDEFINED;
               break;
         }

         return(json_bandwidth);
      }

      Core::JSON::EnumType<OfdmmodeType> DTV::GetJsonOfdmMode(const IDTV::IDvbtTuningParams::OfdmModeType mode) const
      {
         Core::JSON::EnumType<OfdmmodeType> json_mode;

         switch (mode)
         {
            case IDTV::IDvbtTuningParams::OfdmModeType::OFDM_1K:
               json_mode = OfdmmodeType::OFDM_1K;
               break;
            case IDTV::IDvbtTuningParams::OfdmModeType::OFDM_2K:
               json_mode = OfdmmodeType::OFDM_2K;
               break;
            case IDTV::IDvbtTuningParams::OfdmModeType::OFDM_4K:
               json_mode = OfdmmodeType::OFDM_4K;
               break;
            case IDTV::IDvbtTuningParams::OfdmModeType::OFDM_8K:
               json_mode = OfdmmodeType::OFDM_8K;
               break;
            case IDTV::IDvbtTuningParams::OfdmModeType::OFDM_16K:
               json_mode = OfdmmodeType::OFDM_16K;
               break;
            case IDTV::IDvbtTuningParams::OfdmModeType::OFDM_32K:
               json_mode = OfdmmodeType::OFDM_32K;
               break;
            case IDTV::IDvbtTuningParams::OfdmModeType::OFDM_UNDEFINED:
            default:
               json_mode = OfdmmodeType::UNDEFINED;
               break;
         }

         return(json_mode);
      }

      IDTV::TunerType DTV::GetTunerType(Core::JSON::EnumType<TunertypeType> tuner_type) const
      {
         IDTV::TunerType signal;

         switch(tuner_type)
         {
            case TunertypeType::DVBS:
               signal = IDTV::TunerType::DVBS;
               break;
            case TunertypeType::DVBT:
               signal = IDTV::TunerType::DVBT;
               break;
            case TunertypeType::DVBC:
               signal = IDTV::TunerType::DVBC;
               break;
            default:
               signal = IDTV::TunerType::NONE;
               break;
         }

         return(signal);
      }

      IDTV::ILnb::LnbType DTV::GetLnbType(Core::JSON::EnumType<LnbtypeType> lnb_type) const
      {
         IDTV::ILnb::LnbType lnb;

         switch (lnb_type)
         {
            case LnbtypeType::SINGLE:
               lnb = IDTV::ILnb::LnbType::SINGLE;
               break;
            case LnbtypeType::UNICABLE:
               lnb = IDTV::ILnb::LnbType::UNICABLE;
               break;
            case LnbtypeType::UNIVERSAL:
            default:
               lnb = IDTV::ILnb::LnbType::UNIVERSAL;
               break;
         }

         return lnb;
      }

      IDTV::ILnb::LnbPower DTV::GetLnbPower(Core::JSON::EnumType<LnbpowerType> lnb_power) const
      {
         IDTV::ILnb::LnbPower power;

         switch (lnb_power)
         {
            case LnbpowerType::OFF:
               power = IDTV::ILnb::LnbPower::POWER_OFF;
               break;
            case LnbpowerType::ON:
               power = IDTV::ILnb::LnbPower::POWER_ON;
               break;
            case LnbpowerType::AUTO:
            default:
               power = IDTV::ILnb::LnbPower::POWER_AUTO;
               break;
         }

         return power;
      }

      IDTV::ILnb::DiseqcTone DTV::GetDiseqcTone(Core::JSON::EnumType<Diseqc_toneType> tone_type) const
      {
         IDTV::ILnb::DiseqcTone tone;

         switch (tone_type)
         {
            case Diseqc_toneType::A:
               tone = IDTV::ILnb::DiseqcTone::TONE_A;
               break;
            case Diseqc_toneType::B:
               tone = IDTV::ILnb::DiseqcTone::TONE_B;
               break;
            case Diseqc_toneType::OFF:
            default:
               tone = IDTV::ILnb::DiseqcTone::TONE_OFF;
               break;
         }

         return tone;
      }

      IDTV::ILnb::DiseqcCSwitch DTV::GetDiseqcCSwitch(Core::JSON::EnumType<Diseqc_cswitchType> switch_type) const
      {
         IDTV::ILnb::DiseqcCSwitch cswitch;

         switch (switch_type)
         {
            case Diseqc_cswitchType::A:
               cswitch = IDTV::ILnb::DiseqcCSwitch::CS_A;
               break;
            case Diseqc_cswitchType::B:
               cswitch = IDTV::ILnb::DiseqcCSwitch::CS_B;
               break;
            case Diseqc_cswitchType::C:
               cswitch = IDTV::ILnb::DiseqcCSwitch::CS_C;
               break;
            case Diseqc_cswitchType::D:
               cswitch = IDTV::ILnb::DiseqcCSwitch::CS_D;
               break;
            case Diseqc_cswitchType::OFF:
            default:
               cswitch = IDTV::ILnb::DiseqcCSwitch::CS_OFF;
               break;
         }

         return cswitch;
      }

      IDTV::IDvbsTuningParams::PolarityType DTV::GetDvbsPolarity(Core::JSON::EnumType<PolarityType> polarity_type) const
      {
         IDTV::IDvbsTuningParams::PolarityType polarity;

         switch (polarity_type)
         {
            case PolarityType::VERTICAL:
               polarity = IDTV::IDvbsTuningParams::PolarityType::VERTICAL;
               break;
            case PolarityType::LEFT:
               polarity = IDTV::IDvbsTuningParams::PolarityType::LEFT;
               break;
            case PolarityType::RIGHT:
               polarity = IDTV::IDvbsTuningParams::PolarityType::RIGHT;
               break;
            case PolarityType::HORIZONTAL:
            default:
               polarity = IDTV::IDvbsTuningParams::PolarityType::HORIZONTAL;
               break;
         }

         return(polarity);
      }

      IDTV::IDvbsTuningParams::FecType DTV::GetDvbsFEC(Core::JSON::EnumType<FecType> fec_type) const
      {
         IDTV::IDvbsTuningParams::FecType fec;

         switch (fec_type)
         {
            case FecType::FEC1_2:
               fec = IDTV::IDvbsTuningParams::FecType::FEC1_2;
               break;
            case FecType::FEC2_3:
               fec = IDTV::IDvbsTuningParams::FecType::FEC2_3;
               break;
            case FecType::FEC3_4:
               fec = IDTV::IDvbsTuningParams::FecType::FEC3_4;
               break;
            case FecType::FEC5_6:
               fec = IDTV::IDvbsTuningParams::FecType::FEC5_6;
               break;
            case FecType::FEC7_8:
               fec = IDTV::IDvbsTuningParams::FecType::FEC7_8;
               break;
            case FecType::FEC1_4:
               fec = IDTV::IDvbsTuningParams::FecType::FEC1_4;
               break;
            case FecType::FEC1_3:
               fec = IDTV::IDvbsTuningParams::FecType::FEC1_3;
               break;
            case FecType::FEC2_5:
               fec = IDTV::IDvbsTuningParams::FecType::FEC2_5;
               break;
            case FecType::FEC8_9:
               fec = IDTV::IDvbsTuningParams::FecType::FEC8_9;
               break;
            case FecType::FEC9_10:
               fec = IDTV::IDvbsTuningParams::FecType::FEC9_10;
               break;
            case FecType::FEC3_5:
               fec = IDTV::IDvbsTuningParams::FecType::FEC3_5;
               break;
            case FecType::FEC4_5:
               fec = IDTV::IDvbsTuningParams::FecType::FEC4_5;
               break;
            case FecType::FECAUTO:
            default:
               fec = IDTV::IDvbsTuningParams::FecType::FEC_AUTO;
               break;
         }

         return(fec);
      }

      IDTV::IDvbsTuningParams::ModulationType DTV::GetDvbsModulation(Core::JSON::EnumType<DvbsmodulationType> modulation_type) const
      {
         IDTV::IDvbsTuningParams::ModulationType modulation;

         switch(modulation_type)
         {
            case DvbsmodulationType::QPSK:
               modulation = IDTV::IDvbsTuningParams::ModulationType::MOD_QPSK;
               break;
            case DvbsmodulationType::E8PSK:
               modulation = IDTV::IDvbsTuningParams::ModulationType::MOD_8PSK;
               break;
            case DvbsmodulationType::E16QAM:
               modulation = IDTV::IDvbsTuningParams::ModulationType::MOD_16QAM;
               break;
            case DvbsmodulationType::AUTO:
            default:
               modulation = IDTV::IDvbsTuningParams::ModulationType::MOD_AUTO;
               break;
         }

         return(modulation);
      }

      IDTV::IDvbtTuningParams::BandwidthType DTV::GetDvbtBandwidth(Core::JSON::EnumType<DvbtbandwidthType> bandwidth_type) const
      {
         IDTV::IDvbtTuningParams::BandwidthType bwidth;

         switch (bandwidth_type)
         {
            case DvbtbandwidthType::E5MHZ:
               bwidth = IDTV::IDvbtTuningParams::BandwidthType::BW_5MHZ;
               break;
            case DvbtbandwidthType::E6MHZ:
               bwidth = IDTV::IDvbtTuningParams::BandwidthType::BW_6MHZ;
               break;
            case DvbtbandwidthType::E7MHZ:
               bwidth = IDTV::IDvbtTuningParams::BandwidthType::BW_7MHZ;
               break;
            case DvbtbandwidthType::E8MHZ:
               bwidth = IDTV::IDvbtTuningParams::BandwidthType::BW_8MHZ;
               break;
            case DvbtbandwidthType::E10MHZ:
               bwidth = IDTV::IDvbtTuningParams::BandwidthType::BW_10MHZ;
               break;
            case DvbtbandwidthType::UNDEFINED:
            default:
               bwidth = IDTV::IDvbtTuningParams::BandwidthType::BW_UNDEFINED;
               break;
         }

         return(bwidth);
      }

      IDTV::IDvbtTuningParams::OfdmModeType DTV::GetDvbtOfdmMode(Core::JSON::EnumType<OfdmmodeType> mode_type) const
      {
         IDTV::IDvbtTuningParams::OfdmModeType mode;

         switch (mode_type)
         {
            case OfdmmodeType::OFDM_1K:
               mode = IDTV::IDvbtTuningParams::OfdmModeType::OFDM_1K;
               break;
            case OfdmmodeType::OFDM_2K:
               mode = IDTV::IDvbtTuningParams::OfdmModeType::OFDM_2K;
               break;
            case OfdmmodeType::OFDM_4K:
               mode = IDTV::IDvbtTuningParams::OfdmModeType::OFDM_4K;
               break;
            case OfdmmodeType::OFDM_8K:
               mode = IDTV::IDvbtTuningParams::OfdmModeType::OFDM_8K;
               break;
            case OfdmmodeType::OFDM_16K:
               mode = IDTV::IDvbtTuningParams::OfdmModeType::OFDM_16K;
               break;
            case OfdmmodeType::OFDM_32K:
               mode = IDTV::IDvbtTuningParams::OfdmModeType::OFDM_32K;
               break;
            case OfdmmodeType::UNDEFINED:
            default:
               mode = IDTV::IDvbtTuningParams::OfdmModeType::OFDM_UNDEFINED;
               break;
         }

         return(mode);
      }

      IDTV::IDvbcTuningParams::ModulationType DTV::GetDvbcModulation(Core::JSON::EnumType<DvbcmodulationType> modulation_type) const
      {
         IDTV::IDvbcTuningParams::ModulationType modulation;

         switch(modulation_type)
         {
            case DvbcmodulationType::E4QAM:
               modulation = IDTV::IDvbcTuningParams::ModulationType::MOD_4QAM;
               break;
            case DvbcmodulationType::E8QAM:
               modulation = IDTV::IDvbcTuningParams::ModulationType::MOD_8QAM;
               break;
            case DvbcmodulationType::E16QAM:
               modulation = IDTV::IDvbcTuningParams::ModulationType::MOD_16QAM;
               break;
            case DvbcmodulationType::E32QAM:
               modulation = IDTV::IDvbcTuningParams::ModulationType::MOD_32QAM;
               break;
            case DvbcmodulationType::E64QAM:
               modulation = IDTV::IDvbcTuningParams::ModulationType::MOD_64QAM;
               break;
            case DvbcmodulationType::E128QAM:
               modulation = IDTV::IDvbcTuningParams::ModulationType::MOD_128QAM;
               break;
            case DvbcmodulationType::E256QAM:
               modulation = IDTV::IDvbcTuningParams::ModulationType::MOD_256QAM;
               break;
            case DvbcmodulationType::AUTO:
            default:
               modulation = IDTV::IDvbcTuningParams::ModulationType::MOD_AUTO;
               break;
         }

         return(modulation);
      }

      JsonObject DTV::CreateJsonForService(const IDTV::IService *service) const
      {
         JsonObject params;
         std::string str;
         IDTV::IService::ServiceType service_type;
         uint16_t lcn;
         bool bvalue;
         IDTV::IService::RunState run_state;

         service->Fullname(str);
         params["fullname"] = str;

         service->Shortname(str);
         params["shortname"] = str;

         service->Dvburi(str);
         params["dvburi"] = str;

         service->Type(service_type);
         params["servicetype"] = GetJsonServiceType(service_type).Data();

         service->Lcn(lcn);
         params["lcn"] = lcn;

         service->Scrambled(bvalue);
         params["scrambled"] = bvalue;

         service->HasCaDescriptor(bvalue);
         params["hascadescriptor"] = bvalue;

         service->Hidden(bvalue);
         params["hidden"] = bvalue;

         service->Selectable(bvalue);
         params["selectable"] = bvalue;

         service->RunningStatus(run_state);
         params["runningstatus"] = GetJsonRunningStatus(run_state).Data();

         return params;
      }

      JsonObject DTV::CreateJsonForEITEvent(const IDTV::IEitEvent *event) const
      {
         JsonObject params;
         std::string str;
         uint32_t value32;
         uint16_t value16;
         uint8_t value8;
         bool bvalue;
         IDTV::IEitEvent::IContentData::IIterator *iterator;
         IDTV::IEitEvent::IContentData *content_item;

         event->Name(str);
         params["name"] = str;

         event->StartTime(value32);
         params["starttime"] = value32;

         event->Duration(value32);
         params["duration"] = value32;

         event->EventId(value16);
         params["eventid"] = value16;

         event->ShortDescription(str);
         params["shortdescription"] = str;

         event->HasSubtitles(bvalue);
         params["hassubtitles"] = bvalue;

         event->HasAudioDescription(bvalue);
         params["hasaudiodescription"] = bvalue;

         event->ParentalRating(value8);
         params["parentalrating"] = value8;

         JsonArray array;
         event->ContentData(iterator);

         while (iterator->Current(content_item) == Core::ERROR_NONE)
         {
            content_item->Value(value8);
            array.Add(value8);
            iterator->Next();
         }

         params["contentdata"] = array;

         event->HasExtendedInfo(bvalue);
         params["hasextendedinfo"] = bvalue;

         return(params);
      }

      void DTV::ExtractDvbServiceInfo(ServiceInfo& info, const IDTV::IService *service) const
      {
         std::string str;
         IDTV::IService::ServiceType type;
         uint16_t lcn;
         bool bvalue;
         IDTV::IService::RunState run_state;

         service->Fullname(str);
         info.Fullname = str.c_str();

         service->Shortname(str);
         info.Shortname = str.c_str();

         service->Dvburi(str);
         info.Dvburi = str.c_str();

         service->Type(type);
         info.Servicetype = GetJsonServiceType(type);

         service->Lcn(lcn);
         info.Lcn = lcn;

         service->Scrambled(bvalue);
         info.Scrambled = bvalue;

         service->HasCaDescriptor(bvalue);
         info.Hascadescriptor = bvalue;

         service->Hidden(bvalue);
         info.Hidden = bvalue;

         service->Selectable(bvalue);
         info.Selectable = bvalue;

         service->RunningStatus(run_state);
         info.Runningstatus = GetJsonRunningStatus(run_state);
      }

      void DTV::ExtractDvbEventInfo(EiteventInfo& info, const IDTV::IEitEvent *event) const
      {
         std::string str;
         uint32_t value32;
         uint16_t value16;
         uint8_t value8;
         bool bvalue;
         IDTV::IEitEvent::IContentData::IIterator* iterator;
         IDTV::IEitEvent::IContentData* content_data;

         event->Name(str);
         info.Name = str.c_str();

         event->ShortDescription(str);
         info.Shortdescription = str.c_str();

         event->StartTime(value32);
         info.Starttime = value32;

         event->Duration(value32);
         info.Duration = value32;

         event->EventId(value16);
         info.Eventid = value16;

         event->HasSubtitles(bvalue);
         info.Hassubtitles = bvalue;

         event->HasAudioDescription(bvalue);
         info.Hasaudiodescription = bvalue;

         event->ParentalRating(value8);
         info.Parentalrating = value8;

         event->HasExtendedInfo(bvalue);
         info.Hasextendedinfo = bvalue;

         event->ContentData(iterator);
         info.Contentdata.Clear();

         while (iterator->Current(content_data) == Core::ERROR_NONE)
         {
            content_data->Value(value8);
            info.Contentdata.Add(value8);
            iterator->Next();
         }
      }

      void DTV::ExtractDvbStreamInfo(ComponentData& info, const IDTV::IComponent *component) const
      {
         IDTV::IComponent::ComponentType type;
         uint16_t pid;
         IDTV::IComponent::ITag::IIterator *iterator;
         IDTV::IComponent::ITag *tag;
         uint8_t value8;
         IDTV::IComponent::CodecType codec;
         std::string lang;

         component->Type(type);
         info.Type = GetJsonComponentType(type);

         component->Pid(pid);
         info.Pid = pid;

         component->Tags(iterator);
         info.Tags.Clear();

         while (iterator->Current(tag) == Core::ERROR_NONE)
         {
            tag->Tag(value8);
            info.Tags.Add(value8);
            iterator->Next();
         }

         if (type == IDTV::IComponent::ComponentType::VIDEO)
         {
            component->Codec(codec);
            info.Video.Codec = GetJsonCodecType(codec);

            info.Audio.Clear();
            info.Subtitles.Clear();
            info.Teletext.Clear();
         }
         else if (type == IDTV::IComponent::ComponentType::AUDIO)
         {
            IDTV::IComponent::AudType audio_type;
            IDTV::IComponent::AudMode audio_mode;

            component->Codec(codec);
            info.Audio.Codec = GetJsonCodecType(codec);

            component->Language(lang);
            info.Audio.Language = lang.c_str();

            component->AudioType(audio_type);
            info.Audio.Type = GetJsonAudioType(audio_type);

            component->AudioMode(audio_mode);
            info.Audio.Mode = GetJsonAudioMode(audio_mode);

            info.Video.Clear();
            info.Subtitles.Clear();
            info.Teletext.Clear();
         }
         else if (type == IDTV::IComponent::ComponentType::SUBTITLES)
         {
            IDTV::IComponent::SubFormat format;
            uint16_t page;

            component->Language(lang);
            info.Subtitles.Language = lang.c_str();

            component->SubtitleFormat(format);
            info.Subtitles.Format = GetJsonSubtitleFormat(format);

            component->CompositionPage(page);
            info.Subtitles.Compositionpage = page;

            component->AncillaryPage(page);
            info.Subtitles.Ancillarypage = page;

            info.Video.Clear();
            info.Audio.Clear();
            info.Teletext.Clear();
         }
         else if (type == IDTV::IComponent::ComponentType::TELETEXT)
         {
            uint8_t value8;

            component->Language(lang);
            info.Teletext.Language = lang.c_str();

            component->TeletextType(value8);
            info.Teletext.Type = value8;

            component->TeletextMagazine(value8);
            info.Teletext.Magazine = value8;

            component->TeletextPage(value8);
            info.Teletext.Page = value8;

            info.Video.Clear();
            info.Audio.Clear();
            info.Subtitles.Clear();
         }
      }

      void DTV::ExtractDvbTransportInfo(TransportInfo& info, const IDTV::ITransport *transport) const
      {
         IDTV::TunerType tuner_type;
         uint16_t value16;
         uint8_t value8;

         transport->Tuner(tuner_type);
         info.Tunertype = GetJsonTunerType(tuner_type);

         transport->OriginalNetworkId(value16);
         info.Originalnetworkid = value16;

         transport->TransportId(value16);
         info.Transportid = value16;

         transport->SignalStrength(value8);
         info.Strength = value8;

         transport->SignalQuality(value8);
         info.Quality = value8;

         switch (tuner_type)
         {
            case IDTV::TunerType::DVBS:
               ExtractDvbsTuningParams(info.Dvbstuningparams, transport);
               info.Dvbctuningparams.Clear();
               info.Dvbttuningparams.Clear();
               break;
            case IDTV::TunerType::DVBC:
               ExtractDvbcTuningParams(info.Dvbctuningparams, transport);
               info.Dvbstuningparams.Clear();
               info.Dvbttuningparams.Clear();
               break;
            case IDTV::TunerType::DVBT:
               ExtractDvbtTuningParams(info.Dvbttuningparams, transport);
               info.Dvbstuningparams.Clear();
               info.Dvbctuningparams.Clear();
               break;
            default:
               info.Dvbstuningparams.Clear();
               info.Dvbctuningparams.Clear();
               info.Dvbttuningparams.Clear();
               break;
         }
      }

      void DTV::ExtractDvbsTuningParams(DvbstuningparamsInfo& tuning_params, const IDTV::ITransport *transport) const
      {
         IDTV::IDvbsTuningParams *dvbs;

         if (transport->TuningParamsDvbs(dvbs) == Core::ERROR_NONE)
         {
            uint32_t freq;
            IDTV::IDvbsTuningParams::PolarityType polarity;
            uint16_t symbol_rate;
            IDTV::IDvbsTuningParams::FecType fec;
            IDTV::IDvbsTuningParams::ModulationType modulation;
            bool dvbs2;
            std::string sat_name;

            dvbs->Frequency(freq);
            tuning_params.Frequency = freq;

            dvbs->Polarity(polarity);
            tuning_params.Polarity = GetJsonPolarity(polarity);

            dvbs->SymbolRate(symbol_rate);
            tuning_params.Symbolrate = symbol_rate;

            dvbs->Fec(fec);
            tuning_params.Fec = GetJsonFec(fec);

            dvbs->Modulation(modulation);
            tuning_params.Modulation = GetJsonDvbsModulation(modulation);

            dvbs->DvbS2(dvbs2);
            tuning_params.Dvbs2 = dvbs2;

            dvbs->Satellite(sat_name);
            tuning_params.Satellite = sat_name.c_str();
         }
      }

      void DTV::ExtractDvbcTuningParams(DvbctuningparamsInfo& tuning_params, const IDTV::ITransport *transport) const
      {
         IDTV::IDvbcTuningParams *dvbc;

         if (transport->TuningParamsDvbc(dvbc) == Core::ERROR_NONE)
         {
            uint32_t freq;
            uint16_t symbol_rate;
            IDTV::IDvbcTuningParams::ModulationType modulation;

            dvbc->Frequency(freq);
            tuning_params.Frequency = freq;

            dvbc->SymbolRate(symbol_rate);
            tuning_params.Symbolrate = symbol_rate;

            dvbc->Modulation(modulation);
            tuning_params.Modulation = GetJsonDvbcModulation(modulation);
         }
      }

      void DTV::ExtractDvbtTuningParams(DvbttuningparamsInfo& tuning_params, const IDTV::ITransport *transport) const
      {
         IDTV::IDvbtTuningParams *dvbt;

         if (transport->TuningParamsDvbt(dvbt) == Core::ERROR_NONE)
         {
            uint32_t freq;
            IDTV::IDvbtTuningParams::BandwidthType bandwidth;
            IDTV::IDvbtTuningParams::OfdmModeType ofdm;
            bool dvbt2;
            uint8_t plp_id;

            dvbt->Frequency(freq);
            tuning_params.Frequency = freq;

            dvbt->Bandwidth(bandwidth);
            tuning_params.Bandwidth = GetJsonBandwidth(bandwidth);

            dvbt->OfdmMode(ofdm);
            tuning_params.Mode = GetJsonOfdmMode(ofdm);

            dvbt->DvbT2(dvbt2);
            tuning_params.Dvbt2 = dvbt2;

            dvbt->PlpId(plp_id);
            tuning_params.Plpid = plp_id;
         }
      }
   }
}

