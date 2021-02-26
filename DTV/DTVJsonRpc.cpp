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
   #include <ap_dbacc.h>
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
         JSONRPC::Property<Core::JSON::DecUInt8>(_T("numberOfCountries"), &DTV::GetNumberOfCountries, nullptr, this);
         JSONRPC::Property<Core::JSON::ArrayType<CountryconfigData>>(_T("countryList"), &DTV::GetCountryList, nullptr, this);
         JSONRPC::Property<Core::JSON::DecUInt32>(_T("country"), &DTV::GetCountry, &DTV::SetCountry, this);
         JSONRPC::Property<Core::JSON::ArrayType<LnbsettingsInfo>>(_T("lnbList"), &DTV::GetLnbList, nullptr, this);
         JSONRPC::Property<Core::JSON::ArrayType<SatellitesettingsInfo>>(_T("satelliteList"), &DTV::GetSatelliteList, nullptr, this);
         JSONRPC::Property<Core::JSON::DecUInt16>(_T("numberOfServices"), &DTV::GetNumberOfServices, nullptr, this);
         JSONRPC::Property<Core::JSON::ArrayType<ServiceData>>(_T("serviceList"), &DTV::GetServiceList, nullptr, this);
         JSONRPC::Property<NowNextEventsData>(_T("nowNextEvents"), &DTV::GetNowNextEvents, nullptr, this);
         JSONRPC::Property<Core::JSON::ArrayType<EiteventInfo>>(_T("scheduleEvents"), &DTV::GetScheduleEvents, nullptr, this);
         JSONRPC::Property<StatusData>(_T("status"), &DTV::GetStatus, nullptr, this);

         JSONRPC::Register<LnbsettingsInfo, Core::JSON::Boolean>(_T("addLnb"), &DTV::AddLnb, this);
         JSONRPC::Register<SatellitesettingsInfo, Core::JSON::Boolean>(_T("addSatellite"), &DTV::AddSatellite, this);
         JSONRPC::Register<StartServiceSearchParamsData, Core::JSON::Boolean>(_T("startServiceSearch"), &DTV::StartServiceSearch, this);
         JSONRPC::Register<FinishServiceSearchParamsData, Core::JSON::Boolean>(_T("finishServiceSearch"), &DTV::FinishServiceSearch, this);
         JSONRPC::Register<StartPlayingParamsData, Core::JSON::DecSInt32>(_T("startPlaying"), &DTV::StartPlaying, this);
         JSONRPC::Register<Core::JSON::DecSInt32, void>(_T("stopPlaying"), &DTV::StopPlaying, this);
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
            CountryconfigData config;

            for (U8BIT index = 0; index < num_countries; index++)
            {
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
         LnbsettingsInfo lnb;

         SYSLOG(Logging::Notification, (_T("DTV::GetLnbList")));

         void *lnb_ptr = ADB_GetNextLNB(NULL);
         while (lnb_ptr != NULL)
         {
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
            lnb.Unicableinterface = settings.unicable_if;

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
         SatellitesettingsInfo sat;
         U16BIT longitude;
         void *lnb_ptr;

         SYSLOG(Logging::Notification, (_T("DTV::GetSatelliteList")));

         void *sat_ptr = ADB_GetNextSatellite(NULL);
         while (sat_ptr != NULL)
         {
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
      uint32_t DTV::GetServiceList(const string& tuner_type, Core::JSON::ArrayType<ServiceData>& response) const
      {
         U16BIT num_services;
         void **slist;
         E_STB_DP_SIGNAL_TYPE signal;

         SYSLOG(Logging::Notification, (_T("DTV::GetServiceList: %s"), tuner_type.c_str()));

         switch(Core::EnumerateType<TunertypeType>(tuner_type.c_str()).Value())
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

         if ((slist != NULL) && (num_services != 0))
         {
            ServiceData service;
            BOOLEAN is_sig2;
            U8BIT *name;
            U16BIT onet_id, trans_id, serv_id;

            for (U16BIT index = 0; index < num_services; index++)
            {
               if ((signal == SIGNAL_NONE) || (ADB_GetServiceSignalType(slist[index], &is_sig2) == signal))
               {
                  if ((name = ADB_GetServiceShortName(slist[index], FALSE)) != NULL)
                  {
                     // Ignore the UTF-8 lead byte
                     service.Shortname = (char *)(name + 1);
                     STB_ReleaseUnicodeString(name);
                  }
                  else
                  {
                     service.Shortname = _T("");
                  }

                  service.Lcn = ADB_GetServiceLcn(slist[index]);

                  ADB_GetServiceIds(slist[index], &onet_id, &trans_id, &serv_id);

                  service.Dvburi = std::to_string(onet_id) + "." + std::to_string(trans_id) +
                     "." + std::to_string(serv_id);

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
                     ExtractDvbEventInfo(now, response.Now);
                     ADB_ReleaseEventData(now);
                  }
                  else
                  {
                     response.Now.Starttime = 0;
                  }

                  if (next != NULL)
                  {
                     ExtractDvbEventInfo(next, response.Next);
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
                     EiteventInfo event;

                     for (U16BIT i = 0; i < num_events; i++)
                     {
                        start_time = STB_GCConvertToTimestamp(ADB_GetEventStartDateTime(event_list[i]));
                        if ((start_time >= start_utc) && (start_time <= end_utc))
                        {
                           ExtractDvbEventInfo(event_list[i], event);
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
         settings.unicable_if = lnb_settings.Unicableinterface;

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
                     SYSLOG(Logging::Notification, (_T("DTV::StartServiceSearch: manual DVB-T not supported")));
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

         if (service != NULL)
         {
            U8BIT decode_path = ACTL_TuneToService(INVALID_RES_ID, NULL, service, FALSE, ACTL_PATH_PURPOSE_STREAM_LIVE);
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

      E_STB_DP_SIGNAL_TYPE DTV::GetDvbSignalType(TunertypeType tuner_type) const
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

      E_STB_DP_POLARITY DTV::GetDvbPolarity(StartServiceSearchParamsData::DvbstuningparamsData::PolarityType polarity_type) const
      {
         E_STB_DP_POLARITY polarity;

         switch (polarity_type)
         {
            case StartServiceSearchParamsData::DvbstuningparamsData::PolarityType::VERTICAL:
               polarity = POLARITY_VERTICAL;
               break;
            case StartServiceSearchParamsData::DvbstuningparamsData::PolarityType::LEFT:
               polarity = POLARITY_LEFT;
               break;
            case StartServiceSearchParamsData::DvbstuningparamsData::PolarityType::RIGHT:
               polarity = POLARITY_RIGHT;
               break;
            case StartServiceSearchParamsData::DvbstuningparamsData::PolarityType::HORIZONTAL:
            default:
               polarity = POLARITY_HORIZONTAL;
               break;
         }

         return(polarity);
      }

      E_STB_DP_FEC DTV::GetDvbsFEC(StartServiceSearchParamsData::DvbstuningparamsData::FecType fec_type) const
      {
         E_STB_DP_FEC fec;

         switch (fec_type)
         {
            case StartServiceSearchParamsData::DvbstuningparamsData::FecType::FEC1_2:
               fec = FEC_1_2;
               break;
            case StartServiceSearchParamsData::DvbstuningparamsData::FecType::FEC2_3:
               fec = FEC_2_3;
               break;
            case StartServiceSearchParamsData::DvbstuningparamsData::FecType::FEC3_4:
               fec = FEC_3_4;
               break;
            case StartServiceSearchParamsData::DvbstuningparamsData::FecType::FEC5_6:
               fec = FEC_5_6;
               break;
            case StartServiceSearchParamsData::DvbstuningparamsData::FecType::FEC7_8:
               fec = FEC_7_8;
               break;
            case StartServiceSearchParamsData::DvbstuningparamsData::FecType::FEC1_4:
               fec = FEC_1_4;
               break;
            case StartServiceSearchParamsData::DvbstuningparamsData::FecType::FEC1_3:
               fec = FEC_1_3;
               break;
            case StartServiceSearchParamsData::DvbstuningparamsData::FecType::FEC2_5:
               fec = FEC_2_5;
               break;
            case StartServiceSearchParamsData::DvbstuningparamsData::FecType::FEC8_9:
               fec = FEC_8_9;
               break;
            case StartServiceSearchParamsData::DvbstuningparamsData::FecType::FEC9_10:
               fec = FEC_9_10;
               break;
            case StartServiceSearchParamsData::DvbstuningparamsData::FecType::FECAUTO:
            default:
               fec = FEC_AUTOMATIC;
               break;
         }

         return(fec);
      }

      E_STB_DP_MODULATION DTV::GetDvbsModulation(StartServiceSearchParamsData::DvbstuningparamsData::DvbsmodulationType modulation_type) const
      {
         E_STB_DP_MODULATION modulation;

         switch(modulation_type)
         {
            case StartServiceSearchParamsData::DvbstuningparamsData::DvbsmodulationType::QPSK:
               modulation = MOD_QPSK;
               break;
            case StartServiceSearchParamsData::DvbstuningparamsData::DvbsmodulationType::E8PSK:
               modulation = MOD_8PSK;
               break;
            case StartServiceSearchParamsData::DvbstuningparamsData::DvbsmodulationType::E16QAM:
               modulation = MOD_16QAM;
               break;
            case StartServiceSearchParamsData::DvbstuningparamsData::DvbsmodulationType::AUTO:
            default:
               modulation = MOD_AUTO;
               break;
         }

         return(modulation);
      }

      E_STB_DP_CMODE DTV::GetDvbcModulation(StartServiceSearchParamsData::DvbctuningparamsData::DvbcmodulationType modulation_type) const
      {
         E_STB_DP_CMODE modulation;

         switch(modulation_type)
         {
            case StartServiceSearchParamsData::DvbctuningparamsData::DvbcmodulationType::E64QAM:
               modulation = MODE_QAM_64;
               break;
            case StartServiceSearchParamsData::DvbctuningparamsData::DvbcmodulationType::E128QAM:
               modulation = MODE_QAM_128;
               break;
            case StartServiceSearchParamsData::DvbctuningparamsData::DvbcmodulationType::E256QAM:
               modulation = MODE_QAM_256;
               break;
            case StartServiceSearchParamsData::DvbctuningparamsData::DvbcmodulationType::AUTO:
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

      void DTV::ExtractDvbEventInfo(void *src_event, EiteventInfo& dest_event) const
      {
         SetJsonString(ADB_GetEventName(src_event), dest_event.Name, true);
         SetJsonString(ADB_GetEventDescription(src_event), dest_event.Shortdescription, true);

         dest_event.Starttime = STB_GCConvertToTimestamp(ADB_GetEventStartDateTime(src_event));

         U32DHMS dhms = ADB_GetEventDuration(src_event);
         dest_event.Duration = ((DHMS_DAYS(dhms) * 24 + DHMS_HOUR(dhms)) * 60 + DHMS_MINS(dhms)) * 60 + DHMS_SECS(dhms);

         dest_event.Eventid = ADB_GetEventId(src_event);
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

