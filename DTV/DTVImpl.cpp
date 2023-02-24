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

#include "DTVImpl.h"

extern "C"
{
   // DVB include files
   #include <stberc.h>

   #include <ap_cfg.h>
};


namespace WPEFramework {
namespace Plugin {
   SERVICE_REGISTRATION(DTVImpl, 1, 0);

   static U8BIT *country_name = (U8BIT *)"RDK Demo";
   static ACFG_TER_RF_CHANNEL_DATA dvbt_tuning_table[] =
   {
      {(U8BIT *)"Ch 2", 50500000, TBWIDTH_7MHZ, MODE_COFDM_UNDEFINED, TERR_TYPE_DVBT2},
      {(U8BIT *)"Ch 2", 50500000, TBWIDTH_7MHZ, MODE_COFDM_UNDEFINED, TERR_TYPE_DVBT},
      {(U8BIT *)"Ch 3", 57500000, TBWIDTH_7MHZ, MODE_COFDM_UNDEFINED, TERR_TYPE_DVBT2},
      {(U8BIT *)"Ch 3", 57500000, TBWIDTH_7MHZ, MODE_COFDM_UNDEFINED, TERR_TYPE_DVBT},
      {(U8BIT *)"Ch 4", 64500000, TBWIDTH_7MHZ, MODE_COFDM_UNDEFINED, TERR_TYPE_DVBT2},
      {(U8BIT *)"Ch 4", 64500000, TBWIDTH_7MHZ, MODE_COFDM_UNDEFINED, TERR_TYPE_DVBT},
      {(U8BIT *)"Ch 5", 177500000, TBWIDTH_7MHZ, MODE_COFDM_UNDEFINED, TERR_TYPE_DVBT2},
      {(U8BIT *)"Ch 5", 177500000, TBWIDTH_7MHZ, MODE_COFDM_UNDEFINED, TERR_TYPE_DVBT},
      {(U8BIT *)"Ch 6", 184500000, TBWIDTH_7MHZ, MODE_COFDM_UNDEFINED, TERR_TYPE_DVBT2},
      {(U8BIT *)"Ch 6", 184500000, TBWIDTH_7MHZ, MODE_COFDM_UNDEFINED, TERR_TYPE_DVBT},
      {(U8BIT *)"Ch 7", 191500000, TBWIDTH_7MHZ, MODE_COFDM_UNDEFINED, TERR_TYPE_DVBT2},
      {(U8BIT *)"Ch 7", 191500000, TBWIDTH_7MHZ, MODE_COFDM_UNDEFINED, TERR_TYPE_DVBT},
      {(U8BIT *)"Ch 8", 198500000, TBWIDTH_7MHZ, MODE_COFDM_UNDEFINED, TERR_TYPE_DVBT2},
      {(U8BIT *)"Ch 8", 198500000, TBWIDTH_7MHZ, MODE_COFDM_UNDEFINED, TERR_TYPE_DVBT},
      {(U8BIT *)"Ch 9", 205500000, TBWIDTH_7MHZ, MODE_COFDM_UNDEFINED, TERR_TYPE_DVBT2},
      {(U8BIT *)"Ch 9", 205500000, TBWIDTH_7MHZ, MODE_COFDM_UNDEFINED, TERR_TYPE_DVBT},
      {(U8BIT *)"Ch10", 212500000, TBWIDTH_7MHZ, MODE_COFDM_UNDEFINED, TERR_TYPE_DVBT2},
      {(U8BIT *)"Ch10", 212500000, TBWIDTH_7MHZ, MODE_COFDM_UNDEFINED, TERR_TYPE_DVBT},
      {(U8BIT *)"Ch11", 219500000, TBWIDTH_7MHZ, MODE_COFDM_UNDEFINED, TERR_TYPE_DVBT2},
      {(U8BIT *)"Ch11", 219500000, TBWIDTH_7MHZ, MODE_COFDM_UNDEFINED, TERR_TYPE_DVBT},
      {(U8BIT *)"Ch12", 226500000, TBWIDTH_7MHZ, MODE_COFDM_UNDEFINED, TERR_TYPE_DVBT2},
      {(U8BIT *)"Ch12", 226500000, TBWIDTH_7MHZ, MODE_COFDM_UNDEFINED, TERR_TYPE_DVBT},
      {(U8BIT *)"Ch21", 474000000, TBWIDTH_8MHZ, MODE_COFDM_UNDEFINED, TERR_TYPE_DVBT2},
      {(U8BIT *)"Ch21", 474000000, TBWIDTH_8MHZ, MODE_COFDM_UNDEFINED, TERR_TYPE_DVBT},
      {(U8BIT *)"Ch22", 482000000, TBWIDTH_8MHZ, MODE_COFDM_UNDEFINED, TERR_TYPE_DVBT2},
      {(U8BIT *)"Ch22", 482000000, TBWIDTH_8MHZ, MODE_COFDM_UNDEFINED, TERR_TYPE_DVBT},
      {(U8BIT *)"Ch23", 490000000, TBWIDTH_8MHZ, MODE_COFDM_UNDEFINED, TERR_TYPE_DVBT2},
      {(U8BIT *)"Ch23", 490000000, TBWIDTH_8MHZ, MODE_COFDM_UNDEFINED, TERR_TYPE_DVBT},
      {(U8BIT *)"Ch24", 498000000, TBWIDTH_8MHZ, MODE_COFDM_UNDEFINED, TERR_TYPE_DVBT2},
      {(U8BIT *)"Ch24", 498000000, TBWIDTH_8MHZ, MODE_COFDM_UNDEFINED, TERR_TYPE_DVBT},
      {(U8BIT *)"Ch25", 506000000, TBWIDTH_8MHZ, MODE_COFDM_UNDEFINED, TERR_TYPE_DVBT2},
      {(U8BIT *)"Ch25", 506000000, TBWIDTH_8MHZ, MODE_COFDM_UNDEFINED, TERR_TYPE_DVBT},
      {(U8BIT *)"Ch26", 514000000, TBWIDTH_8MHZ, MODE_COFDM_UNDEFINED, TERR_TYPE_DVBT2},
      {(U8BIT *)"Ch26", 514000000, TBWIDTH_8MHZ, MODE_COFDM_UNDEFINED, TERR_TYPE_DVBT},
      {(U8BIT *)"Ch27", 522000000, TBWIDTH_8MHZ, MODE_COFDM_UNDEFINED, TERR_TYPE_DVBT2},
      {(U8BIT *)"Ch27", 522000000, TBWIDTH_8MHZ, MODE_COFDM_UNDEFINED, TERR_TYPE_DVBT},
      {(U8BIT *)"Ch28", 530000000, TBWIDTH_8MHZ, MODE_COFDM_UNDEFINED, TERR_TYPE_DVBT2},
      {(U8BIT *)"Ch28", 530000000, TBWIDTH_8MHZ, MODE_COFDM_UNDEFINED, TERR_TYPE_DVBT},
      {(U8BIT *)"Ch29", 538000000, TBWIDTH_8MHZ, MODE_COFDM_UNDEFINED, TERR_TYPE_DVBT2},
      {(U8BIT *)"Ch29", 538000000, TBWIDTH_8MHZ, MODE_COFDM_UNDEFINED, TERR_TYPE_DVBT},
      {(U8BIT *)"Ch30", 546000000, TBWIDTH_8MHZ, MODE_COFDM_UNDEFINED, TERR_TYPE_DVBT2},
      {(U8BIT *)"Ch30", 546000000, TBWIDTH_8MHZ, MODE_COFDM_UNDEFINED, TERR_TYPE_DVBT},
      {(U8BIT *)"Ch31", 554000000, TBWIDTH_8MHZ, MODE_COFDM_UNDEFINED, TERR_TYPE_DVBT2},
      {(U8BIT *)"Ch31", 554000000, TBWIDTH_8MHZ, MODE_COFDM_UNDEFINED, TERR_TYPE_DVBT},
      {(U8BIT *)"Ch32", 562000000, TBWIDTH_8MHZ, MODE_COFDM_UNDEFINED, TERR_TYPE_DVBT2},
      {(U8BIT *)"Ch32", 562000000, TBWIDTH_8MHZ, MODE_COFDM_UNDEFINED, TERR_TYPE_DVBT},
      {(U8BIT *)"Ch33", 570000000, TBWIDTH_8MHZ, MODE_COFDM_UNDEFINED, TERR_TYPE_DVBT2},
      {(U8BIT *)"Ch33", 570000000, TBWIDTH_8MHZ, MODE_COFDM_UNDEFINED, TERR_TYPE_DVBT},
      {(U8BIT *)"Ch34", 578000000, TBWIDTH_8MHZ, MODE_COFDM_UNDEFINED, TERR_TYPE_DVBT2},
      {(U8BIT *)"Ch34", 578000000, TBWIDTH_8MHZ, MODE_COFDM_UNDEFINED, TERR_TYPE_DVBT},
      {(U8BIT *)"Ch35", 586000000, TBWIDTH_8MHZ, MODE_COFDM_UNDEFINED, TERR_TYPE_DVBT2},
      {(U8BIT *)"Ch35", 586000000, TBWIDTH_8MHZ, MODE_COFDM_UNDEFINED, TERR_TYPE_DVBT},
      {(U8BIT *)"Ch36", 594000000, TBWIDTH_8MHZ, MODE_COFDM_UNDEFINED, TERR_TYPE_DVBT2},
      {(U8BIT *)"Ch36", 594000000, TBWIDTH_8MHZ, MODE_COFDM_UNDEFINED, TERR_TYPE_DVBT},
      {(U8BIT *)"Ch37", 602000000, TBWIDTH_8MHZ, MODE_COFDM_UNDEFINED, TERR_TYPE_DVBT2},
      {(U8BIT *)"Ch37", 602000000, TBWIDTH_8MHZ, MODE_COFDM_UNDEFINED, TERR_TYPE_DVBT},
      {(U8BIT *)"Ch38", 610000000, TBWIDTH_8MHZ, MODE_COFDM_UNDEFINED, TERR_TYPE_DVBT2},
      {(U8BIT *)"Ch38", 610000000, TBWIDTH_8MHZ, MODE_COFDM_UNDEFINED, TERR_TYPE_DVBT},
      {(U8BIT *)"Ch39", 618000000, TBWIDTH_8MHZ, MODE_COFDM_UNDEFINED, TERR_TYPE_DVBT2},
      {(U8BIT *)"Ch39", 618000000, TBWIDTH_8MHZ, MODE_COFDM_UNDEFINED, TERR_TYPE_DVBT},
      {(U8BIT *)"Ch40", 626000000, TBWIDTH_8MHZ, MODE_COFDM_UNDEFINED, TERR_TYPE_DVBT2},
      {(U8BIT *)"Ch40", 626000000, TBWIDTH_8MHZ, MODE_COFDM_UNDEFINED, TERR_TYPE_DVBT},
      {(U8BIT *)"Ch41", 634000000, TBWIDTH_8MHZ, MODE_COFDM_UNDEFINED, TERR_TYPE_DVBT2},
      {(U8BIT *)"Ch41", 634000000, TBWIDTH_8MHZ, MODE_COFDM_UNDEFINED, TERR_TYPE_DVBT},
      {(U8BIT *)"Ch42", 642000000, TBWIDTH_8MHZ, MODE_COFDM_UNDEFINED, TERR_TYPE_DVBT2},
      {(U8BIT *)"Ch42", 642000000, TBWIDTH_8MHZ, MODE_COFDM_UNDEFINED, TERR_TYPE_DVBT},
      {(U8BIT *)"Ch43", 650000000, TBWIDTH_8MHZ, MODE_COFDM_UNDEFINED, TERR_TYPE_DVBT2},
      {(U8BIT *)"Ch43", 650000000, TBWIDTH_8MHZ, MODE_COFDM_UNDEFINED, TERR_TYPE_DVBT},
      {(U8BIT *)"Ch44", 658000000, TBWIDTH_8MHZ, MODE_COFDM_UNDEFINED, TERR_TYPE_DVBT2},
      {(U8BIT *)"Ch44", 658000000, TBWIDTH_8MHZ, MODE_COFDM_UNDEFINED, TERR_TYPE_DVBT},
      {(U8BIT *)"Ch45", 666000000, TBWIDTH_8MHZ, MODE_COFDM_UNDEFINED, TERR_TYPE_DVBT2},
      {(U8BIT *)"Ch45", 666000000, TBWIDTH_8MHZ, MODE_COFDM_UNDEFINED, TERR_TYPE_DVBT},
      {(U8BIT *)"Ch46", 674000000, TBWIDTH_8MHZ, MODE_COFDM_UNDEFINED, TERR_TYPE_DVBT2},
      {(U8BIT *)"Ch46", 674000000, TBWIDTH_8MHZ, MODE_COFDM_UNDEFINED, TERR_TYPE_DVBT},
      {(U8BIT *)"Ch47", 682000000, TBWIDTH_8MHZ, MODE_COFDM_UNDEFINED, TERR_TYPE_DVBT2},
      {(U8BIT *)"Ch47", 682000000, TBWIDTH_8MHZ, MODE_COFDM_UNDEFINED, TERR_TYPE_DVBT},
      {(U8BIT *)"Ch48", 690000000, TBWIDTH_8MHZ, MODE_COFDM_UNDEFINED, TERR_TYPE_DVBT2},
      {(U8BIT *)"Ch48", 690000000, TBWIDTH_8MHZ, MODE_COFDM_UNDEFINED, TERR_TYPE_DVBT},
      {(U8BIT *)"Ch49", 698000000, TBWIDTH_8MHZ, MODE_COFDM_UNDEFINED, TERR_TYPE_DVBT2},
      {(U8BIT *)"Ch49", 698000000, TBWIDTH_8MHZ, MODE_COFDM_UNDEFINED, TERR_TYPE_DVBT},
      {(U8BIT *)"Ch50", 706000000, TBWIDTH_8MHZ, MODE_COFDM_UNDEFINED, TERR_TYPE_DVBT2},
      {(U8BIT *)"Ch50", 706000000, TBWIDTH_8MHZ, MODE_COFDM_UNDEFINED, TERR_TYPE_DVBT},
      {(U8BIT *)"Ch51", 714000000, TBWIDTH_8MHZ, MODE_COFDM_UNDEFINED, TERR_TYPE_DVBT2},
      {(U8BIT *)"Ch51", 714000000, TBWIDTH_8MHZ, MODE_COFDM_UNDEFINED, TERR_TYPE_DVBT},
      {(U8BIT *)"Ch52", 722000000, TBWIDTH_8MHZ, MODE_COFDM_UNDEFINED, TERR_TYPE_DVBT2},
      {(U8BIT *)"Ch52", 722000000, TBWIDTH_8MHZ, MODE_COFDM_UNDEFINED, TERR_TYPE_DVBT},
      {(U8BIT *)"Ch53", 730000000, TBWIDTH_8MHZ, MODE_COFDM_UNDEFINED, TERR_TYPE_DVBT2},
      {(U8BIT *)"Ch53", 730000000, TBWIDTH_8MHZ, MODE_COFDM_UNDEFINED, TERR_TYPE_DVBT},
      {(U8BIT *)"Ch54", 738000000, TBWIDTH_8MHZ, MODE_COFDM_UNDEFINED, TERR_TYPE_DVBT2},
      {(U8BIT *)"Ch54", 738000000, TBWIDTH_8MHZ, MODE_COFDM_UNDEFINED, TERR_TYPE_DVBT},
      {(U8BIT *)"Ch55", 746000000, TBWIDTH_8MHZ, MODE_COFDM_UNDEFINED, TERR_TYPE_DVBT2},
      {(U8BIT *)"Ch55", 746000000, TBWIDTH_8MHZ, MODE_COFDM_UNDEFINED, TERR_TYPE_DVBT},
      {(U8BIT *)"Ch56", 754000000, TBWIDTH_8MHZ, MODE_COFDM_UNDEFINED, TERR_TYPE_DVBT2},
      {(U8BIT *)"Ch56", 754000000, TBWIDTH_8MHZ, MODE_COFDM_UNDEFINED, TERR_TYPE_DVBT},
      {(U8BIT *)"Ch57", 762000000, TBWIDTH_8MHZ, MODE_COFDM_UNDEFINED, TERR_TYPE_DVBT2},
      {(U8BIT *)"Ch57", 762000000, TBWIDTH_8MHZ, MODE_COFDM_UNDEFINED, TERR_TYPE_DVBT},
      {(U8BIT *)"Ch58", 770000000, TBWIDTH_8MHZ, MODE_COFDM_UNDEFINED, TERR_TYPE_DVBT2},
      {(U8BIT *)"Ch58", 770000000, TBWIDTH_8MHZ, MODE_COFDM_UNDEFINED, TERR_TYPE_DVBT},
      {(U8BIT *)"Ch59", 778000000, TBWIDTH_8MHZ, MODE_COFDM_UNDEFINED, TERR_TYPE_DVBT2},
      {(U8BIT *)"Ch59", 778000000, TBWIDTH_8MHZ, MODE_COFDM_UNDEFINED, TERR_TYPE_DVBT},
      {(U8BIT *)"Ch60", 786000000, TBWIDTH_8MHZ, MODE_COFDM_UNDEFINED, TERR_TYPE_DVBT2},
      {(U8BIT *)"Ch60", 786000000, TBWIDTH_8MHZ, MODE_COFDM_UNDEFINED, TERR_TYPE_DVBT},
      {(U8BIT *)"Ch61", 794000000, TBWIDTH_8MHZ, MODE_COFDM_UNDEFINED, TERR_TYPE_DVBT2},
      {(U8BIT *)"Ch61", 794000000, TBWIDTH_8MHZ, MODE_COFDM_UNDEFINED, TERR_TYPE_DVBT},
      {(U8BIT *)"Ch62", 802000000, TBWIDTH_8MHZ, MODE_COFDM_UNDEFINED, TERR_TYPE_DVBT2},
      {(U8BIT *)"Ch62", 802000000, TBWIDTH_8MHZ, MODE_COFDM_UNDEFINED, TERR_TYPE_DVBT},
      {(U8BIT *)"Ch63", 810000000, TBWIDTH_8MHZ, MODE_COFDM_UNDEFINED, TERR_TYPE_DVBT2},
      {(U8BIT *)"Ch63", 810000000, TBWIDTH_8MHZ, MODE_COFDM_UNDEFINED, TERR_TYPE_DVBT},
      {(U8BIT *)"Ch64", 818000000, TBWIDTH_8MHZ, MODE_COFDM_UNDEFINED, TERR_TYPE_DVBT2},
      {(U8BIT *)"Ch64", 818000000, TBWIDTH_8MHZ, MODE_COFDM_UNDEFINED, TERR_TYPE_DVBT},
      {(U8BIT *)"Ch65", 826000000, TBWIDTH_8MHZ, MODE_COFDM_UNDEFINED, TERR_TYPE_DVBT2},
      {(U8BIT *)"Ch65", 826000000, TBWIDTH_8MHZ, MODE_COFDM_UNDEFINED, TERR_TYPE_DVBT},
      {(U8BIT *)"Ch66", 834000000, TBWIDTH_8MHZ, MODE_COFDM_UNDEFINED, TERR_TYPE_DVBT2},
      {(U8BIT *)"Ch66", 834000000, TBWIDTH_8MHZ, MODE_COFDM_UNDEFINED, TERR_TYPE_DVBT},
      {(U8BIT *)"Ch67", 842000000, TBWIDTH_8MHZ, MODE_COFDM_UNDEFINED, TERR_TYPE_DVBT2},
      {(U8BIT *)"Ch67", 842000000, TBWIDTH_8MHZ, MODE_COFDM_UNDEFINED, TERR_TYPE_DVBT},
      {(U8BIT *)"Ch68", 850000000, TBWIDTH_8MHZ, MODE_COFDM_UNDEFINED, TERR_TYPE_DVBT2},
      {(U8BIT *)"Ch68", 850000000, TBWIDTH_8MHZ, MODE_COFDM_UNDEFINED, TERR_TYPE_DVBT},
      {(U8BIT *)"Ch69", 858000000, TBWIDTH_8MHZ, MODE_COFDM_UNDEFINED, TERR_TYPE_DVBT2},
      {(U8BIT *)"Ch69", 858000000, TBWIDTH_8MHZ, MODE_COFDM_UNDEFINED, TERR_TYPE_DVBT}
   };

   DTVImpl::DTVImpl()
   {
      DTVImpl::instance(this);
   }

   DTVImpl::~DTVImpl()
   {
   }

   DTVImpl* DTVImpl::instance(DTVImpl *dtv)
   {
      static DTVImpl *dtv_instance = nullptr;

      if (dtv != nullptr)
      {
         dtv_instance = dtv;
      }

      return(dtv_instance);
   }

   uint32_t DTVImpl::Initialize(PluginHost::IShell* framework)
   {
      Config config;
      config.FromString(framework->ConfigLine());

      SYSLOG(Logging::Startup, (_T("DTVImpl::Initialize: PID=%u"), getpid()));

      E_DVB_INIT_SUBS_TTXT subs_ttxt = DVB_INIT_NO_TELETEXT_OR_SUBTITLES;

      if (config.SubtitleProcessing && config.TeletextProcessing)
      {
         subs_ttxt = DVB_INIT_TELETEXT_AND_SUBTITLES;
      }
      else if (config.SubtitleProcessing)
      {
         subs_ttxt = DVB_INIT_SUBTITLES_ONLY;
      }
      else if (config.TeletextProcessing)
      {
         subs_ttxt = DVB_INIT_TELETEXT_ONLY;
      }

      if (APP_InitialiseDVB(DvbEventHandler, subs_ttxt))
      {
         SYSLOG(Logging::Startup, (_T("DVB initialised")));

         ACFG_COUNTRY_CONFIG country_config;

         memset(&country_config, 0, sizeof(ACFG_COUNTRY_CONFIG));

         country_config.country_name = country_name;
         country_config.country_code = COUNTRY_CODE_USERDEFINED;
         country_config.ter_rf_channel_table_ptr = dvbt_tuning_table;
         country_config.num_ter_rf_channels = sizeof(dvbt_tuning_table) / sizeof(dvbt_tuning_table[0]);

         ACFG_SetCountryConfig(COUNTRY_CODE_USERDEFINED, &country_config);
         ACFG_SetCountry(COUNTRY_CODE_USERDEFINED);
      }
      else
      {
         SYSLOG(Logging::Fatal, (_T("Initialise: Failed to initialise DVBCore")));
      }

      return Core::ERROR_NONE;
   }

   uint32_t DTVImpl::Register(IDTV::INotification* notification)
   {
      m_notification_mutex.Lock();

      // Don't register the same notification callback multiple times
      if (std::find(m_notification_callbacks.begin(), m_notification_callbacks.end(), notification) == m_notification_callbacks.end())
      {
         m_notification_callbacks.push_back(notification);
         notification->AddRef();
      }

      m_notification_mutex.Unlock();

      return Core::ERROR_NONE;
   }

   uint32_t DTVImpl::Unregister(IDTV::INotification* notification)
   {
      m_notification_mutex.Lock();

      auto itr = std::find(m_notification_callbacks.begin(), m_notification_callbacks.end(), notification);
      if (itr != m_notification_callbacks.end())
      {
         (*itr)->Release();
         m_notification_callbacks.erase(itr);
      }

      m_notification_mutex.Unlock();

      return Core::ERROR_NONE;
   }

   uint32_t DTVImpl::GetNumberOfCountries(uint8_t& count) const
   {
      count = ACFG_GetNumCountries();
      return Core::ERROR_NONE;
   }

   uint32_t DTVImpl::GetCountryList(ICountry::IIterator*& countries) const
   {
      std::list<IDTV::ICountry*> country_list;
      U8BIT num_countries = 0;
      U8BIT **country_names = NULL;

      ACFG_GetCountryList(&country_names, &num_countries);

      if ((num_countries != 0) && (country_names != NULL))
      {
         for (U8BIT index = 0; index < num_countries; index++)
         {
            country_list.push_back(Core::Service<CountryImpl>::Create<IDTV::ICountry>((const char *)country_names[index], ACFG_GetCountryCode(index)));
         }

         ACFG_ReleaseCountryList(country_names, num_countries);
      }

      // Create country list that will be returned as the result
      countries = Core::Service<CountryImpl::IteratorImpl>::Create<IDTV::ICountry::IIterator>(country_list);

      for (auto country : country_list)
      {
         country->Release();
      }

      return Core::ERROR_NONE;
   }

   uint32_t DTVImpl::GetCountry(uint32_t& code) const
   {
      code = ACFG_GetCountry();
      return Core::ERROR_NONE;
   }

   uint32_t DTVImpl::SetCountry(const uint32_t code)
   {
      uint32_t result = Core::ERROR_BAD_REQUEST;

      if (ACFG_SetCountry(code))
      {
         result = Core::ERROR_NONE;
      }

      return result;
   }

   uint32_t DTVImpl::GetLnbList(ILnb::IIterator*& lnbs) const
   {
      std::list<IDTV::ILnb*> lnb_list;

      void *lnb_ptr = ADB_GetNextLNB(NULL);
      while (lnb_ptr != NULL)
      {
         lnb_list.push_back(Core::Service<LnbImpl>::Create<IDTV::ILnb>(lnb_ptr));
         lnb_ptr = ADB_GetNextLNB(lnb_ptr);
      }

      // Create LNB list that will be returned as the result
      lnbs = Core::Service<LnbImpl::IteratorImpl>::Create<IDTV::ILnb::IIterator>(lnb_list);

      for (auto lnb : lnb_list)
      {
         lnb->Release();
      }

      return (Core::ERROR_NONE);
   }

   uint32_t DTVImpl::GetSatelliteList(ISatellite::IIterator*& satellites) const
   {
      std::list<IDTV::ISatellite*> sat_list;

      void *sat_ptr = ADB_GetNextSatellite(NULL);
      while (sat_ptr != NULL)
      {
         sat_list.push_back(Core::Service<SatelliteImpl>::Create<IDTV::ISatellite>(sat_ptr));
         sat_ptr = ADB_GetNextSatellite(sat_ptr);
      }

      // Create satellite list that will be returned as the result
      satellites = Core::Service<SatelliteImpl::IteratorImpl>::Create<IDTV::ISatellite::IIterator>(sat_list);

      for (auto sat : sat_list)
      {
         sat->Release();
      }

      return (Core::ERROR_NONE);
   }

   uint32_t DTVImpl::GetNumberOfServices(uint16_t& count) const
   {
      count = ADB_GetNumServices();
      return (Core::ERROR_NONE);
   }

   uint32_t DTVImpl::GetServiceList(IService::IIterator*& services) const
   {
      return (GetServiceList(IDTV::TunerType::NONE, services));
   }

   uint32_t DTVImpl::GetServiceList(const IDTV::TunerType tuner_type, IService::IIterator*& services) const
   {
      U16BIT num_services;
      void **slist;
      std::list<IDTV::IService*> service_list;
      E_STB_DP_SIGNAL_TYPE signal;

      switch (tuner_type)
      {
         case IDTV::TunerType::DVBC:
            signal = SIGNAL_QAM;
            break;
         case IDTV::TunerType::DVBS:
            signal = SIGNAL_QPSK;
            break;
         case IDTV::TunerType::DVBT:
            signal = SIGNAL_COFDM;
            break;
         case IDTV::TunerType::NONE:
         default:
            signal = SIGNAL_NONE;
            break;
      }

      ADB_GetServiceList(ADB_SERVICE_LIST_DIGITAL, &slist, &num_services);

      if ((slist != NULL) && (num_services != 0))
      {
         BOOLEAN is_sig2;

         for (U16BIT index = 0; index < num_services; index++)
         {
            if ((signal == SIGNAL_NONE) || (ADB_GetServiceSignalType(slist[index], &is_sig2) == signal))
            {
               service_list.push_back(Core::Service<ServiceImpl>::Create<IDTV::IService>(slist[index]));
            }
         }

         ADB_ReleaseServiceList(slist, num_services);
      }

      // Create service list iterator that will be returned
      services = Core::Service<ServiceImpl::IteratorImpl>::Create<IDTV::IService::IIterator>(service_list);

      for (auto service : service_list)
      {
         service->Release();
      }

      return (Core::ERROR_NONE);
   }

   uint32_t DTVImpl::GetServiceList(const uint16_t onet_id, const uint16_t trans_id, IService::IIterator*& services) const
   {
      U16BIT num_services;
      void **slist;
      std::list<IDTV::IService*> service_list;

      void *transport = ADB_GetTransportFromIds(ADB_INVALID_DVB_ID, onet_id, trans_id);
      if (transport != NULL)
      {
         ADB_GetTransportServiceList(transport, &slist, &num_services);

         if ((slist != NULL) && (num_services != 0))
         {
            for (U16BIT index = 0; index < num_services; index++)
            {
               service_list.push_back(Core::Service<ServiceImpl>::Create<IDTV::IService>(slist[index]));
            }

            ADB_ReleaseServiceList(slist, num_services);
         }
      }

      // Create service list iterator that will be returned
      services = Core::Service<ServiceImpl::IteratorImpl>::Create<IDTV::IService::IIterator>(service_list);

      for (auto service : service_list)
      {
         service->Release();
      }

      return (Core::ERROR_NONE);
   }

   uint32_t DTVImpl::GetNowNextEvents(const uint16_t onet_id, const uint16_t trans_id, const uint16_t serv_id,
      IEitEvent*& now_event, IEitEvent*& next_event) const
   {
      uint32_t result = Core::ERROR_BAD_REQUEST;

      void *service = ADB_FindServiceByIds(onet_id, trans_id, serv_id);
      if (service != NULL)
      {
         void *now;
         void *next;

         ADB_GetNowNextEvents(service, &now, &next);

         if (now != NULL)
         {
            now_event = Core::Service<EitEventImpl>::Create<IDTV::IEitEvent>(now);
            ADB_ReleaseEventData(now);
         }
         else
         {
            now = nullptr;
         }

         if (next != NULL)
         {
            next_event = Core::Service<EitEventImpl>::Create<IDTV::IEitEvent>(next);
            ADB_ReleaseEventData(next);
         }
         else
         {
            next = nullptr;
         }

         result = Core::ERROR_NONE;
      }
  
      return result;
   }

   uint32_t DTVImpl::GetScheduleEvents(const uint16_t onet_id, const uint16_t trans_id, const uint16_t serv_id,
      const uint32_t start_utc, const uint32_t end_utc, IEitEvent::IIterator*& events) const
   {
      uint32_t result = Core::ERROR_BAD_REQUEST;
      std::list<IDTV::IEitEvent*> list;

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
                  list.push_back(Core::Service<EitEventImpl>::Create<IDTV::IEitEvent>(event_list[i]));
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

      // Create event list iterator that will be returned
      events = Core::Service<EitEventImpl::IteratorImpl>::Create<IDTV::IEitEvent::IIterator>(list);

      for (auto event : list)
      {
         event->Release();
      }

      return result;
   }

   uint32_t DTVImpl::GetStatus(const int32_t handle, IStatus*& status) const
   {
      uint32_t result = Core::ERROR_BAD_REQUEST;

      if ((handle >= 0) && (handle < (int32_t)STB_DPGetNumPaths()))
      {
         status = Core::Service<StatusImpl>::Create<IDTV::IStatus>(handle);
         result = Core::ERROR_NONE;
      }

      return result;
   }

   uint32_t DTVImpl::GetServiceInfo(const uint16_t onet_id, const uint16_t trans_id, const uint16_t serv_id,
      IService*& service_info) const
   {
      uint32_t result = Core::ERROR_BAD_REQUEST;

      void *service = ADB_FindServiceByIds(onet_id, trans_id, serv_id);
      if (service != NULL)
      {
         service_info = Core::Service<ServiceImpl>::Create<IDTV::IService>(service);
         result = Core::ERROR_NONE;
      }

      return result;
   }

   uint32_t DTVImpl::GetServiceComponents(const uint16_t onet_id, const uint16_t trans_id, const uint16_t serv_id,
      IComponent::IIterator*& components) const
   {
      uint32_t result = Core::ERROR_BAD_REQUEST;
      std::list<IDTV::IComponent*> component_list;

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
               component_list.push_back(Core::Service<ComponentImpl>::Create<IDTV::IComponent>(streams[index]));
            }

            ADB_ReleaseStreamList(streams, num_streams);
         }

         result = Core::ERROR_NONE;
      }

      // Create component list iterator that will be returned
      components = Core::Service<ComponentImpl::IteratorImpl>::Create<IDTV::IComponent::IIterator>(component_list);

      for (auto component : component_list)
      {
         component->Release();
      }

      return result;
   }

   uint32_t DTVImpl::GetTransportInfo(const uint16_t onet_id, const uint16_t trans_id,
      ITransport*& transport_info) const
   {
      uint32_t result = Core::ERROR_BAD_REQUEST;

      void *transport = ADB_GetTransportFromIds(ADB_INVALID_DVB_ID, onet_id, trans_id);
      if (transport != NULL)
      {
         transport_info = Core::Service<TransportImpl>::Create<IDTV::ITransport>(transport);
         result = Core::ERROR_NONE;
      }

      return result;
   }

   uint32_t DTVImpl::GetExtendedEventInfo(const uint16_t onet_id, const uint16_t trans_id, const uint16_t serv_id,
      const uint16_t event_id, IEitExtendedEvent*& info) const
   {
      uint32_t result = Core::ERROR_BAD_REQUEST;

      void *service = ADB_FindServiceByIds(onet_id, trans_id, serv_id);
      if (service != NULL)
      {
         void *event = ADB_GetEvent(service, event_id);
         if (event != NULL)
         {
            info = Core::Service<EitExtendedEventImpl>::Create<IDTV::IEitExtendedEvent>(event);
            result = Core::ERROR_NONE;
         }
      }

      return result;
   }

   uint32_t DTVImpl::GetSignalInfo(const int32_t handle, ISignalInfo*& info) const
   {
      uint32_t result = Core::ERROR_BAD_REQUEST;

      if ((handle >= 0) && (handle < (int32_t)STB_DPGetNumPaths()))
      {
         info = Core::Service<SignalInfoImpl>::Create<IDTV::ISignalInfo>(handle);
         result = Core::ERROR_NONE;
      }

      return result;
   }

   uint32_t DTVImpl::AddLnb(const string lnb_name, const ILnb::LnbType type, const ILnb::LnbPower power,
      const ILnb::DiseqcTone tone, const ILnb::DiseqcCSwitch cswitch, const bool is22khz, const bool is12v,
      const bool is_pulse_pos, const bool is_diseqc_pos, const bool is_smatv, const uint8_t diseqc_repeats,
      const uint8_t uswitch, const uint8_t unicable_channel, const uint32_t unicable_freq)
   {
      uint32_t result = Core::ERROR_BAD_REQUEST;
      ADB_LNB_SETTINGS settings;

      memset(&settings, 0, sizeof(ADB_LNB_SETTINGS));

      settings.name = (U8BIT *)lnb_name.c_str();
      settings.type = GetDvbLnbType(type);
      settings.power = GetDvbLnbPower(power);
      settings.diseqc_tone = GetDvbDiseqcTone(tone);
      settings.c_switch = GetDvbDiseqcCSwitch(cswitch);
      settings.is_22k = (is22khz ? TRUE : FALSE);
      settings.is_12v = (is12v ? TRUE : FALSE);
      settings.is_pulse_posn = (is_pulse_pos ? TRUE : FALSE);
      settings.is_diseqc_posn = (is_diseqc_pos ? TRUE : FALSE);
      settings.is_smatv = (is_smatv ? TRUE : FALSE);
      settings.diseqc_repeats = diseqc_repeats;
      settings.u_switch = uswitch;
      settings.unicable_chan = unicable_channel;
      settings.unicable_if = unicable_freq;

      if (ADB_AddLNB(&settings) != NULL)
      {
         result = Core::ERROR_NONE;
      }

      return result;
   }

   uint32_t DTVImpl::AddSatellite(const string sat_name, const int16_t longitude, const string lnb_name)
   {
      uint32_t result = Core::ERROR_BAD_REQUEST;

      if (lnb_name.size() != 0)
      {
         BOOLEAN east_west;
         U16BIT value16;

         if (longitude < 0)
         {
            value16 = (U16BIT)-longitude;
            east_west = FALSE;
         }
         else
         {
            value16 = (U16BIT)longitude;
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
                  if (strcmp(lnb_name.c_str(), (char *)settings.name) == 0)
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
            if (ADB_AddSatellite((U8BIT *)sat_name.c_str(), 0, value16, east_west, lnb_ptr) != NULL)
            {
               result = Core::ERROR_NONE;
            }
         }
      }

      return result;
   }

   uint32_t DTVImpl::StartServiceSearch(const IDTV::TunerType tuner_type, const IDTV::ServiceSearchType search_type,
      const bool retune)
   {
      uint32_t result = Core::ERROR_BAD_REQUEST;
      E_STB_DP_SIGNAL_TYPE dvb_signal_type = GetDvbSignalType(tuner_type);
      E_ACTL_SEARCH_TYPE dvb_search_type;

      if (search_type == IDTV::ServiceSearchType::FREQUENCY)
      {
         dvb_search_type = ACTL_FREQ_SEARCH;
      }
      else
      {
         dvb_search_type = ACTL_NETWORK_SEARCH;
      }

      ADB_PrepareDatabaseForSearch(dvb_signal_type, NULL, (retune ? TRUE : FALSE), FALSE);

      if (ACTL_StartServiceSearch(dvb_signal_type, dvb_search_type))
      {
         result = Core::ERROR_NONE;
      }

      return result;
   }

   uint32_t DTVImpl::StartServiceSearch(const IDTV::ServiceSearchType search_type,
      const bool retune, const uint32_t freq_hz, const uint16_t symbol_rate,
      const IDvbcTuningParams::ModulationType modulation)
   {
      uint32_t result = Core::ERROR_BAD_REQUEST;
      E_ACTL_SEARCH_TYPE dvb_search_type;
      S_MANUAL_TUNING_PARAMS dvb_params;

      memset(&dvb_params, 0, sizeof(S_MANUAL_TUNING_PARAMS));

      dvb_params.freq = freq_hz;
      dvb_params.u.cab.symbol_rate = symbol_rate;

      switch (modulation)
      {
         case IDvbcTuningParams::ModulationType::MOD_4QAM:
            dvb_params.u.cab.mode = MODE_QAM_4;
            break;
         case IDvbcTuningParams::ModulationType::MOD_8QAM:
            dvb_params.u.cab.mode = MODE_QAM_8;
            break;
         case IDvbcTuningParams::ModulationType::MOD_16QAM:
            dvb_params.u.cab.mode = MODE_QAM_16;
            break;
         case IDvbcTuningParams::ModulationType::MOD_32QAM:
            dvb_params.u.cab.mode = MODE_QAM_32;
            break;
         case IDvbcTuningParams::ModulationType::MOD_64QAM:
            dvb_params.u.cab.mode = MODE_QAM_64;
            break;
         case IDvbcTuningParams::ModulationType::MOD_128QAM:
            dvb_params.u.cab.mode = MODE_QAM_128;
            break;
         case IDvbcTuningParams::ModulationType::MOD_256QAM:
            dvb_params.u.cab.mode = MODE_QAM_256;
            break;
         case IDvbcTuningParams::ModulationType::MOD_AUTO:
         default:
            dvb_params.u.cab.mode = MODE_QAM_AUTO;
            break;
      }

      if (search_type == IDTV::ServiceSearchType::FREQUENCY)
      {
         dvb_search_type = ACTL_FREQ_SEARCH;
      }
      else
      {
         dvb_search_type = ACTL_NETWORK_SEARCH;
      }

      ADB_PrepareDatabaseForSearch(SIGNAL_QAM, NULL, (retune ? TRUE : FALSE), FALSE);

      if (ACTL_StartManualSearch(SIGNAL_QAM, &dvb_params, dvb_search_type))
      {
         result = Core::ERROR_NONE;
      }

      return result;
   }

   uint32_t DTVImpl::StartServiceSearch(const IDTV::ServiceSearchType search_type,
      const bool retune, const string sat_name, const uint32_t freq_khz,
      const IDvbsTuningParams::PolarityType polarity, const uint16_t symbol_rate,
      const IDvbsTuningParams::FecType fec, const IDvbsTuningParams::ModulationType modulation, const bool dvbs2)
   {
      uint32_t result = Core::ERROR_BAD_REQUEST;
      E_ACTL_SEARCH_TYPE dvb_search_type;
      S_MANUAL_TUNING_PARAMS dvb_params;

      memset(&dvb_params, 0, sizeof(S_MANUAL_TUNING_PARAMS));

      dvb_params.u.sat.satellite = FindSatellite(sat_name.c_str());

      if (dvb_params.u.sat.satellite != NULL)
      {
         dvb_params.freq = freq_khz;
         dvb_params.u.sat.symbol_rate = symbol_rate;

         switch(polarity)
         {
            case IDvbsTuningParams::PolarityType::HORIZONTAL:
               dvb_params.u.sat.polarity = POLARITY_HORIZONTAL;
               break;
            case IDvbsTuningParams::PolarityType::VERTICAL:
               dvb_params.u.sat.polarity = POLARITY_VERTICAL;
               break;
            case IDvbsTuningParams::PolarityType::LEFT:
               dvb_params.u.sat.polarity = POLARITY_LEFT;
               break;
            case IDvbsTuningParams::PolarityType::RIGHT:
               dvb_params.u.sat.polarity = POLARITY_RIGHT;
               break;
         }

         switch (modulation)
         {
            case IDvbsTuningParams::ModulationType::MOD_QPSK:
               dvb_params.u.sat.modulation = MOD_QPSK;
               break;
            case IDvbsTuningParams::ModulationType::MOD_8PSK:
               dvb_params.u.sat.modulation = MOD_8PSK;
               break;
            case IDvbsTuningParams::ModulationType::MOD_16QAM:
               dvb_params.u.sat.modulation = MOD_16QAM;
               break;
            case IDvbsTuningParams::ModulationType::MOD_AUTO:
            default:
               dvb_params.u.sat.modulation = MOD_AUTO;
               break;
         }

         switch(fec)
         {
            case IDvbsTuningParams::FecType::FEC1_2:
               dvb_params.u.sat.fec = FEC_1_2;
               break;
            case IDvbsTuningParams::FecType::FEC2_3:
               dvb_params.u.sat.fec = FEC_2_3;
               break;
            case IDvbsTuningParams::FecType::FEC3_4:
               dvb_params.u.sat.fec = FEC_3_4;
               break;
            case IDvbsTuningParams::FecType::FEC5_6:
               dvb_params.u.sat.fec = FEC_5_6;
               break;
            case IDvbsTuningParams::FecType::FEC7_8:
               dvb_params.u.sat.fec = FEC_7_8;
               break;
            case IDvbsTuningParams::FecType::FEC1_4:
               dvb_params.u.sat.fec = FEC_1_4;
               break;
            case IDvbsTuningParams::FecType::FEC1_3:
               dvb_params.u.sat.fec = FEC_1_3;
               break;
            case IDvbsTuningParams::FecType::FEC2_5:
               dvb_params.u.sat.fec = FEC_2_5;
               break;
            case IDvbsTuningParams::FecType::FEC8_9:
               dvb_params.u.sat.fec = FEC_8_9;
               break;
            case IDvbsTuningParams::FecType::FEC9_10:
               dvb_params.u.sat.fec = FEC_9_10;
               break;
            case IDvbsTuningParams::FecType::FEC3_5:
               dvb_params.u.sat.fec = FEC_3_5;
               break;
            case IDvbsTuningParams::FecType::FEC4_5:
               dvb_params.u.sat.fec = FEC_4_5;
               break;
            case IDvbsTuningParams::FecType::FEC_AUTO:
            default:
               dvb_params.u.sat.fec = FEC_AUTOMATIC;
               break;
         }

         dvb_params.u.sat.dvb_s2 = (dvbs2 ? TRUE : FALSE);

         if (search_type == IDTV::ServiceSearchType::FREQUENCY)
         {
            dvb_search_type = ACTL_FREQ_SEARCH;
         }
         else
         {
            dvb_search_type = ACTL_NETWORK_SEARCH;
         }

         ADB_PrepareDatabaseForSearch(SIGNAL_QPSK, NULL, (retune ? TRUE : FALSE), FALSE);

         if (ACTL_StartManualSearch(SIGNAL_QPSK, &dvb_params, dvb_search_type))
         {
            result = Core::ERROR_NONE;
         }
      }

      return result;
   }

   uint32_t DTVImpl::StartServiceSearch(const IDTV::ServiceSearchType search_type,
      const bool retune, const uint32_t freq_hz, const IDvbtTuningParams::BandwidthType bandwidth,
      const IDvbtTuningParams::OfdmModeType mode, const bool dvbt2, const uint8_t plp_id)
   {
      uint32_t result = Core::ERROR_BAD_REQUEST;
      E_ACTL_SEARCH_TYPE dvb_search_type;
      S_MANUAL_TUNING_PARAMS dvb_params;

      memset(&dvb_params, 0, sizeof(S_MANUAL_TUNING_PARAMS));

      dvb_params.freq = freq_hz;

      switch (bandwidth)
      {
         case IDvbtTuningParams::BandwidthType::BW_5MHZ:
            dvb_params.u.terr.bwidth = TBWIDTH_5MHZ;
            break;
         case IDvbtTuningParams::BandwidthType::BW_6MHZ:
            dvb_params.u.terr.bwidth = TBWIDTH_6MHZ;
            break;
         case IDvbtTuningParams::BandwidthType::BW_7MHZ:
            dvb_params.u.terr.bwidth = TBWIDTH_7MHZ;
            break;
         case IDvbtTuningParams::BandwidthType::BW_8MHZ:
            dvb_params.u.terr.bwidth = TBWIDTH_8MHZ;
            break;
         case IDvbtTuningParams::BandwidthType::BW_10MHZ:
            dvb_params.u.terr.bwidth = TBWIDTH_10MHZ;
            break;
         case IDvbtTuningParams::BandwidthType::BW_UNDEFINED:
         default:
            dvb_params.u.terr.bwidth = TBWIDTH_UNDEFINED;
            break;
      }

      switch(mode)
      {
         case IDvbtTuningParams::OfdmModeType::OFDM_1K:
            dvb_params.u.terr.mode = MODE_COFDM_1K;
            break;
         case IDvbtTuningParams::OfdmModeType::OFDM_2K:
            dvb_params.u.terr.mode = MODE_COFDM_2K;
            break;
         case IDvbtTuningParams::OfdmModeType::OFDM_4K:
            dvb_params.u.terr.mode = MODE_COFDM_4K;
            break;
         case IDvbtTuningParams::OfdmModeType::OFDM_8K:
            dvb_params.u.terr.mode = MODE_COFDM_8K;
            break;
         case IDvbtTuningParams::OfdmModeType::OFDM_16K:
            dvb_params.u.terr.mode = MODE_COFDM_16K;
            break;
         case IDvbtTuningParams::OfdmModeType::OFDM_32K:
            dvb_params.u.terr.mode = MODE_COFDM_32K;
            break;
         case IDvbtTuningParams::OfdmModeType::OFDM_UNDEFINED:
         default:
            dvb_params.u.terr.mode = MODE_COFDM_UNDEFINED;
            break;
      }

      dvb_params.u.terr.type = (dvbt2 ? TERR_TYPE_DVBT2 : TERR_TYPE_DVBT);
      dvb_params.u.terr.plp_id = plp_id;

      if (search_type == IDTV::ServiceSearchType::FREQUENCY)
      {
         dvb_search_type = ACTL_FREQ_SEARCH;
      }
      else
      {
         dvb_search_type = ACTL_NETWORK_SEARCH;
      }

      ADB_PrepareDatabaseForSearch(SIGNAL_COFDM, NULL, (retune ? TRUE : FALSE), FALSE);

      if (ACTL_StartManualSearch(SIGNAL_COFDM, &dvb_params, dvb_search_type))
      {
         result = Core::ERROR_NONE;
      }

      return result;
   }

   uint32_t DTVImpl::FinishServiceSearch(const IDTV::TunerType tuner_type, const bool save_changes)
   {
      uint32_t result;
      E_STB_DP_SIGNAL_TYPE dvb_signal_type = GetDvbSignalType(tuner_type);

      if (dvb_signal_type != SIGNAL_NONE)
      {
         ADB_FinaliseDatabaseAfterSearch((save_changes ? TRUE : FALSE), dvb_signal_type, NULL, TRUE, TRUE, FALSE);
         result = Core::ERROR_NONE;
      }
      else
      {
         result = Core::ERROR_BAD_REQUEST;
      }

      return(result);
   }

   uint32_t DTVImpl::StartPlaying(const string dvburi, const bool monitor_only, int32_t& play_handle)
   {
      uint32_t result = Core::ERROR_BAD_REQUEST;
      U16BIT onet_id, trans_id, serv_id;

      if (std::sscanf(dvburi.c_str(), "%hu.%hu.%hu", &onet_id, &trans_id, &serv_id) == 3)
      {
         void *service = ADB_FindServiceByIds(onet_id, trans_id, serv_id);
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
         }
      }

      return result;
   }

   uint32_t DTVImpl::StartPlaying(const uint16_t lcn, const bool monitor_only, int32_t& play_handle)
   {
      uint32_t result = Core::ERROR_BAD_REQUEST;

      void *service = ADB_FindServiceByLcn(ADB_SERVICE_LIST_ALL, lcn, FALSE);
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
      }

      return result;
   }

   uint32_t DTVImpl::StopPlaying(const int32_t play_handle)
   {
      if (play_handle >= 0)
      {
         U8BIT decode_path = (U8BIT)play_handle;
         ACTL_TuneOff(decode_path);
         ACTL_ReleasePath(decode_path);
      }

      return (Core::ERROR_NONE);
   }

   void DTVImpl::DvbEventHandler(U32BIT event, void *event_data, U32BIT data_size)
   {
      switch (event)
      {
         case STB_EVENT_SEARCH_FAIL:
         case STB_EVENT_SEARCH_SUCCESS:
         case UI_EVENT_UPDATE:
         {
            //STB_SPDebugWrite("DTV::DvbEventHandler: event=0x%08x\n", event);
            DTVImpl::instance()->NotifySearchEvent();
            break;
         }

         case APP_EVENT_SERVICE_UPDATED:
         {
            DTVImpl::instance()->NotifyServiceEvent(IDTV::INotification::ServiceEventType::SERVICE_UPDATED, *(void **)event_data);
            break;
         }

         case APP_EVENT_SERVICE_ADDED:
         {
            DTVImpl::instance()->NotifyServiceEvent(IDTV::INotification::ServiceEventType::SERVICE_ADDED, *(void **)event_data);
            break;
         }

         case APP_EVENT_SERVICE_DELETED:
         {
            DTVImpl::instance()->NotifyServiceEvent(IDTV::INotification::ServiceEventType::SERVICE_DELETED, *(void **)event_data);
            break;
         }

         case APP_EVENT_SERVICE_VIDEO_CODEC_CHANGED:
         case APP_EVENT_SERVICE_VIDEO_PID_UPDATE:
         {
            DTVImpl::instance()->NotifyServiceEvent(IDTV::INotification::ServiceEventType::VIDEO_CHANGED, *(void **)event_data);
            break;
         }

         case APP_EVENT_SERVICE_AUDIO_CODEC_CHANGED:
         case APP_EVENT_SERVICE_AUDIO_PID_UPDATE:
         {
            DTVImpl::instance()->NotifyServiceEvent(IDTV::INotification::ServiceEventType::AUDIO_CHANGED, *(void **)event_data);
            break;
         }

         case APP_EVENT_SERVICE_SUBTITLE_UPDATE:
         {
            DTVImpl::instance()->NotifyServiceEvent(IDTV::INotification::ServiceEventType::SUBTITLES_CHANGED, *(void **)event_data);
            break;
         }

         case APP_EVENT_SERVICE_EIT_NOW_UPDATE:
         {
            /* Service, and hence event info, can only be provided if the service is available */
            if ((event_data != NULL) && (*(void **)event_data != NULL))
            {
               void *service = *(void **)event_data;
               void *now;

               ADB_GetNowNextEvents(service, &now, NULL);

               DTVImpl::instance()->NotifyServiceEvent(IDTV::INotification::ServiceEventType::NOW_EVENT_CHANGED,
                  service, now);

               if (now != NULL)
               {
                  ADB_ReleaseEventData(now);
               }
            }
            break;
         }

         default:
         {
            //STB_SPDebugWrite("DTV::DvbEventHandler: Unhandled event=0x%08x\n", event);
            break;
         }
      }
   }

   void DTVImpl::NotifySearchEvent(void)
   {
      uint8_t handle = ACTL_GetServiceSearchPath();
      bool finished = (ACTL_IsSearchComplete() ? true : false);
      uint8_t progress;

      if (finished)
      {
         progress = 100;
      }
      else
      {
         progress = ACTL_GetSearchProgress();
      }

      // Loop through all the registered callbacks and fire off the notification
      m_notification_mutex.Lock();

      for (const auto callback : m_notification_callbacks)
      {
         callback->SearchEvent(handle, finished, progress);
      }

      m_notification_mutex.Unlock();
   }

   void DTVImpl::NotifyServiceEvent(IDTV::INotification::ServiceEventType event_type, const void *service,
      const void *now_event)
   {
      IService *iserv;

      // Loop through all the registered callbacks and fire off the notification
      m_notification_mutex.Lock();

      for (const auto callback : m_notification_callbacks)
      {
         iserv = Core::Service<ServiceImpl>::Create<IDTV::IService>(service);

         if (now_event != nullptr)
         {
            IEitEvent *now = Core::Service<EitEventImpl>::Create<IDTV::IEitEvent>(now_event);
            callback->ServiceEvent(event_type, iserv, now);
         }
         else
         {
            callback->ServiceEvent(event_type, iserv);
         }
      }

      m_notification_mutex.Unlock();
   }

   E_STB_DP_SIGNAL_TYPE DTVImpl::GetDvbSignalType(IDTV::TunerType tuner_type) const
   {
      E_STB_DP_SIGNAL_TYPE signal;

      switch(tuner_type)
      {
         case TunerType::DVBS:
            signal = SIGNAL_QPSK;
            break;
         case TunerType::DVBT:
            signal = SIGNAL_COFDM;
            break;
         case TunerType::DVBC:
            signal = SIGNAL_QAM;
            break;
         default:
            signal = SIGNAL_NONE;
            break;
      }

      return(signal);
   }

   E_STB_DP_LNB_TYPE DTVImpl::GetDvbLnbType(ILnb::LnbType lnb_type) const
   {
      E_STB_DP_LNB_TYPE type;

      switch(lnb_type)
      {
         case ILnb::LnbType::SINGLE:
            type = LNB_TYPE_SINGLE;
            break;
         case ILnb::LnbType::UNICABLE:
            type = LNB_TYPE_UNICABLE;
            break;
         case ILnb::LnbType::USERDEFINED:
            type = LNB_TYPE_USER_DEFINED;
            break;
         case ILnb::LnbType::UNIVERSAL:
         default:
            type = LNB_TYPE_UNIVERSAL;
            break;
      }

      return(type);
   }

   E_STB_DP_LNB_POWER DTVImpl::GetDvbLnbPower(ILnb::LnbPower lnb_power) const
   {
      E_STB_DP_LNB_POWER power;

      switch(lnb_power)
      {
         case ILnb::LnbPower::POWER_ON:
            power = LNB_POWER_ON;
            break;
         case ILnb::LnbPower::POWER_AUTO:
            power = LNB_POWER_AUTO;
            break;
         case ILnb::LnbPower::POWER_OFF:
         default:
            power = LNB_POWER_OFF;
            break;
      }

      return(power);
   }

   E_STB_DP_DISEQC_TONE DTVImpl::GetDvbDiseqcTone(ILnb::DiseqcTone diseqc_tone) const
   {
      E_STB_DP_DISEQC_TONE tone;

      switch(diseqc_tone)
      {
         case ILnb::DiseqcTone::TONE_A:
            tone = DISEQC_TONE_A;
            break;
         case ILnb::DiseqcTone::TONE_B:
            tone = DISEQC_TONE_B;
            break;
         case ILnb::DiseqcTone::TONE_OFF:
         default:
            tone = DISEQC_TONE_OFF;
            break;
      }

      return(tone);
   }

   E_STB_DP_DISEQC_CSWITCH DTVImpl::GetDvbDiseqcCSwitch(ILnb::DiseqcCSwitch diseqc_cswitch) const
   {
      E_STB_DP_DISEQC_CSWITCH cswitch;

      switch(diseqc_cswitch)
      {
         case ILnb::DiseqcCSwitch::CS_A:
            cswitch = DISEQC_CSWITCH_A;
            break;
         case ILnb::DiseqcCSwitch::CS_B:
            cswitch = DISEQC_CSWITCH_B;
            break;
         case ILnb::DiseqcCSwitch::CS_C:
            cswitch = DISEQC_CSWITCH_C;
            break;
         case ILnb::DiseqcCSwitch::CS_D:
            cswitch = DISEQC_CSWITCH_D;
            break;
         case ILnb::DiseqcCSwitch::CS_OFF:
         default:
            cswitch = DISEQC_CSWITCH_OFF;
            break;
      }

      return(cswitch);
   }

   void* DTVImpl::FindSatellite(const char *satellite_name) const
   {
      U8BIT *name;

      if (STB_IsUnicodeString((U8BIT *)satellite_name))
      {
         // Given name is in DVB format so skip the first char which is the type indicator byte
         satellite_name++;
      }

      void *sat_ptr = ADB_GetNextSatellite(NULL);
      while (sat_ptr != NULL)
      {
         if ((name = ADB_GetSatelliteName(sat_ptr)) != NULL)
         {
            if (STB_IsUnicodeString(name))
            {
               // Satellite name is in DVB format so skip the first char which is the type indicator byte
               name++;
            }

            if (strcmp(satellite_name, (char *)name) == 0)
            {
               // Found the named satellite
               break;
            }
         }

         sat_ptr = ADB_GetNextSatellite(sat_ptr);
      }

      return(sat_ptr);
   }

   string DTVImpl::GetLangString(const U32BIT lang_code)
   {
      char lang[4];

      lang[0] = (char)((lang_code >> 16) & 0xff);
      lang[1] = (char)((lang_code >> 8) & 0xff);
      lang[2] = (char)(lang_code & 0xff);
      lang[3] = '\0';

      return string(lang);
   }
}
}

