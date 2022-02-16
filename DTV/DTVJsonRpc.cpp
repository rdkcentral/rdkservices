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
#include "DTV.h"
#include <interfaces/json/JsonData_DTV.h>

extern "C"
{
   // DVB include files
   #include <stbuni.h>
   #include <stbdpc.h>
   #include <stbsitab.h>

   #include <app.h>
   #include <ap_cfg.h>
   #include <ap_cntrl.h>
};

namespace WPEFramework
{
   namespace Plugin
   {
      using namespace JsonData::DTV;

      // Registration
      //

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

      // API implementation
      //

      // Property: numberOfCountries - get number of country configurations
      // Return codes:
      //  - ERROR_NONE: Success
      uint32_t DTV::GetNumberOfCountries(Core::JSON::DecUInt8 &response) const
      {
         SYSLOG(Logging::Notification, (_T("DTV::GetNumberOfCountries")));
         response = ACFG_GetNumCountries();
         return (Core::ERROR_NONE);
      }

      // Property: countryList - get list of available country configurations
      // Return codes:
      //  - ERROR_NONE: Success
      uint32_t DTV::GetCountryList(Core::JSON::ArrayType<CountryconfigData>& response) const
      {
         U8BIT num_countries = 0;
         U8BIT **country_names = NULL;

         ACFG_GetCountryList(&country_names, &num_countries);

         if ((num_countries != 0) && (country_names != NULL))
         {
            for (U8BIT index = 0; index < num_countries; index++)
            {
               CountryconfigData config;

               config.Name = Core::ToString((char *)country_names[index]);
               config.Code = ACFG_GetCountryCode(index);

               response.Add(config);
            }

            ACFG_ReleaseCountryList(country_names, num_countries);
         }

         return (Core::ERROR_NONE);
      }

      // Property: country - get the ISO 3-character country code for the configured country
      // Return codes:
      //  - ERROR_NONE: Success
      uint32_t DTV::GetCountry(Core::JSON::DecUInt32 &response) const
      {
         SYSLOG(Logging::Notification, (_T("DTV::GetCountry")));
         response = ACFG_GetCountry();
         return (Core::ERROR_NONE);
      }

      // Property: country - set configured country using the ISO 3-character country code
      // Return codes:
      //  - ERROR_NONE: Success
      //  - ERROR_BAD_REQUEST: country code isn't recognised by DVB stack
      uint32_t DTV::SetCountry(Core::JSON::DecUInt32 code) const
      {
         uint32_t result = Core::ERROR_BAD_REQUEST;

         SYSLOG(Logging::Notification, (_T("DTV::SetCountry: code=%u"), (uint32_t)code));
         if (ACFG_SetCountry(code))
         {
            result = Core::ERROR_NONE;
         }

         return(result);
      }

      // Property: lnbList - get LNB list
      // Return codes:
      //  - ERROR_NONE: Success
      uint32_t DTV::GetLnbList(Core::JSON::ArrayType<LnbsettingsInfo>& response) const
      {
         ADB_LNB_SETTINGS settings;

         SYSLOG(Logging::Notification, (_T("DTV::GetLnbList")));

         void *lnb_ptr = ADB_GetNextLNB(NULL);
         while (lnb_ptr != NULL)
         {
            LnbsettingsInfo lnb;

            ADB_GetLNBSettings(lnb_ptr, &settings);

            SetJsonString(settings.name, lnb.Name);
            lnb.Type = GetJsonLnbType(settings.type);
            lnb.Power = GetJsonLnbPower(settings.power);
            lnb.Diseqc_tone = GetJsonDiseqcTone(settings.diseqc_tone);
            lnb.Diseqc_cswitch = GetJsonDiseqcCSwitch(settings.c_switch);
            lnb.Is22k = (settings.is_22k ? true : false);
            lnb.Is12v = (settings.is_12v ? true : false);
            lnb.Ispulseposition = (settings.is_pulse_posn ? true : false);
            lnb.Isdiseqcposition = (settings.is_diseqc_posn ? true : false);
            lnb.Issmatv = (settings.is_smatv ? true : false);
            lnb.Diseqcrepeats = settings.diseqc_repeats;
            lnb.U_switch = settings.u_switch;
            lnb.Unicablechannel = settings.unicable_chan;
            lnb.Unicablefreq = settings.unicable_if;

            SYSLOG(Logging::Notification, (_T("DTV::GetLnbList: adding %s"), lnb.Name.Value().c_str()));

            response.Add(lnb);

            lnb_ptr = ADB_GetNextLNB(lnb_ptr);
         }

         return (Core::ERROR_NONE);
      }

      // Property: satelliteList - get satellite list
      // Return codes:
      //  - ERROR_NONE: Success
      uint32_t DTV::GetSatelliteList(Core::JSON::ArrayType<SatellitesettingsInfo>& response) const
      {
         U16BIT longitude;
         void *lnb_ptr;

         SYSLOG(Logging::Notification, (_T("DTV::GetSatelliteList")));

         void *sat_ptr = ADB_GetNextSatellite(NULL);
         while (sat_ptr != NULL)
         {
            SatellitesettingsInfo sat;

            sat.Name = Core::ToString((char *)ADB_GetSatelliteName(sat_ptr));

            longitude = ADB_GetSatelliteLongitude(sat_ptr);

            if (ADB_GetSatelliteDirection(sat_ptr))
            {
               // Longitude value is east, so the value is positive
               sat.Longitude = longitude;
            }
            else
            {
               sat.Longitude = -longitude;
            }

            if ((lnb_ptr = ADB_GetSatelliteLNB(sat_ptr)) != NULL)
            {
               ADB_LNB_SETTINGS settings;

               if (ADB_GetLNBSettings(lnb_ptr, &settings) && (settings.name != NULL))
               {
                  sat.Lnb = Core::ToString((char *)settings.name);
               }
            }

            SYSLOG(Logging::Notification, (_T("DTV::GetSatelliteList: adding %s, LNB %s"),
               sat.Name.Value().c_str(), sat.Lnb.Value().c_str()));

            response.Add(sat);

            sat_ptr = ADB_GetNextSatellite(sat_ptr);
         }

         return (Core::ERROR_NONE);
      }

      // Property: numberOfServices - get total number of services in the service database
      // Return codes:
      //  - ERROR_NONE: Success
      uint32_t DTV::GetNumberOfServices(Core::JSON::DecUInt16 &response) const
      {
         SYSLOG(Logging::Notification, (_T("DTV::GetNumberOfServices")));
         response = ADB_GetNumServices();
         return (Core::ERROR_NONE);
      }

      // Property: serviceList - get service list
      // Return codes:
      //  - ERROR_NONE: Success
      uint32_t DTV::GetServiceList(const string& index, Core::JSON::ArrayType<ServiceInfo>& response) const
      {
         U16BIT onet_id, trans_id;
         void *transport;
         E_STB_DP_SIGNAL_TYPE signal = SIGNAL_NONE;
         U16BIT num_services = 0;
         void **slist = NULL;

         SYSLOG(Logging::Notification, (_T("DTV::GetServiceList: %s"), index.c_str()));

         // The index may be a doublet specifying a transport, or a tuner type
         int num_args = std::sscanf(index.c_str(), "%hu.%hu", &onet_id, &trans_id);
         if (num_args == 2)
         {
            transport = ADB_GetTransportFromIds(ADB_INVALID_DVB_ID, onet_id, trans_id);
            if (transport != NULL)
            {
               ADB_GetTransportServiceList(transport, &slist, &num_services);
            }
         }
         else
         {
            // Check for a tuner type, otherwise all services will be returned
            switch(Core::EnumerateType<TunertypeType>(index.c_str()).Value())
            {
               case TunertypeType::DVBS:
                  signal = SIGNAL_QPSK;
                  break;
               case TunertypeType::DVBT:
                  signal = SIGNAL_COFDM;
                  break;
               case TunertypeType::DVBC:
                  signal = SIGNAL_QAM;
                  break;
               default:
                  signal = SIGNAL_NONE;
                  break;
            }

            ADB_GetServiceList(ADB_SERVICE_LIST_DIGITAL, &slist, &num_services);
         }

         if ((slist != NULL) && (num_services != 0))
         {
            BOOLEAN is_sig2;

            for (U16BIT index = 0; index < num_services; index++)
            {
               if ((signal == SIGNAL_NONE) || (ADB_GetServiceSignalType(slist[index], &is_sig2) == signal))
               {
                  ServiceInfo service;

                  ExtractDvbServiceInfo(service, slist[index]);
                  response.Add(service);
               }
            }

            ADB_ReleaseServiceList(slist, num_services);
         }

         return (Core::ERROR_NONE);
      }

      // Property: nowNextEvents - get the now & next EIT events for the defined service
      // Return codes:
      //  - ERROR_NONE: Success
      //  - ERROR_BAD_REQUEST: invalid service URI or service can't be found
      uint32_t DTV::GetNowNextEvents(const string& service_uri, NowNextEventsData& response) const
      {
         uint32_t result = Core::ERROR_BAD_REQUEST;

         SYSLOG(Logging::Notification, (_T("DTV::GetNowNextEvents: %s"), service_uri.c_str()));

         if (service_uri.length() != 0)
         {
            U16BIT onet_id, trans_id, serv_id;

            if (std::sscanf(service_uri.c_str(), "%hu.%hu.%hu", &onet_id, &trans_id, &serv_id) == 3)
            {
               void *service = ADB_FindServiceByIds(onet_id, trans_id, serv_id);
               if (service != NULL)
               {
                  void *now;
                  void *next;

                  ADB_GetNowNextEvents(service, &now, &next);

                  if (now != NULL)
                  {
                     ExtractDvbEventInfo(response.Now, now);
                     ADB_ReleaseEventData(now);
                  }
                  else
                  {
                     response.Now.Starttime = 0;
                  }

                  if (next != NULL)
                  {
                     ExtractDvbEventInfo(response.Next, next);
                     ADB_ReleaseEventData(next);
                  }
                  else
                  {
                     response.Next.Starttime = 0;
                  }

                  result = Core::ERROR_NONE;
               }
            }
         }

         return (result);
      }

      // Property: scheduleEvents - get the schedule EIT events for the given service.
      //                            The service and optional start and end times should be provided
      //                            as in index in the format "@o.t.s:start,end", where o, t & s are
      //                            the original network, transport and service IDs of the service in
      //                            decimal, and start and end are start and end times in UTC time in seconds
      //                            such that any events that start within this period will be returned.
      // Return codes:
      //  - ERROR_NONE: Success
      //  - ERROR_BAD_REQUEST: invalid service URI or service can't be found
      uint32_t DTV::GetScheduleEvents(const string& index, Core::JSON::ArrayType<EiteventInfo>& response) const
      {
         uint32_t result = Core::ERROR_BAD_REQUEST;

         SYSLOG(Logging::Notification, (_T("DTV::GetScheduleEvents: %s"), index.c_str()));

         if (index.length() != 0)
         {
            U16BIT onet_id, trans_id, serv_id;
            U32BIT start_utc = 0;
            U32BIT end_utc = 0xffffffff;

            int num_args = std::sscanf(index.c_str(), "%hu.%hu.%hu:%u,%u", &onet_id, &trans_id, &serv_id,
               &start_utc, &end_utc);

            if (num_args >= 3)
            {
               void *service = ADB_FindServiceByIds(onet_id, trans_id, serv_id);
               if (service != NULL)
               {
                  void **event_list;
                  U16BIT num_events;

                  ADB_GetEventSchedule(FALSE, service, &event_list, &num_events);
                  if (event_list != NULL)
                  {
                     U32BIT start_time;

                     for (U16BIT i = 0; i < num_events; i++)
                     {
                        start_time = STB_GCConvertToTimestamp(ADB_GetEventStartDateTime(event_list[i]));
                        if ((start_time >= start_utc) && (start_time <= end_utc))
                        {
                           EiteventInfo event;

                           ExtractDvbEventInfo(event, event_list[i]);
                           response.Add(event);
                        }
                        else if (start_time > end_utc)
                        {
                           /* Events are provided in increasing date/time order so all events after
                            * this will be outside of the requested window and don't need to be checked */
                           break;
                        }
                     }

                     ADB_ReleaseEventList(event_list, num_events);
                  }

                  result = Core::ERROR_NONE;
               }
            }
         }

         return (result);
      }

      // Property: status - get the status for the given decode path
      // Return codes:
      //  - ERROR_NONE: Success
      //  - ERROR_BAD_REQUEST: invalid play handle
      uint32_t DTV::GetStatus(const string& index, StatusData& response) const
      {
         uint32_t result = Core::ERROR_BAD_REQUEST;

         if (index.length() != 0)
         {
            U8BIT path = (U8BIT)atoi(index.c_str());

            if (path < STB_DPGetNumPaths())
            {
               response.Tuner = STB_DPGetPathTuner(path);
               response.Demux = STB_DPGetPathDemux(path);

               void *service = ADB_GetTunedService(path);
               if (service != NULL)
               {
                  U16BIT onet_id, trans_id, serv_id;

                  response.Pmtpid = ADB_GetServicePmtPid(service);
                  response.Lcn = ADB_GetServiceLcn(service);

                  ADB_GetServiceIds(service, &onet_id, &trans_id, &serv_id);
                  response.Dvburi = std::to_string(onet_id) + "." + std::to_string(trans_id) +
                     "." + std::to_string(serv_id);
               }
               else
               {
                  response.Pmtpid = DVB_INVALID_ID;
                  response.Lcn = 0;
                  response.Dvburi = _T("");
               }

               result = Core::ERROR_NONE;
            }
         }

         return (result);
      }

      /************************
       * Version 2 properties
       ************************/

      // Property: serviceInfo - get service info
      // Return codes:
      //  - ERROR_NONE: Success
      //  - ERROR_BAD_REQUEST: invalid service URI or service can't be found
      uint32_t DTV::GetServiceInfo(const string& index, ServiceInfo& response) const
      {
         uint32_t result = Core::ERROR_BAD_REQUEST;
         U16BIT onet_id, trans_id, serv_id;

         SYSLOG(Logging::Notification, (_T("DTV::GetServiceInfo: %s"), index.c_str()));

         // The index must be a service's DVB triplet
         int num_args = std::sscanf(index.c_str(), "%hu.%hu.%hu", &onet_id, &trans_id, &serv_id);
         if (num_args == 3)
         {
            void *service = ADB_FindServiceByIds(onet_id, trans_id, serv_id);
            if (service != NULL)
            {
               ExtractDvbServiceInfo(response, service);
               result = Core::ERROR_NONE;
            }
         }

         return (result);
      }

      // Property: serviceComponents - get service components
      // Return codes:
      //  - ERROR_NONE: Success
      //  - ERROR_BAD_REQUEST: invalid service URI or service can't be found
      uint32_t DTV::GetServiceComponents(const string& index, Core::JSON::ArrayType<ComponentData>& response) const
      {
         uint32_t result = Core::ERROR_BAD_REQUEST;
         U16BIT onet_id, trans_id, serv_id;

         SYSLOG(Logging::Notification, (_T("DTV::GetServiceComponents: %s"), index.c_str()));

         // The index must be a service's DVB triplet
         int num_args = std::sscanf(index.c_str(), "%hu.%hu.%hu", &onet_id, &trans_id, &serv_id);
         if (num_args == 3)
         {
            void *service = ADB_FindServiceByIds(onet_id, trans_id, serv_id);
            if (service != NULL)
            {
               void **streams;
               U16BIT num_streams;

               ADB_GetStreamList(service, ADB_STREAM_LIST_ALL, &streams, &num_streams);

               if ((streams != NULL) && (num_streams != 0))
               {
                  for (U16BIT index = 0; index < num_streams; index++)
                  {
                     ComponentData component;
                     ExtractDvbStreamInfo(component, streams[index]);
                     response.Add(component);
                  }

                  ADB_ReleaseStreamList(streams, num_streams);
               }

               result = Core::ERROR_NONE;
            }
         }

         return (result);
      }

      // Property: transportInfo - get transport info
      // Return codes:
      //  - ERROR_NONE: Success
      //  - ERROR_BAD_REQUEST: invalid transport URI or transport can't be found
      uint32_t DTV::GetTransportInfo(const string& index, TransportInfo& response) const
      {
         uint32_t result = Core::ERROR_BAD_REQUEST;
         U16BIT onet_id, trans_id;

         SYSLOG(Logging::Notification, (_T("DTV::GetTransportInfo: %s"), index.c_str()));

         // The index must be a transport's DVB doublet
         int num_args = std::sscanf(index.c_str(), "%hu.%hu", &onet_id, &trans_id);
         if (num_args == 2)
         {
            void *transport = ADB_GetTransportFromIds(ADB_INVALID_DVB_ID, onet_id, trans_id);
            if (transport != NULL)
            {
               ExtractDvbTransportInfo(response, transport);
               result = Core::ERROR_NONE;
            }
         }

         return (result);
      }

      // Property: extendedEventInfo - get extended EIT info for an event
      // Return codes:
      //  - ERROR_NONE: Success
      //  - ERROR_BAD_REQUEST: invalid service URI, service can't be found, or event can't be found
      uint32_t DTV::GetExtendedEventInfo(const string& index, ExtendedeventinfoData& response) const
      {
         uint32_t result = Core::ERROR_BAD_REQUEST;
         U16BIT onet_id, trans_id, serv_id, event_id;

         SYSLOG(Logging::Notification, (_T("DTV::GetExtendedEventInfo: %s"), index.c_str()));

         // The index must be a service's DVB triplet followed by the event ID
         int num_args = std::sscanf(index.c_str(), "%hu.%hu.%hu:%hu", &onet_id, &trans_id,
            &serv_id, &event_id);
         if (num_args == 4)
         {
            void *service = ADB_FindServiceByIds(onet_id, trans_id, serv_id);
            if (service != NULL)
            {
               void *event = ADB_GetEvent(service, event_id);
               if (event != NULL)
               {
                  U16BIT num_items;

                  SetJsonString(ADB_GetEventExtendedDescription(event), response.Description, true);

                  ADB_EVENT_ITEMIZED_INFO* items = ADB_GetEventItemizedDescription(event, &num_items);
                  if ((items != NULL) && (num_items != 0))
                  {
                     for (U16BIT num = 0; num < num_items; num++)
                     {
                        ExtendedeventinfoData::ExtendedeventitemData ex_item;

                        SetJsonString(items[num].item_description, ex_item.Description, false);
                        SetJsonString(items[num].item, ex_item.Item, false);

                        response.Items.Add(ex_item);
                     }

                     ADB_ReleaseEventItemizedInfo(items, num_items);
                  }

                  ADB_ReleaseEventData(event);

                  result = Core::ERROR_NONE;
               }
            }
         }

         return(result);
      }

      // Property: signalInfo - locked status, strength and quality for the tuned play handle
      // Return codes:
      //  - ERROR_NONE: Success
      //  - ERROR_BAD_REQUEST: no play handle given
      uint32_t DTV::GetSignalInfo(const string& index, SignalInfoData& response) const
      {
         uint32_t result = Core::ERROR_BAD_REQUEST;

         SYSLOG(Logging::Notification, (_T("DTV::GetSignalInfo: %s"), index.c_str()));

         if (index.length() != 0)
         {
            U8BIT path = (U8BIT)atoi(index.c_str());
            U8BIT tuner = STB_DPGetPathTuner(path);

            response.Locked = (ACTL_IsTuned(path) ? true : false);

            if (response.Locked && (tuner != INVALID_RES_ID))
            {
               response.Strength = STB_TuneGetSignalStrength(tuner);
               response.Quality = STB_TuneGetDataIntegrity(tuner);
            }
            else
            {
               void *transport = ADB_GetTunedTransport(path);

               response.Strength = ADB_GetTransportTunedStrength(transport);
               response.Quality = ADB_GetTransportTunedQuality(transport);
            }

            result = Core::ERROR_NONE;
         }

         return(result);
      }

      uint32_t DTV::AddLnb(const LnbsettingsInfo& lnb_settings, Core::JSON::Boolean& response)
      {
         uint32_t result = Core::ERROR_BAD_REQUEST;
         ADB_LNB_SETTINGS settings;

         SYSLOG(Logging::Notification, (_T("DTV::AddLnb: %s"), lnb_settings.Name.Value().c_str()));

         response = false;

         memset(&settings, 0, sizeof(ADB_LNB_SETTINGS));

         string name(lnb_settings.Name.Value().c_str());

         settings.name = (U8BIT *)name.c_str();
         settings.type = GetDvbLnbType(lnb_settings.Type);
         settings.power = GetDvbLnbPower(lnb_settings.Power);
         settings.diseqc_tone = GetDvbDiseqcTone(lnb_settings.Diseqc_tone);
         settings.c_switch = GetDvbDiseqcCSwitch(lnb_settings.Diseqc_cswitch);
         settings.is_22k = (lnb_settings.Is22k ? TRUE : FALSE);
         settings.is_12v = (lnb_settings.Is12v ? TRUE : FALSE);
         settings.is_pulse_posn = (lnb_settings.Ispulseposition ? TRUE : FALSE);
         settings.is_diseqc_posn = (lnb_settings.Isdiseqcposition ? TRUE : FALSE);
         settings.is_smatv = (lnb_settings.Issmatv ? TRUE : FALSE);
         settings.diseqc_repeats = lnb_settings.Diseqcrepeats;
         settings.u_switch = lnb_settings.U_switch;
         settings.unicable_chan = lnb_settings.Unicablechannel;
         settings.unicable_if = lnb_settings.Unicablefreq;

         if (ADB_AddLNB(&settings) != NULL)
         {
            response = true;
            result = Core::ERROR_NONE;
         }

         return(result);
      }

      uint32_t DTV::AddSatellite(const SatellitesettingsInfo& sat_settings, Core::JSON::Boolean& response)
      {
         uint32_t result = Core::ERROR_BAD_REQUEST;

         SYSLOG(Logging::Notification, (_T("DTV::AddSatellite: %s, lnb=%s"), sat_settings.Name.Value().c_str(),
            sat_settings.Lnb.Value().c_str()));

         response = false;

         if (sat_settings.Lnb.IsSet())
         {
            U16BIT longitude;
            BOOLEAN east_west;

            if (sat_settings.Longitude < 0)
            {
               longitude = -sat_settings.Longitude;
               east_west = FALSE;
            }
            else
            {
               longitude = sat_settings.Longitude;
               east_west = TRUE;
            }

            // Find the LNB
            ADB_LNB_SETTINGS settings;

            void *lnb_ptr = ADB_GetNextLNB(NULL);
            while (lnb_ptr != NULL)
            {
               if (ADB_GetLNBSettings(lnb_ptr, &settings))
               {
                  if (settings.name != NULL)
                  {
                     if (strcmp(sat_settings.Lnb.Value().c_str(), (char *)settings.name) == 0)
                     {
                        // Found the LNB
                        break;
                     }
                  }
               }

               lnb_ptr = ADB_GetNextLNB(lnb_ptr);
            }

            if (lnb_ptr != NULL)
            {
               if (ADB_AddSatellite((U8BIT *)sat_settings.Name.Value().c_str(), 0, longitude, east_west, lnb_ptr) != NULL)
               {
                  response = true;
                  result = Core::ERROR_NONE;
               }
            }
         }

         return(result);
      }

      uint32_t DTV::StartServiceSearch(const StartServiceSearchParamsData& start_search, Core::JSON::Boolean& response)
      {
         uint32_t result = Core::ERROR_BAD_REQUEST;
         E_STB_DP_SIGNAL_TYPE signal = GetDvbSignalType(start_search.Tunertype);

         response = false;

         if (signal != SIGNAL_NONE)
         {
            S_MANUAL_TUNING_PARAMS tuning_params;
            bool manual_search;

            if (start_search.Usetuningparams)
            {
               manual_search = true;

               memset(&tuning_params, 0, sizeof(S_MANUAL_TUNING_PARAMS));

               switch(signal)
               {
                  case SIGNAL_QPSK:
                     SYSLOG(Logging::Notification, (_T("DTV::StartServiceSearch: manual DVB-S")));
                     tuning_params.u.sat.satellite = FindSatellite(start_search.Dvbstuningparams.Satellite.Value().c_str());
                     tuning_params.freq = start_search.Dvbstuningparams.Frequency;
                     tuning_params.u.sat.polarity = GetDvbPolarity(start_search.Dvbstuningparams.Polarity);
                     tuning_params.u.sat.symbol_rate = start_search.Dvbstuningparams.Symbolrate.Value();
                     tuning_params.u.sat.fec = GetDvbsFEC(start_search.Dvbstuningparams.Fec);
                     tuning_params.u.sat.dvb_s2 = (start_search.Dvbstuningparams.Dvbs2 ? TRUE : FALSE);
                     tuning_params.u.sat.modulation = GetDvbsModulation(start_search.Dvbstuningparams.Modulation);
                     break;
                  case SIGNAL_COFDM:
                     SYSLOG(Logging::Notification, (_T("DTV::StartServiceSearch: manual DVB-T")));
                     tuning_params.freq = start_search.Dvbttuningparams.Frequency;
                     tuning_params.u.terr.bwidth = GetDvbBandwidth(start_search.Dvbttuningparams.Bandwidth);
                     tuning_params.u.terr.mode = GetDvbOfdmMode(start_search.Dvbttuningparams.Mode);
                     tuning_params.u.terr.type = (start_search.Dvbttuningparams.Dvbt2 ? TERR_TYPE_DVBT2 : TERR_TYPE_DVBT);
                     tuning_params.u.terr.plp_id = start_search.Dvbttuningparams.Plpid;
                     break;
                  case SIGNAL_QAM:
                     SYSLOG(Logging::Notification, (_T("DTV::StartServiceSearch: manual DVB-C")));
                     tuning_params.freq = start_search.Dvbctuningparams.Frequency;
                     tuning_params.u.cab.symbol_rate = start_search.Dvbctuningparams.Symbolrate.Value();
                     tuning_params.u.cab.mode = GetDvbcModulation(start_search.Dvbctuningparams.Modulation);
                     break;
                  default:
                     break;
               }
            }
            else
            {
               manual_search = false;
               SYSLOG(Logging::Notification, (_T("DTV::StartServiceSearch: auto")));
            }

            E_ACTL_SEARCH_TYPE search;

            if (start_search.Searchtype == StartServiceSearchParamsData::SearchtypeType::FREQUENCY)
            {
               search = ACTL_FREQ_SEARCH;
            }
            else
            {
               search = ACTL_NETWORK_SEARCH;
            }

            ADB_PrepareDatabaseForSearch(signal, NULL, (start_search.Retune ? TRUE : FALSE), FALSE);

            BOOLEAN search_started;

            if (manual_search)
            {
               search_started = ACTL_StartManualSearch(signal, &tuning_params, search);
            }
            else
            {
               search_started = ACTL_StartServiceSearch(signal, search);
            }

            if (search_started)
            {
               response = true;
               result = Core::ERROR_NONE;
            }
         }

         return(result);
      }

      uint32_t DTV::FinishServiceSearch(const FinishServiceSearchParamsData& finish_search, Core::JSON::Boolean& response)
      {
         uint32_t result;
         E_STB_DP_SIGNAL_TYPE signal = GetDvbSignalType(finish_search.Tunertype);

         if (signal != SIGNAL_NONE)
         {
            ADB_FinaliseDatabaseAfterSearch(finish_search.Savechanges, signal, NULL, TRUE, TRUE, FALSE);
            result = Core::ERROR_NONE;
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
         U16BIT onet_id, trans_id, serv_id;
         void *service = NULL;
         bool monitor_only = false;

         if (play_params.Dvburi.IsSet())
         {
            SYSLOG(Logging::Notification, (_T("DTV::StartPlaying: uri=%s"), play_params.Dvburi.Value().c_str()));

            // Find the service defined by the given DVB uri
            if (std::sscanf(play_params.Dvburi.Value().c_str(), "%hu.%hu.%hu", &onet_id, &trans_id, &serv_id) == 3)
            {
               service = ADB_FindServiceByIds(onet_id, trans_id, serv_id);
            }
         }
         else if (play_params.Lcn.IsSet())
         {
            SYSLOG(Logging::Notification, (_T("DTV::StartPlaying: lcn=%u"), play_params.Lcn.Value()));
            service = ADB_FindServiceByLcn(ADB_SERVICE_LIST_ALL, play_params.Lcn.Value(), FALSE);
         }

         if (play_params.Monitoronly.IsSet())
         {
            monitor_only = play_params.Monitoronly;
            SYSLOG(Logging::Notification, (_T("DTV::StartPlaying: monitor=%u"), monitor_only));
         }

         if (service != NULL)
         {
            E_ACTL_PATH_PURPOSE purpose = (monitor_only ? ACTL_PATH_PURPOSE_MONITOR_SI : ACTL_PATH_PURPOSE_STREAM_LIVE);

            U8BIT decode_path = ACTL_TuneToService(INVALID_RES_ID, NULL, service, FALSE, purpose);
            if (decode_path != INVALID_RES_ID)
            {
               play_handle = decode_path;
               result = Core::ERROR_NONE;
            }
            else
            {
               play_handle = -1;
            }

            SYSLOG(Logging::Notification, (_T("DTV::StartPlaying: %d"), play_handle.Value()));
         }

         return(result);
      }

      uint32_t DTV::StopPlaying(Core::JSON::DecSInt32 play_handle)
      {
         SYSLOG(Logging::Notification, (_T("DTV::StopPlaying: %d"), play_handle.Value()));

         if (play_handle.Value() >= 0)
         {
            U8BIT decode_path = (U8BIT)play_handle.Value();
            ACTL_TuneOff(decode_path);
            ACTL_ReleasePath(decode_path);
         }

         return(Core::ERROR_NONE);
      }

      void DTV::EventSearchStatus(SearchstatusParamsData& params)
      {
         Notify(_T("searchstatus"), params);
      }

      void DTV::EventService(const string& event_name, ServiceupdatedParamsInfo& params)
      {
         Notify(event_name, params);
      }

      void DTV::EventEventChanged(EventchangedParamsData& params)
      {
         Notify(_T("eventchanged"), params);
      }

      void DTV::ExtractDvbServiceInfo(ServiceInfo& service, void *serv_ptr) const
      {
         U8BIT *name;
         U16BIT onet_id, trans_id, serv_id;

         if ((name = ADB_GetServiceFullName(serv_ptr, FALSE)) != NULL)
         {
            // Ignore the UTF-8 lead byte
            service.Fullname = (char *)(name + 1);
            STB_ReleaseUnicodeString(name);
         }
         else
         {
            service.Fullname = _T("");
         }

         if ((name = ADB_GetServiceShortName(serv_ptr, FALSE)) != NULL)
         {
            // Ignore the UTF-8 lead byte
            service.Shortname = (char *)(name + 1);
            STB_ReleaseUnicodeString(name);
         }
         else
         {
            service.Shortname = _T("");
         }

         ADB_GetServiceIds(serv_ptr, &onet_id, &trans_id, &serv_id);

         service.Dvburi = std::to_string(onet_id) + "." + std::to_string(trans_id) +
            "." + std::to_string(serv_id);

         service.Servicetype = GetJsonServiceType(ADB_GetServiceType(serv_ptr));
         service.Lcn = ADB_GetServiceLcn(serv_ptr);
         service.Scrambled = (ADB_GetServiceScrambledFlag(serv_ptr) ? true : false);
         service.Hascadescriptor = (ADB_GetServiceHasCaDesc(serv_ptr) ? true : false);
         service.Hidden = (ADB_GetServiceHiddenFlag(serv_ptr) ? true : false);
         service.Selectable = (ADB_GetServiceSelectableFlag(serv_ptr) ? true : false);
         service.Runningstatus = GetJsonRunningStatus(ADB_GetServiceRunningStatus(serv_ptr));
      }

      void DTV::ExtractDvbStreamInfo(ComponentData& component, void *stream) const
      {
         ADB_STREAM_TYPE stream_type = ADB_GetStreamType(stream);

         component.Type = GetJsonStreamType(stream_type);
         component.Pid = ADB_GetStreamPID(stream);

         U8BIT num_tags = ADB_GetStreamNumTags(stream);

         for (U8BIT index = 0; index < num_tags; index++)
         {
            component.Tags.Add(ADB_GetStreamTag(stream, index));
         }

         if ((stream_type >= ADB_VIDEO_STREAM) && (stream_type <= ADB_AVS_VIDEO_STREAM))
         {
            component.Video.Codec = GetJsonStreamCodec(stream_type);
         }
         else if ((stream_type >= ADB_AUDIO_STREAM) && (stream_type <= ADB_EAC3_AUDIO_STREAM))
         {
            component.Audio.Codec = GetJsonStreamCodec(stream_type);
            component.Audio.Language = GetJsonLanguageCode(ADB_GetAudioStreamLangCode(stream));
            component.Audio.Type = GetJsonAudioType(ADB_GetAudioStreamType(stream));
            component.Audio.Mode = GetJsonAudioMode(ADB_GetAudioStreamMode(stream));
         }
         else if (stream_type == ADB_SUBTITLE_STREAM)
         {
            component.Subtitles.Language = GetJsonLanguageCode(ADB_GetSubtitleStreamLangCode(stream));
            component.Subtitles.Format = GetJsonSubtitleFormat(ADB_GetSubtitleStreamType(stream));
            component.Subtitles.Compositionpage = ADB_GetSubtitleStreamCompositionPage(stream);
            component.Subtitles.Ancillarypage = ADB_GetSubtitleStreamAncillaryPage(stream);
         }
         else if (stream_type == ADB_TTEXT_STREAM)
         {
            component.Teletext.Language = GetJsonLanguageCode(ADB_GetTtextStreamLangCode(stream));
            component.Teletext.Type = ADB_GetTtextStreamType(stream);
            component.Teletext.Magazine = ADB_GetTtextStreamMagazine(stream);
            component.Teletext.Page = ADB_GetTtextStreamPage(stream);
         }
      }

      void DTV::ExtractDvbTransportInfo(TransportInfo& transport, void *trans_ptr) const
      {
         E_STB_DP_SIGNAL_TYPE signal_type = ADB_GetTransportSignalType(trans_ptr);

         transport.Tunertype = GetJsonTunerType(signal_type);
         transport.Originalnetworkid = ADB_GetTransportOriginalNetworkId(trans_ptr);
         transport.Transportid = ADB_GetTransportTid(trans_ptr);
         transport.Strength = ADB_GetTransportTunedStrength(trans_ptr);
         transport.Quality = ADB_GetTransportTunedQuality(trans_ptr);

         switch (signal_type)
         {
            case SIGNAL_QPSK:
               ExtractDvbsTuningParams(transport.Dvbstuningparams, trans_ptr);
               break;
            case SIGNAL_QAM:
               ExtractDvbcTuningParams(transport.Dvbctuningparams, trans_ptr);
               break;
            case SIGNAL_COFDM:
               ExtractDvbtTuningParams(transport.Dvbttuningparams, trans_ptr);
               break;
            default:
               break;
         }
      }

      void DTV::ExtractDvbsTuningParams(DvbstuningparamsInfo& tuning_params, void *transport) const
      {
         U32BIT frequency;
         E_STB_DP_POLARITY polarity;
         U16BIT symbol_rate;
         E_STB_DP_FEC fec;
         BOOLEAN dvb_s2;
         E_STB_DP_MODULATION modulation;

         ADB_GetTransportSatTuningParams(transport, &frequency, &polarity, &symbol_rate, &fec,
            &dvb_s2, &modulation);

         tuning_params.Frequency = frequency;
         tuning_params.Polarity = GetJsonPolarity(polarity);
         tuning_params.Symbolrate = symbol_rate;
         tuning_params.Fec = GetJsonFec(fec);
         tuning_params.Modulation = GetJsonDvbsModulation(modulation);
         tuning_params.Dvbs2 = (dvb_s2 ? true : false);

         void *satellite = ADB_GetTransportSatellite(transport);
         if (satellite != NULL)
         {
            tuning_params.Satellite = Core::ToString(ADB_GetSatelliteName(satellite));
         }
      }

      void DTV::ExtractDvbcTuningParams(DvbctuningparamsInfo& tuning_params, void *transport) const
      {
         U32BIT frequency;
         E_STB_DP_CMODE modulation;
         U16BIT symbol_rate;

         ADB_GetTransportCableTuningParams(transport, &frequency, &modulation, &symbol_rate);

         tuning_params.Frequency = frequency;
         tuning_params.Symbolrate = symbol_rate;
         tuning_params.Modulation = GetJsonDvbcModulation(modulation);
      }

      void DTV::ExtractDvbtTuningParams(DvbttuningparamsInfo& tuning_params, void *transport) const
      {
         E_STB_DP_TTYPE terr_type;
         U32BIT frequency;
         E_STB_DP_TMODE mode;
         E_STB_DP_TBWIDTH bandwidth;
         U8BIT plp_id;

         ADB_GetTransportTerrestrialTuningParams(transport, &terr_type, &frequency, &mode,
            &bandwidth, &plp_id);

         tuning_params.Frequency = frequency;
         tuning_params.Bandwidth = GetJsonBandwidth(bandwidth);
         tuning_params.Mode = GetJsonOfdmMode(mode);
         tuning_params.Dvbt2 = (terr_type == TERR_TYPE_DVBT2) ? true : false;
         tuning_params.Plpid = plp_id;
      }

      Core::JSON::EnumType<PolarityType> DTV::GetJsonPolarity(E_STB_DP_POLARITY dvb_polarity) const
      {
         Core::JSON::EnumType<PolarityType> polarity;

         switch (dvb_polarity)
         {
            case POLARITY_HORIZONTAL:
               polarity = PolarityType::HORIZONTAL;
               break;
            case POLARITY_VERTICAL:
               polarity = PolarityType::VERTICAL;
               break;
            case POLARITY_LEFT:
               polarity = PolarityType::LEFT;
               break;
            case POLARITY_RIGHT:
               polarity = PolarityType::RIGHT;
               break;
         }

         return(polarity);
      }

      Core::JSON::EnumType<FecType> DTV::GetJsonFec(E_STB_DP_FEC dvb_fec) const
      {
         Core::JSON::EnumType<FecType> fec;

         switch (dvb_fec)
         {
            case FEC_AUTOMATIC:
               fec = FecType::FECAUTO;
               break;
            case FEC_1_2:
               fec = FecType::FEC1_2;
               break;
            case FEC_2_3:
               fec = FecType::FEC2_3;
               break;
            case FEC_3_4:
               fec = FecType::FEC3_4;
               break;
            case FEC_5_6:
               fec = FecType::FEC5_6;
               break;
            case FEC_7_8:
               fec = FecType::FEC7_8;
               break;
            case FEC_1_4:
               fec = FecType::FEC1_4;
               break;
            case FEC_1_3:
               fec = FecType::FEC1_3;
               break;
            case FEC_2_5:
               fec = FecType::FEC2_5;
               break;
            case FEC_8_9:
               fec = FecType::FEC8_9;
               break;
            case FEC_9_10:
               fec = FecType::FEC9_10;
               break;
            case FEC_3_5:
               fec = FecType::FEC3_5;
               break;
            case FEC_4_5:
               fec = FecType::FEC4_5;
               break;
         }

         return(fec);
      }

      Core::JSON::EnumType<DvbsmodulationType> DTV::GetJsonDvbsModulation(E_STB_DP_MODULATION dvb_modulation) const
      {
         Core::JSON::EnumType<DvbsmodulationType> modulation;

         switch (dvb_modulation)
         {
            case MOD_QPSK:
               modulation = DvbsmodulationType::QPSK;
               break;
            case MOD_8PSK:
               modulation = DvbsmodulationType::E8PSK;
               break;
            case MOD_16QAM:
               modulation = DvbsmodulationType::E16QAM;
               break;
            case MOD_AUTO:
            default:
               modulation = DvbsmodulationType::AUTO;
               break;
         }

         return(modulation);
      }

      Core::JSON::EnumType<DvbcmodulationType> DTV::GetJsonDvbcModulation(E_STB_DP_CMODE dvb_modulation) const
      {
         Core::JSON::EnumType<DvbcmodulationType> modulation;

         switch (dvb_modulation)
         {
            case MODE_QAM_AUTO:
               modulation = DvbcmodulationType::AUTO;
               break;
            case MODE_QAM_4:
               modulation = DvbcmodulationType::E4QAM;
               break;
            case MODE_QAM_8:
               modulation = DvbcmodulationType::E8QAM;
               break;
            case MODE_QAM_16:
               modulation = DvbcmodulationType::E16QAM;
               break;
            case MODE_QAM_32:
               modulation = DvbcmodulationType::E32QAM;
               break;
            case MODE_QAM_64:
               modulation = DvbcmodulationType::E64QAM;
               break;
            case MODE_QAM_128:
               modulation = DvbcmodulationType::E128QAM;
               break;
            case MODE_QAM_256:
               modulation = DvbcmodulationType::E256QAM;
               break;
         }

         return(modulation);
      }

      Core::JSON::EnumType<DvbtbandwidthType> DTV::GetJsonBandwidth(E_STB_DP_TBWIDTH dvb_bandwidth) const
      {
         Core::JSON::EnumType<DvbtbandwidthType> bandwidth;

         switch (dvb_bandwidth)
         {
            case TBWIDTH_5MHZ:
               bandwidth = DvbtbandwidthType::E5MHZ;
               break;
            case TBWIDTH_6MHZ:
               bandwidth = DvbtbandwidthType::E6MHZ;
               break;
            case TBWIDTH_7MHZ:
               bandwidth = DvbtbandwidthType::E7MHZ;
               break;
            case TBWIDTH_8MHZ:
               bandwidth = DvbtbandwidthType::E8MHZ;
               break;
            case TBWIDTH_10MHZ:
               bandwidth = DvbtbandwidthType::E10MHZ;
               break;
            case TBWIDTH_UNDEFINED:
            default:
               bandwidth = DvbtbandwidthType::UNDEFINED;
               break;
         }

         return(bandwidth);
      }

      Core::JSON::EnumType<OfdmmodeType> DTV::GetJsonOfdmMode(E_STB_DP_TMODE dvb_mode) const
      {
         Core::JSON::EnumType<OfdmmodeType> mode;

         switch (dvb_mode)
         {
            case MODE_COFDM_2K:
               mode = OfdmmodeType::OFDM_2K;
               break;
            case MODE_COFDM_8K:
               mode = OfdmmodeType::OFDM_8K;
               break;
            case MODE_COFDM_4K:
               mode = OfdmmodeType::OFDM_4K;
               break;
            case MODE_COFDM_1K:
               mode = OfdmmodeType::OFDM_1K;
               break;
            case MODE_COFDM_16K:
               mode = OfdmmodeType::OFDM_16K;
               break;
            case MODE_COFDM_32K:
               mode = OfdmmodeType::OFDM_32K;
               break;
            case MODE_COFDM_UNDEFINED:
            default:
               mode = OfdmmodeType::UNDEFINED;
               break;
         }

         return(mode);
      }

      Core::JSON::EnumType<TunertypeType> DTV::GetJsonTunerType(E_STB_DP_SIGNAL_TYPE signal_type) const
      {
         Core::JSON::EnumType<TunertypeType> tuner_type;

         switch (signal_type)
         {
            case SIGNAL_QPSK:
               tuner_type = TunertypeType::DVBS;
               break;
            case SIGNAL_COFDM:
               tuner_type = TunertypeType::DVBT;
               break;
            case SIGNAL_QAM:
               tuner_type = TunertypeType::DVBC;
               break;
            case SIGNAL_NONE:
            default:
               tuner_type = TunertypeType::NONE;
               break;
         }

         return(tuner_type);
      }

      Core::JSON::EnumType<ComponentData::TypeType> DTV::GetJsonStreamType(ADB_STREAM_TYPE type) const
      {
         Core::JSON::EnumType<ComponentData::TypeType> comp_type;

         switch (type)
         {
            case ADB_VIDEO_STREAM:
            case ADB_H264_VIDEO_STREAM:
            case ADB_H265_VIDEO_STREAM:
            case ADB_AVS_VIDEO_STREAM:
               comp_type = ComponentData::TypeType::VIDEO;
               break;
            case ADB_AUDIO_STREAM:
            case ADB_AAC_AUDIO_STREAM:
            case ADB_HEAAC_AUDIO_STREAM:
            case ADB_HEAACv2_AUDIO_STREAM:
            case ADB_AC3_AUDIO_STREAM:
            case ADB_EAC3_AUDIO_STREAM:
               comp_type = ComponentData::TypeType::AUDIO;
               break;
            case ADB_SUBTITLE_STREAM:
               comp_type = ComponentData::TypeType::SUBTITLES;
               break;
            case ADB_DATA_STREAM:
               comp_type = ComponentData::TypeType::DATA;
               break;
            case ADB_TTEXT_STREAM:
               comp_type = ComponentData::TypeType::TELETEXT;
               break;
         }

         return(comp_type);
      }

      Core::JSON::EnumType<CodecType> DTV::GetJsonStreamCodec(ADB_STREAM_TYPE type) const
      {
         Core::JSON::EnumType<CodecType> codec;

         switch (type)
         {
            case ADB_VIDEO_STREAM:
               codec = CodecType::MPEG2;
               break;
            case ADB_H264_VIDEO_STREAM:
               codec = CodecType::H264;
               break;
            case ADB_H265_VIDEO_STREAM:
               codec = CodecType::H265;
               break;
            case ADB_AVS_VIDEO_STREAM:
               codec = CodecType::AVS;
               break;
            case ADB_AUDIO_STREAM:
               codec = CodecType::MPEG1;
               break;
            case ADB_AAC_AUDIO_STREAM:
               codec = CodecType::AAC;
               break;
            case ADB_HEAAC_AUDIO_STREAM:
               codec = CodecType::HEAAC;
               break;
            case ADB_HEAACv2_AUDIO_STREAM:
               codec = CodecType::HEAACV2;
               break;
            case ADB_AC3_AUDIO_STREAM:
               codec = CodecType::AC3;
               break;
            case ADB_EAC3_AUDIO_STREAM:
               codec = CodecType::EAC3;
               break;
         }

         return(codec);
      }

      Core::JSON::String DTV::GetJsonLanguageCode(U32BIT code) const
      {
         char lang[4];

         lang[0] = (char)((code >> 16) & 0xff);
         lang[1] = (char)((code >> 8) & 0xff);
         lang[2] = (char)(code & 0xff);
         lang[3] = '\0';

         return(Core::JSON::String(lang));
      }

      Core::JSON::EnumType<ComponentData::AudioData::TypeType> DTV::GetJsonAudioType(ADB_AUDIO_TYPE type) const
      {
         Core::JSON::EnumType<ComponentData::AudioData::TypeType> audio_type;

         switch (type)
         {
            case ADB_AUDIO_TYPE_UNDEFINED:
               audio_type = ComponentData::AudioData::TypeType::UNDEFINED;
               break;
            case ADB_AUDIO_TYPE_CLEAN_EFFECTS:
               audio_type = ComponentData::AudioData::TypeType::CLEAN;
               break;
            case ADB_AUDIO_TYPE_FOR_HEARING_IMPAIRED:
               audio_type = ComponentData::AudioData::TypeType::HEARINGIMPAIRED;
               break;
            case ADB_AUDIO_TYPE_FOR_VISUALLY_IMPAIRED:
               audio_type = ComponentData::AudioData::TypeType::VISUALLYIMPAIRED;
               break;
            default:
               audio_type = ComponentData::AudioData::TypeType::UNKNOWN;
               break;
         }

         return(audio_type);
      }

      Core::JSON::EnumType<ComponentData::AudioData::ModeType> DTV::GetJsonAudioMode(E_STB_DP_AUDIO_MODE mode) const
      {
         Core::JSON::EnumType<ComponentData::AudioData::ModeType> audio_mode;

         switch (mode)
         {
            case AUDIO_STEREO:
               audio_mode = ComponentData::AudioData::ModeType::STEREO;
               break;
            case AUDIO_LEFT:
               audio_mode = ComponentData::AudioData::ModeType::LEFT;
               break;
            case AUDIO_RIGHT:
               audio_mode = ComponentData::AudioData::ModeType::RIGHT;
               break;
            case AUDIO_MONO:
               audio_mode = ComponentData::AudioData::ModeType::MONO;
               break;
            case AUDIO_MULTICHANNEL:
               audio_mode = ComponentData::AudioData::ModeType::MULTICHANNEL;
               break;
            case AUDIO_UNDEF:
            default:
               audio_mode = ComponentData::AudioData::ModeType::UNDEFINED;
               break;
         }

         return(audio_mode);
      }

      Core::JSON::EnumType<ComponentData::SubtitlesData::FormatType> DTV::GetJsonSubtitleFormat(ADB_SUBTITLE_TYPE type) const
      {
         Core::JSON::EnumType<ComponentData::SubtitlesData::FormatType> format;

         switch (type)
         {
            case ADB_SUBTITLE_TYPE_DVB:
               format = ComponentData::SubtitlesData::FormatType::DEFAULT;
               break;
            case ADB_SUBTITLE_TYPE_DVB_4_3:
               format = ComponentData::SubtitlesData::FormatType::E4_3;
               break;
            case ADB_SUBTITLE_TYPE_DVB_16_9:
               format = ComponentData::SubtitlesData::FormatType::E16_9;
               break;
            case ADB_SUBTITLE_TYPE_DVB_221_1:
               format = ComponentData::SubtitlesData::FormatType::E221_1;
               break;
            case ADB_SUBTITLE_TYPE_DVB_HD:
               format = ComponentData::SubtitlesData::FormatType::HD;
               break;
            case ADB_SUBTITLE_TYPE_DVB_HARD_HEARING:
               format = ComponentData::SubtitlesData::FormatType::HARDOFHEARING;
               break;
            case ADB_SUBTITLE_TYPE_DVB_HARD_HEARING_4_3:
               format = ComponentData::SubtitlesData::FormatType::HARDOFHEARING4_3;
               break;
            case ADB_SUBTITLE_TYPE_DVB_HARD_HEARING_16_9:
               format = ComponentData::SubtitlesData::FormatType::HARDOFHEARING16_9;
               break;
            case ADB_SUBTITLE_TYPE_DVB_HARD_HEARING_221_1:
               format = ComponentData::SubtitlesData::FormatType::HARDOFHEARING221_1;
               break;
            case ADB_SUBTITLE_TYPE_DVB_HARD_HEARING_HD:
               format = ComponentData::SubtitlesData::FormatType::HARDOFHEARINGHD;
               break;
         }

         return(format);
      }

      Core::JSON::EnumType<ServicetypeType> DTV::GetJsonServiceType(ADB_SERVICE_TYPE type) const
      {
         Core::JSON::EnumType<ServicetypeType> serv_type;

         switch(type)
         {
            case ADB_SERVICE_TYPE_TV:
               serv_type = ServicetypeType::TV;
               break;
            case ADB_SERVICE_TYPE_RADIO:
               serv_type = ServicetypeType::RADIO;
               break;
            case ADB_SERVICE_TYPE_TELETEXT:
               serv_type = ServicetypeType::TELETEXT;
               break;
            case ADB_SERVICE_TYPE_NVOD_REF:
               serv_type = ServicetypeType::NVOD;
               break;
            case ADB_SERVICE_TYPE_NVOD_TIMESHIFT:
               serv_type = ServicetypeType::NVOD_TIMESHIFT;
               break;
            case ADB_SERVICE_TYPE_MOSAIC:
               serv_type = ServicetypeType::MOSAIC;
               break;
            case ADB_SERVICE_TYPE_AVC_RADIO:
               serv_type = ServicetypeType::AVC_RADIO;
               break;
            case ADB_SERVICE_TYPE_AVC_MOSAIC:
               serv_type = ServicetypeType::AVC_MOSAIC;
               break;
            case ADB_SERVICE_TYPE_DATA:
               serv_type = ServicetypeType::DATA;
               break;
            case ADB_SERVICE_TYPE_MPEG2_HD:
               serv_type = ServicetypeType::MPEG2_HD;
               break;
            case ADB_SERVICE_TYPE_AVC_SD_TV:
               serv_type = ServicetypeType::AVC_SD_TV;
               break;
            case ADB_SERVICE_TYPE_AVC_SD_NVOD_TIMESHIFT:
               serv_type = ServicetypeType::AVC_SD_NVOD_TIMESHIFT;
               break;
            case ADB_SERVICE_TYPE_AVC_SD_NVOD_REF:
               serv_type = ServicetypeType::AVC_SD_NVOD;
               break;
            case ADB_SERVICE_TYPE_HD_TV:
               serv_type = ServicetypeType::HD_TV;
               break;
            case ADB_SERVICE_TYPE_AVC_HD_NVOD_TIMESHIFT:
               serv_type = ServicetypeType::AVC_HD_NVOD_TIMESHIFT;
               break;
            case ADB_SERVICE_TYPE_AVC_HD_NVOD_REF:
               serv_type = ServicetypeType::AVC_HD_NVOD;
               break;
            case ADB_SERVICE_TYPE_UHD_TV:
               serv_type = ServicetypeType::UHD_TV;
               break;
            default:
               serv_type = ServicetypeType::UNKNOWN;
               break;
         }

         return(serv_type);
      }

      Core::JSON::EnumType<RunningstatusType> DTV::GetJsonRunningStatus(U8BIT status) const
      {
         Core::JSON::EnumType<RunningstatusType> run_state;

         switch(status)
         {
            case RUN_STATE_NOT_RUNNING:
               run_state = RunningstatusType::NOTRUNNING;
               break;
            case RUN_STATE_STARTS_SOON:
               run_state = RunningstatusType::STARTSSOON;
               break;
            case RUN_STATE_PAUSING:
               run_state = RunningstatusType::PAUSING;
               break;
            case RUN_STATE_RUNNING:
               run_state = RunningstatusType::RUNNING;
               break;
            case RUN_STATE_OFF_AIR:
               run_state = RunningstatusType::OFFAIR;
               break;
            case RUN_STATE_UNDEFINED:
            default:
               run_state = RunningstatusType::UNDEFINED;
               break;
         }

         return(run_state);
      }

      Core::JSON::EnumType<LnbtypeType> DTV::GetJsonLnbType(E_STB_DP_LNB_TYPE type) const
      {
         Core::JSON::EnumType<LnbtypeType> lnb_type;

         switch(type)
         {
            case LNB_TYPE_SINGLE:
               lnb_type = LnbtypeType::SINGLE;
               break;
            case LNB_TYPE_UNIVERSAL:
               lnb_type = LnbtypeType::UNIVERSAL;
               break;
            case LNB_TYPE_UNICABLE:
               lnb_type = LnbtypeType::UNICABLE;
               break;
         }

         return(lnb_type);
      }

      Core::JSON::EnumType<LnbpowerType> DTV::GetJsonLnbPower(E_STB_DP_LNB_POWER power) const
      {
         Core::JSON::EnumType<LnbpowerType> lnb_power;

         switch(power)
         {
            case LNB_POWER_OFF:
               lnb_power = LnbpowerType::OFF;
               break;
            case LNB_POWER_ON:
               lnb_power = LnbpowerType::ON;
               break;
            case LNB_POWER_AUTO:
               lnb_power = LnbpowerType::AUTO;
               break;
         }

         return(lnb_power);
      }

      Core::JSON::EnumType<Diseqc_toneType> DTV::GetJsonDiseqcTone(E_STB_DP_DISEQC_TONE tone) const
      {
         Core::JSON::EnumType<Diseqc_toneType> diseqc_tone;

         switch(tone)
         {
            case DISEQC_TONE_OFF:
               diseqc_tone = Diseqc_toneType::OFF;
               break;
            case DISEQC_TONE_A:
               diseqc_tone = Diseqc_toneType::A;
               break;
            case DISEQC_TONE_B:
               diseqc_tone = Diseqc_toneType::B;
               break;
         }

         return(diseqc_tone);
      }

      Core::JSON::EnumType<Diseqc_cswitchType> DTV::GetJsonDiseqcCSwitch(E_STB_DP_DISEQC_CSWITCH cswitch) const
      {
         Core::JSON::EnumType<Diseqc_cswitchType> diseqc_cswitch;

         switch(cswitch)
         {
            case DISEQC_CSWITCH_OFF:
               diseqc_cswitch = Diseqc_cswitchType::OFF;
               break;
            case DISEQC_CSWITCH_A:
               diseqc_cswitch = Diseqc_cswitchType::A;
               break;
            case DISEQC_CSWITCH_B:
               diseqc_cswitch = Diseqc_cswitchType::B;
               break;
            case DISEQC_CSWITCH_C:
               diseqc_cswitch = Diseqc_cswitchType::C;
               break;
            case DISEQC_CSWITCH_D:
               diseqc_cswitch = Diseqc_cswitchType::D;
               break;
         }

         return(diseqc_cswitch);
      }

      E_STB_DP_LNB_TYPE DTV::GetDvbLnbType(Core::JSON::EnumType<LnbtypeType> lnb_type) const
      {
         E_STB_DP_LNB_TYPE type;

         switch(lnb_type)
         {
            case LnbtypeType::SINGLE:
               type = LNB_TYPE_SINGLE;
               break;
            case LnbtypeType::UNICABLE:
               type = LNB_TYPE_UNICABLE;
               break;
            case LnbtypeType::UNIVERSAL:
            default:
               type = LNB_TYPE_UNIVERSAL;
               break;
         }

         return(type);
      }

      E_STB_DP_LNB_POWER DTV::GetDvbLnbPower(Core::JSON::EnumType<LnbpowerType> lnb_power) const
      {
         E_STB_DP_LNB_POWER power;

         switch(lnb_power)
         {
            case LnbpowerType::ON:
               power = LNB_POWER_ON;
               break;
            case LnbpowerType::AUTO:
               power = LNB_POWER_AUTO;
               break;
            case LnbpowerType::OFF:
            default:
               power = LNB_POWER_OFF;
               break;
         }

         return(power);
      }

      E_STB_DP_DISEQC_TONE DTV::GetDvbDiseqcTone(Core::JSON::EnumType<Diseqc_toneType> diseqc_tone) const
      {
         E_STB_DP_DISEQC_TONE tone;

         switch(diseqc_tone)
         {
            case Diseqc_toneType::A:
               tone = DISEQC_TONE_A;
               break;
            case Diseqc_toneType::B:
               tone = DISEQC_TONE_B;
               break;
            case Diseqc_toneType::OFF:
            default:
               tone = DISEQC_TONE_OFF;
               break;
         }

         return(tone);
      }

      E_STB_DP_DISEQC_CSWITCH DTV::GetDvbDiseqcCSwitch(Core::JSON::EnumType<Diseqc_cswitchType> diseqc_cswitch) const
      {
         E_STB_DP_DISEQC_CSWITCH cswitch;

         switch(diseqc_cswitch)
         {
            case Diseqc_cswitchType::A:
               cswitch = DISEQC_CSWITCH_A;
               break;
            case Diseqc_cswitchType::B:
               cswitch = DISEQC_CSWITCH_B;
               break;
            case Diseqc_cswitchType::C:
               cswitch = DISEQC_CSWITCH_C;
               break;
            case Diseqc_cswitchType::D:
               cswitch = DISEQC_CSWITCH_D;
               break;
            case Diseqc_cswitchType::OFF:
            default:
               cswitch = DISEQC_CSWITCH_OFF;
               break;
         }

         return(cswitch);
      }

      E_STB_DP_SIGNAL_TYPE DTV::GetDvbSignalType(Core::JSON::EnumType<TunertypeType> tuner_type) const
      {
         E_STB_DP_SIGNAL_TYPE signal;

         switch(tuner_type)
         {
            case TunertypeType::DVBS:
               signal = SIGNAL_QPSK;
               break;
            case TunertypeType::DVBT:
               signal = SIGNAL_COFDM;
               break;
            case TunertypeType::DVBC:
               signal = SIGNAL_QAM;
               break;
            default:
               signal = SIGNAL_NONE;
               break;
         }

         return(signal);
      }

      E_STB_DP_POLARITY DTV::GetDvbPolarity(Core::JSON::EnumType<PolarityType> polarity_type) const
      {
         E_STB_DP_POLARITY polarity;

         switch (polarity_type)
         {
            case PolarityType::VERTICAL:
               polarity = POLARITY_VERTICAL;
               break;
            case PolarityType::LEFT:
               polarity = POLARITY_LEFT;
               break;
            case PolarityType::RIGHT:
               polarity = POLARITY_RIGHT;
               break;
            case PolarityType::HORIZONTAL:
            default:
               polarity = POLARITY_HORIZONTAL;
               break;
         }

         return(polarity);
      }

      E_STB_DP_FEC DTV::GetDvbsFEC(Core::JSON::EnumType<FecType> fec_type) const
      {
         E_STB_DP_FEC fec;

         switch (fec_type)
         {
            case FecType::FEC1_2:
               fec = FEC_1_2;
               break;
            case FecType::FEC2_3:
               fec = FEC_2_3;
               break;
            case FecType::FEC3_4:
               fec = FEC_3_4;
               break;
            case FecType::FEC5_6:
               fec = FEC_5_6;
               break;
            case FecType::FEC7_8:
               fec = FEC_7_8;
               break;
            case FecType::FEC1_4:
               fec = FEC_1_4;
               break;
            case FecType::FEC1_3:
               fec = FEC_1_3;
               break;
            case FecType::FEC2_5:
               fec = FEC_2_5;
               break;
            case FecType::FEC8_9:
               fec = FEC_8_9;
               break;
            case FecType::FEC9_10:
               fec = FEC_9_10;
               break;
            case FecType::FEC3_5:
               fec = FEC_3_5;
               break;
            case FecType::FEC4_5:
               fec = FEC_4_5;
               break;
            case FecType::FECAUTO:
            default:
               fec = FEC_AUTOMATIC;
               break;
         }

         return(fec);
      }

      E_STB_DP_MODULATION DTV::GetDvbsModulation(Core::JSON::EnumType<DvbsmodulationType> modulation_type) const
      {
         E_STB_DP_MODULATION modulation;

         switch(modulation_type)
         {
            case DvbsmodulationType::QPSK:
               modulation = MOD_QPSK;
               break;
            case DvbsmodulationType::E8PSK:
               modulation = MOD_8PSK;
               break;
            case DvbsmodulationType::E16QAM:
               modulation = MOD_16QAM;
               break;
            case DvbsmodulationType::AUTO:
            default:
               modulation = MOD_AUTO;
               break;
         }

         return(modulation);
      }

      E_STB_DP_TBWIDTH DTV::GetDvbBandwidth(Core::JSON::EnumType<DvbtbandwidthType> bandwidth_type) const
      {
         E_STB_DP_TBWIDTH bwidth;

         switch (bandwidth_type)
         {
            case DvbtbandwidthType::E5MHZ:
               bwidth = TBWIDTH_5MHZ;
               break;
            case DvbtbandwidthType::E6MHZ:
               bwidth = TBWIDTH_6MHZ;
               break;
            case DvbtbandwidthType::E7MHZ:
               bwidth = TBWIDTH_7MHZ;
               break;
            case DvbtbandwidthType::E8MHZ:
               bwidth = TBWIDTH_8MHZ;
               break;
            case DvbtbandwidthType::E10MHZ:
               bwidth = TBWIDTH_10MHZ;
               break;
            case DvbtbandwidthType::UNDEFINED:
            default:
               bwidth = TBWIDTH_UNDEFINED;
               break;
         }

         return(bwidth);
      }

      E_STB_DP_TMODE DTV::GetDvbOfdmMode(Core::JSON::EnumType<OfdmmodeType> mode_type) const
      {
         E_STB_DP_TMODE mode;

         switch (mode_type)
         {
            case OfdmmodeType::OFDM_1K:
               mode = MODE_COFDM_1K;
               break;
            case OfdmmodeType::OFDM_2K:
               mode = MODE_COFDM_2K;
               break;
            case OfdmmodeType::OFDM_4K:
               mode = MODE_COFDM_4K;
               break;
            case OfdmmodeType::OFDM_8K:
               mode = MODE_COFDM_8K;
               break;
            case OfdmmodeType::OFDM_16K:
               mode = MODE_COFDM_16K;
               break;
            case OfdmmodeType::OFDM_32K:
               mode = MODE_COFDM_32K;
               break;
            case OfdmmodeType::UNDEFINED:
            default:
               mode = MODE_COFDM_UNDEFINED;
               break;
         }

         return(mode);
      }

      E_STB_DP_CMODE DTV::GetDvbcModulation(Core::JSON::EnumType<DvbcmodulationType> modulation_type) const
      {
         E_STB_DP_CMODE modulation;

         switch(modulation_type)
         {
            case DvbcmodulationType::E4QAM:
               modulation = MODE_QAM_4;
               break;
            case DvbcmodulationType::E8QAM:
               modulation = MODE_QAM_8;
               break;
            case DvbcmodulationType::E16QAM:
               modulation = MODE_QAM_16;
               break;
            case DvbcmodulationType::E32QAM:
               modulation = MODE_QAM_32;
               break;
            case DvbcmodulationType::E64QAM:
               modulation = MODE_QAM_64;
               break;
            case DvbcmodulationType::E128QAM:
               modulation = MODE_QAM_128;
               break;
            case DvbcmodulationType::E256QAM:
               modulation = MODE_QAM_256;
               break;
            case DvbcmodulationType::AUTO:
            default:
               modulation = MODE_QAM_AUTO;
               break;
         }

         return(modulation);
      }

      void* DTV::FindSatellite(const char *satellite_name) const
      {
         U8BIT *name;

         void *sat_ptr = ADB_GetNextSatellite(NULL);
         while (sat_ptr != NULL)
         {
            if ((name = ADB_GetSatelliteName(sat_ptr)) != NULL)
            {
               if (strcmp(satellite_name, (char *)name + 1) == 0)
               {
                  // Found the named satellite
                  break;
               }
            }

            sat_ptr = ADB_GetNextSatellite(sat_ptr);
         }

         return(sat_ptr);
      }

      void DTV::ExtractDvbEventInfo(EiteventInfo& event, void *dvb_event) const
      {
         SetJsonString(ADB_GetEventName(dvb_event), event.Name, true);
         SetJsonString(ADB_GetEventDescription(dvb_event), event.Shortdescription, true);

         event.Starttime = STB_GCConvertToTimestamp(ADB_GetEventStartDateTime(dvb_event));

         U32DHMS dhms = ADB_GetEventDuration(dvb_event);
         event.Duration = ((DHMS_DAYS(dhms) * 24 + DHMS_HOUR(dhms)) * 60 + DHMS_MINS(dhms)) * 60 + DHMS_SECS(dhms);

         event.Eventid = ADB_GetEventId(dvb_event);
         event.Hassubtitles = (ADB_GetEventSubtitlesAvailFlag(dvb_event) ? true : false);
         event.Hasaudiodescription = (ADB_GetEventAudioDescriptionFlag(dvb_event) ? true : false);
         event.Parentalrating = ADB_GetEventParentalAge(dvb_event);
         event.Hasextendedinfo = (ADB_GetEventHasExtendedDescription(dvb_event) ? true : false);

         U8BIT content_len;
         U8BIT *content_data = ADB_GetEventContentData(dvb_event, &content_len);
         if ((content_len != 0) && (content_data != NULL))
         {
            for (U8BIT i = 0; i < content_len; i++)
            {
               event.Contentdata.Add(content_data[i]);
            }
         }
      }

      void DTV::SetJsonString(U8BIT *src_string, Core::JSON::String& out_string, bool free_src) const
      {
         if (src_string != NULL)
         {
            // Strip any DVB control chars from the string and output it minus the unicode indicator byte, if present
            U8BIT *outstr = STB_UnicodeStripControlChars(src_string);
            if (outstr != NULL)
            {
               if (STB_IsUnicodeString(outstr))
               {
                  out_string = Core::ToString((char *)outstr + 1);
               }
               else
               {
                  out_string = Core::ToString((char *)outstr);
               }

               STB_ReleaseUnicodeString(outstr);
            }
            else
            {
               // The string isn't unicode so just use the source string
               out_string = Core::ToString((char *)src_string);
            }

            if (free_src)
            {
               STB_ReleaseUnicodeString(src_string);
            }
         }
         else
         {
            out_string = string();
         }
      }
   }
}

