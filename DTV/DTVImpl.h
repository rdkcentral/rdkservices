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

#pragma once

#include "Module.h"

#include <interfaces/IDTV.h>

extern "C"
{
   // DVB include files
   #include <techtype.h>
   #include <stbhwc.h>
   #include <stbdpc.h>
   #include <stbuni.h>

   #include <app.h>
   #include <ap_dbacc.h>
   #include <ap_cntrl.h>
};

namespace WPEFramework {
namespace Plugin {
   class DTVImpl : public Exchange::IDTV
   {
   public:
      DTVImpl();
      ~DTVImpl() override;

      static DTVImpl* instance(DTVImpl *dtv = nullptr);

      // Do not allow copy/move constructors
      DTVImpl(const DTVImpl&) = delete;
      DTVImpl& operator=(const DTVImpl&) = delete;

      BEGIN_INTERFACE_MAP(DTVImpl)
         INTERFACE_ENTRY(Exchange::IDTV)
      END_INTERFACE_MAP

   public:
      class CountryImpl : public IDTV::ICountry
      {
      public:
         class IteratorImpl : public IDTV::ICountry::IIterator
         {
         public:
            IteratorImpl() = delete;
            IteratorImpl(const IteratorImpl&) = delete;
            IteratorImpl& operator=(const IteratorImpl&) = delete;

            IteratorImpl(const std::list<IDTV::ICountry*>& container)
            {
               std::list<IDTV::ICountry*>::const_iterator index = container.begin();
               while (index != container.end())
               {
                  IDTV::ICountry* country = (*index);
                  country->AddRef();
                  m_list.push_back(country);
                  index++;
               }

               m_iterator = m_list.begin();
            }

            ~IteratorImpl() override
            {
               while (m_list.size() != 0)
               {
                  m_list.front()->Release();
                  m_list.pop_front();
               }
            }

         public:
            uint32_t Current(IDTV::ICountry*& country) const override
            {
               uint32_t result = Core::ERROR_INVALID_RANGE;

               if (m_iterator != m_list.end())
               {
                  country = (*m_iterator);
                  country->AddRef();
                  result = Core::ERROR_NONE;
               }

               return result;
            }

            uint32_t Next() override
            {
               uint32_t result = Core::ERROR_INVALID_RANGE;

               if (m_iterator != m_list.end())
               {
                  m_iterator++;
                  result = Core::ERROR_NONE;
               }

               return result;
            }

            uint32_t Reset() override
            {
               m_iterator = m_list.begin();
               return Core::ERROR_NONE;
            }

            BEGIN_INTERFACE_MAP(CountryImpl::IteratorImpl)
               INTERFACE_ENTRY(Exchange::IDTV::ICountry::IIterator)
            END_INTERFACE_MAP

         private:
            std::list<IDTV::ICountry*> m_list;
            std::list<IDTV::ICountry*>::iterator m_iterator;
         }; // class ICountry::IteratorImpl

      public:
         CountryImpl() = delete;
         CountryImpl(const CountryImpl&) = delete;
         CountryImpl& operator=(const CountryImpl&) = delete;

         CountryImpl(const char *name, const uint32_t code): m_name(name), m_code(code)
         {
         }

         uint32_t Name(string& name) const override
         {
            TRACE(Trace::Information, (_T("DTVImpl::CountryImpl::Name: %s"), m_name.c_str()));
            name = m_name;
            return Core::ERROR_NONE;
         }

         uint32_t Code(uint32_t& code) const override
         {
            TRACE(Trace::Information, (_T("DTVImpl::CountryImpl::Code: 0x%06x"), m_code));
            code = m_code;
            return Core::ERROR_NONE;
         }

         BEGIN_INTERFACE_MAP(CountryImpl)
            INTERFACE_ENTRY(Exchange::IDTV::ICountry)
         END_INTERFACE_MAP

      private:
         std::string m_name;
         uint32_t m_code;
      }; // class CountryImpl

      class ServiceImpl : public IDTV::IService
      {
      public:
         class IteratorImpl : public IDTV::IService::IIterator
         {
         public:
            IteratorImpl() = delete;
            IteratorImpl(const IteratorImpl&) = delete;
            IteratorImpl& operator=(const IteratorImpl&) = delete;

            IteratorImpl(const std::list<IDTV::IService*>& container)
            {
               std::list<IDTV::IService*>::const_iterator index = container.begin();
               while (index != container.end())
               {
                  IDTV::IService* service = (*index);
                  service->AddRef();
                  m_list.push_back(service);
                  index++;
               }

               m_iterator = m_list.begin();
            }

            ~IteratorImpl() override
            {
               while (m_list.size() != 0)
               {
                  m_list.front()->Release();
                  m_list.pop_front();
               }
            }

         public:
            uint32_t Current(IDTV::IService*& service) const override
            {
               uint32_t result = Core::ERROR_INVALID_RANGE;

               if (m_iterator != m_list.end())
               {
                  service  = (*m_iterator);
                  service ->AddRef();
                  result = Core::ERROR_NONE;
               }

               return result;
            }

            uint32_t Next() override
            {
               uint32_t result = Core::ERROR_INVALID_RANGE;

               if (m_iterator != m_list.end())
               {
                  m_iterator++;
                  result = Core::ERROR_NONE;
               }

               return result;
            }

            uint32_t Reset() override
            {
               m_iterator = m_list.begin();
               return Core::ERROR_NONE;
            }

            BEGIN_INTERFACE_MAP(ServiceImpl::IteratorImpl)
               INTERFACE_ENTRY(Exchange::IDTV::IService::IIterator)
            END_INTERFACE_MAP

         private:
            std::list<IDTV::IService*> m_list;
            std::list<IDTV::IService*>::iterator m_iterator;
         }; // class IService::IteratorImpl

         ServiceImpl() = delete;
         ServiceImpl(const ServiceImpl&) = delete;
         ServiceImpl& operator=(const ServiceImpl&) = delete;

         ServiceImpl(const void *service)
         {
            U8BIT *name;
            U16BIT onet_id, trans_id, serv_id;

            if ((name = ADB_GetServiceFullName((void *)service, FALSE)) != NULL)
            {
               // Ignore the UTF-8 lead byte
               m_fullname = string((char *)(name + 1));
               STB_ReleaseUnicodeString(name);
            }
            else
            {
               m_fullname = _T("");
            }

            if ((name = ADB_GetServiceShortName((void *)service, FALSE)) != NULL)
            {
               // Ignore the UTF-8 lead byte
               m_shortname = string((char *)(name + 1));
               STB_ReleaseUnicodeString(name);
            }
            else
            {
               m_shortname = _T("");
            }

            ADB_GetServiceIds((void *)service, &onet_id, &trans_id, &serv_id);

            m_dvburi = std::to_string(onet_id) + "." + std::to_string(trans_id) +
               "." + std::to_string(serv_id);

            ADB_SERVICE_TYPE serv_type = ADB_GetServiceType((void *)service);
            switch (serv_type)
            {
               case ADB_SERVICE_TYPE_TV:
                  m_type = IDTV::IService::ServiceType::TV;
                  break;
               case ADB_SERVICE_TYPE_RADIO:
                  m_type = IDTV::IService::ServiceType::RADIO;
                  break;
               case ADB_SERVICE_TYPE_TELETEXT:
                  m_type = IDTV::IService::ServiceType::TELETEXT;
                  break;
               case ADB_SERVICE_TYPE_NVOD_REF:
                  m_type = IDTV::IService::ServiceType::NVOD;
                  break;
               case ADB_SERVICE_TYPE_NVOD_TIMESHIFT:
                  m_type = IDTV::IService::ServiceType::NVOD_TIMESHIFT;
                  break;
               case ADB_SERVICE_TYPE_MOSAIC:
                  m_type = IDTV::IService::ServiceType::MOSAIC;
                  break;
               case ADB_SERVICE_TYPE_AVC_RADIO:
                  m_type = IDTV::IService::ServiceType::AVC_RADIO;
                  break;
               case ADB_SERVICE_TYPE_AVC_MOSAIC:
                  m_type = IDTV::IService::ServiceType::AVC_MOSAIC;
                  break;
               case ADB_SERVICE_TYPE_DATA:
                  m_type = IDTV::IService::ServiceType::DATA;
                  break;
               case ADB_SERVICE_TYPE_MPEG2_HD:
                  m_type = IDTV::IService::ServiceType::MPEG2_HD;
                  break;
               case ADB_SERVICE_TYPE_AVC_SD_TV:
                  m_type = IDTV::IService::ServiceType::AVC_SD_TV;
                  break;
               case ADB_SERVICE_TYPE_AVC_SD_NVOD_TIMESHIFT:
                  m_type = IDTV::IService::ServiceType::AVC_SD_NVOD_TIMESHIFT;
                  break;
               case ADB_SERVICE_TYPE_AVC_SD_NVOD_REF:
                  m_type = IDTV::IService::ServiceType::AVC_SD_NVOD;
                  break;
               case ADB_SERVICE_TYPE_HD_TV:
                  m_type = IDTV::IService::ServiceType::HD_TV;
                  break;
               case ADB_SERVICE_TYPE_AVC_HD_NVOD_TIMESHIFT:
                  m_type = IDTV::IService::ServiceType::AVC_HD_NVOD_TIMESHIFT;
                  break;
               case ADB_SERVICE_TYPE_AVC_HD_NVOD_REF:
                  m_type = IDTV::IService::ServiceType::AVC_HD_NVOD;
                  break;
               case ADB_SERVICE_TYPE_UHD_TV:
                  m_type = IDTV::IService::ServiceType::UHD_TV;
                  break;
               default:
                  m_type = IDTV::IService::ServiceType::UNKNOWN;
                  break;
            }

            m_lcn = ADB_GetServiceLcn((void *)service);
            m_scrambled = (ADB_GetServiceScrambledFlag((void *)service) ? true : false);
            m_has_ca_desc = (ADB_GetServiceHasCaDesc((void *)service) ? true : false);
            m_hidden = (ADB_GetServiceHiddenFlag((void *)service) ? true : false);
            m_selectable = (ADB_GetServiceSelectableFlag((void *)service) ? true : false);

            switch (ADB_GetServiceRunningStatus((void *)service))
            {
               case RUN_STATE_NOT_RUNNING:
                  m_run_state = IDTV::IService::RunState::RUN_STATE_NOT_RUNNING;
                  break;
               case RUN_STATE_STARTS_SOON:
                  m_run_state = IDTV::IService::RunState::RUN_STATE_STARTS_SOON;
                  break;
               case RUN_STATE_PAUSING:
                  m_run_state = IDTV::IService::RunState::RUN_STATE_PAUSING;
                  break;
               case RUN_STATE_RUNNING:
                  m_run_state = IDTV::IService::RunState::RUN_STATE_RUNNING;
                  break;
               case RUN_STATE_OFF_AIR:
                  m_run_state = IDTV::IService::RunState::RUN_STATE_OFF_AIR;
                  break;
               case RUN_STATE_UNDEFINED:
               default:
                  m_run_state = IDTV::IService::RunState::RUN_STATE_UNDEFINED;
                  break;
            }
         }

         uint32_t Fullname(string& fullname) const override
         {
            fullname = m_fullname;
            return Core::ERROR_NONE;
         }

         uint32_t Shortname(string& shortname) const override
         {
            shortname = m_shortname;
            return Core::ERROR_NONE;
         }

         uint32_t Dvburi(string& dvburi) const override
         {
            dvburi = m_dvburi;
            return Core::ERROR_NONE;
         }

         uint32_t Type(ServiceType& type) const override
         {
            type = m_type;
            return Core::ERROR_NONE;
         }

         uint32_t Lcn(uint16_t& lcn) const override
         {
            lcn = m_lcn;
            return Core::ERROR_NONE;
         }

         uint32_t Scrambled(bool& scrambled) const override
         {
            scrambled = m_scrambled;
            return Core::ERROR_NONE;
         }

         uint32_t HasCaDescriptor(bool& has_ca_desc) const override
         {
            has_ca_desc = m_has_ca_desc;
            return Core::ERROR_NONE;
         }

         uint32_t Hidden(bool& hidden) const override
         {
            hidden = m_hidden;
            return Core::ERROR_NONE;
         }

         uint32_t Selectable(bool& selectable) const override
         {
            selectable = m_selectable;
            return Core::ERROR_NONE;
         }

         uint32_t RunningStatus(RunState& run_state) const override
         {
            run_state = m_run_state;
            return Core::ERROR_NONE;
         }

         BEGIN_INTERFACE_MAP(ServiceImpl)
            INTERFACE_ENTRY(Exchange::IDTV::IService)
         END_INTERFACE_MAP

      private:
         string m_fullname;
         string m_shortname;
         string m_dvburi;
         ServiceType m_type;
         uint16_t m_lcn;
         bool m_scrambled;
         bool m_has_ca_desc;
         bool m_hidden;
         bool m_selectable;
         RunState m_run_state;
      }; // class ServiceImpl

      class DvbcTuningParamsImpl : public IDTV::IDvbcTuningParams
      {
      public:
         DvbcTuningParamsImpl() = delete;
         DvbcTuningParamsImpl(const DvbcTuningParamsImpl&) = delete;
         DvbcTuningParamsImpl& operator=(const DvbcTuningParamsImpl&) = delete;

         DvbcTuningParamsImpl(const uint32_t freq_hz, const uint16_t symbol_rate,
            const IDvbcTuningParams::ModulationType modulation) :
            m_frequency(freq_hz),
            m_symbolrate(symbol_rate),
            m_modulation(modulation)
         {
         }

         uint32_t Frequency(uint32_t& freq_hz) const override
         {
            freq_hz = m_frequency;
            return Core::ERROR_NONE;
         }

         uint32_t SymbolRate(uint16_t& sym_rate) const override
         {
            sym_rate = m_symbolrate;
            return Core::ERROR_NONE;
         }

         uint32_t Modulation(IDvbcTuningParams::ModulationType& modulation) const override
         {
            modulation = m_modulation;
            return Core::ERROR_NONE;
         }

         BEGIN_INTERFACE_MAP(DvbcTuningParamsImpl)
            INTERFACE_ENTRY(Exchange::IDTV::IDvbcTuningParams)
         END_INTERFACE_MAP

      private:
         uint32_t m_frequency;
         uint16_t m_symbolrate;
         IDvbcTuningParams::ModulationType m_modulation;
      }; // class DvbcTuningParamsImpl

      class DvbsTuningParamsImpl : public IDTV::IDvbsTuningParams
      {
      public:
         DvbsTuningParamsImpl() = delete;
         DvbsTuningParamsImpl(const DvbsTuningParamsImpl&) = delete;
         DvbsTuningParamsImpl& operator=(const DvbsTuningParamsImpl&) = delete;

         DvbsTuningParamsImpl(const string sat_name, const uint32_t freq_khz,
            const IDvbsTuningParams::PolarityType polarity, const uint16_t symbol_rate,
            const IDvbsTuningParams::FecType fec, const IDvbsTuningParams::ModulationType modulation,
            const bool dvbs2) :
            m_satname(sat_name),
            m_frequency(freq_khz),
            m_polarity(polarity),
            m_symbolrate(symbol_rate),
            m_fec(fec),
            m_modulation(modulation),
            m_dvbs2(dvbs2)
         {
SYSLOG(Logging::Notification, (_T("DvbsTuningParamsImpl: sat_name=%s"), m_satname.c_str()));
         }

         uint32_t Satellite(string& sat_name) const override
         {
            sat_name = m_satname;
SYSLOG(Logging::Notification, (_T("DvbsTuningParamsImpl::Satellite: sat_name=%s"), sat_name.c_str()));
            return Core::ERROR_NONE;
         }

         uint32_t Frequency(uint32_t& freq_khz) const override
         {
            freq_khz = m_frequency;
            return Core::ERROR_NONE;
         }

         uint32_t Polarity(IDvbsTuningParams::PolarityType& polarity) const override
         {
            polarity = m_polarity;
            return Core::ERROR_NONE;
         }

         uint32_t SymbolRate(uint16_t& sym_rate) const override
         {
            sym_rate = m_symbolrate;
            return Core::ERROR_NONE;
         }

         uint32_t Fec(IDvbsTuningParams::FecType& fec) const override
         {
            fec = m_fec;
            return Core::ERROR_NONE;
         }

         uint32_t Modulation(IDvbsTuningParams::ModulationType& modulation) const override
         {
            modulation = m_modulation;
            return Core::ERROR_NONE;
         }

         uint32_t DvbS2(bool& dvbs2) const override
         {
            dvbs2 = m_dvbs2;
            return Core::ERROR_NONE;
         }

         BEGIN_INTERFACE_MAP(DvbsTuningParamsImpl)
            INTERFACE_ENTRY(Exchange::IDTV::IDvbsTuningParams)
         END_INTERFACE_MAP

      private:
         string m_satname;
         uint32_t m_frequency;
         IDvbsTuningParams::PolarityType m_polarity;
         uint16_t m_symbolrate;
         IDvbsTuningParams::FecType m_fec;
         IDvbsTuningParams::ModulationType m_modulation;
         bool m_dvbs2;
      }; // class DvbsTuningParamsImpl

      class DvbtTuningParamsImpl : public IDTV::IDvbtTuningParams
      {
      public:
         DvbtTuningParamsImpl() = delete;
         DvbtTuningParamsImpl(const DvbtTuningParamsImpl&) = delete;
         DvbtTuningParamsImpl& operator=(const DvbtTuningParamsImpl&) = delete;

         DvbtTuningParamsImpl(const uint32_t freq_hz, const IDvbtTuningParams::BandwidthType bandwidth,
            const IDvbtTuningParams::OfdmModeType mode, const bool dvbt2, const uint8_t plp_id = 0) :
            m_frequency(freq_hz),
            m_bandwidth(bandwidth),
            m_mode(mode),
            m_dvbt2(dvbt2),
            m_plpid(plp_id)
         {
         }

         uint32_t Frequency(uint32_t& freq_hz) const override
         {
            freq_hz = m_frequency;
            return Core::ERROR_NONE;
         }

         uint32_t Bandwidth(IDvbtTuningParams::BandwidthType& bandwidth) const override
         {
            bandwidth = m_bandwidth;
            return Core::ERROR_NONE;
         }

         uint32_t OfdmMode(IDvbtTuningParams::OfdmModeType& mode) const override
         {
            mode = m_mode;
            return Core::ERROR_NONE;
         }

         uint32_t DvbT2(bool& dvbt2) const override
         {
            dvbt2 = m_dvbt2;
            return Core::ERROR_NONE;
         }

         uint32_t PlpId(uint8_t& plp_id) const override
         {
            plp_id = m_plpid;
            return Core::ERROR_NONE;
         }

         BEGIN_INTERFACE_MAP(DvbtTuningParamsImpl)
            INTERFACE_ENTRY(Exchange::IDTV::IDvbtTuningParams)
         END_INTERFACE_MAP

      private:
         uint32_t m_frequency;
         IDvbtTuningParams::BandwidthType m_bandwidth;
         IDvbtTuningParams::OfdmModeType m_mode;
         bool m_dvbt2;
         uint8_t m_plpid;
      }; // class DvbtTuningParamsImpl

      class LnbImpl : public IDTV::ILnb
      {
      public:
         class IteratorImpl : public IDTV::ILnb::IIterator
         {
         public:
            IteratorImpl() = delete;
            IteratorImpl(const IteratorImpl&) = delete;
            IteratorImpl& operator=(const IteratorImpl&) = delete;

            IteratorImpl(const std::list<IDTV::ILnb*>& container)
            {
               std::list<IDTV::ILnb*>::const_iterator index = container.begin();
               while (index != container.end())
               {
                  IDTV::ILnb* lnb = (*index);
                  lnb->AddRef();
                  m_list.push_back(lnb);
                  index++;
               }

               m_iterator = m_list.begin();
            }

            ~IteratorImpl() override
            {
               while (m_list.size() != 0)
               {
                  m_list.front()->Release();
                  m_list.pop_front();
               }
            }

         public:
            uint32_t Current(IDTV::ILnb*& lnb) const override
            {
               uint32_t result = Core::ERROR_INVALID_RANGE;

               if (m_iterator != m_list.end())
               {
                  lnb = (*m_iterator);
                  lnb->AddRef();
                  result = Core::ERROR_NONE;
               }

               return result;
            }

            uint32_t Next() override
            {
               uint32_t result = Core::ERROR_INVALID_RANGE;

               if (m_iterator != m_list.end())
               {
                  m_iterator++;
                  result = Core::ERROR_NONE;
               }

               return result;
            }

            uint32_t Reset() override
            {
               m_iterator = m_list.begin();
               return Core::ERROR_NONE;
            }

            BEGIN_INTERFACE_MAP(LnbImpl::IteratorImpl)
               INTERFACE_ENTRY(Exchange::IDTV::ILnb::IIterator)
            END_INTERFACE_MAP

         private:
            std::list<IDTV::ILnb*> m_list;
            std::list<IDTV::ILnb*>::iterator m_iterator;
         }; // class ILnb::IteratorImpl

      public:
         LnbImpl() = delete;
         LnbImpl(const LnbImpl&) = delete;
         LnbImpl& operator=(const LnbImpl&) = delete;

         LnbImpl(const void *lnb_ptr)
         {
            ADB_LNB_SETTINGS settings;

            ADB_GetLNBSettings((void *)lnb_ptr, &settings);

            U16BIT nchar;
            U8BIT *name = STB_ConvertStringToUTF8(settings.name, &nchar, TRUE, 0);
            if (name != NULL)
            {
               /* Ignore the leading UTF-8 indicator byte */
               m_name = string((char *)name + 1);
               STB_ReleaseUnicodeString(name);
            }

            switch(settings.type)
            {
               case LNB_TYPE_SINGLE:
                  m_type = ILnb::LnbType::SINGLE;
                  break;
               case LNB_TYPE_UNIVERSAL:
                  m_type = ILnb::LnbType::UNIVERSAL;
                  break;
               case LNB_TYPE_UNICABLE:
                  m_type = ILnb::LnbType::UNICABLE;
                  break; 
            }

            switch(settings.power)
            {
               case LNB_POWER_OFF:
                  m_power = ILnb::LnbPower::POWER_OFF;
                  break;
               case LNB_POWER_ON:
                  m_power = ILnb::LnbPower::POWER_ON;
                  break;
               case LNB_POWER_AUTO:
                  m_power = ILnb::LnbPower::POWER_AUTO;
                  break;
            }

            switch(settings.diseqc_tone)
            {
               case DISEQC_TONE_OFF:
                  m_tone = ILnb::DiseqcTone::TONE_OFF;
                  break;
               case DISEQC_TONE_A:
                  m_tone = ILnb::DiseqcTone::TONE_A;
                  break;
               case DISEQC_TONE_B:
                  m_tone = ILnb::DiseqcTone::TONE_B;
                  break;
            }

            switch(settings.c_switch)
            {
               case DISEQC_CSWITCH_OFF:
                  m_cswitch = ILnb::DiseqcCSwitch::CS_OFF;
                  break;
               case DISEQC_CSWITCH_A:
                  m_cswitch = ILnb::DiseqcCSwitch::CS_A;
                  break;
               case DISEQC_CSWITCH_B:
                  m_cswitch = ILnb::DiseqcCSwitch::CS_B;
                  break;
               case DISEQC_CSWITCH_C:
                  m_cswitch = ILnb::DiseqcCSwitch::CS_C;
                  break;
               case DISEQC_CSWITCH_D:
                  m_cswitch = ILnb::DiseqcCSwitch::CS_D;
                  break;
            }

            m_is22khz = (settings.is_22k ? true : false);
            m_is12v = (settings.is_12v ? true : false);
            m_is_pulse_pos = (settings.is_pulse_posn ? true : false);
            m_is_diseqc_pos = (settings.is_diseqc_posn ? true : false);
            m_is_smatv = (settings.is_smatv ? true : false);
            m_diseqc_repeats = settings.diseqc_repeats;
            m_uswitch = settings.u_switch;
            m_unicable_channel = settings.unicable_chan;
            m_unicable_freq = settings.unicable_if;
         }

         uint32_t Name(string& name) const override
         {
            name = m_name;
            return Core::ERROR_NONE;
         }

         uint32_t Type(LnbType& type) const override
         {
            type = m_type;
            return Core::ERROR_NONE;
         }

         uint32_t Power(LnbPower& power) const override
         {
            power = m_power;
            return Core::ERROR_NONE;
         }

         uint32_t Tone(DiseqcTone& tone) const override
         {
            tone = m_tone;
            return Core::ERROR_NONE;
         }

         uint32_t CSwitch(DiseqcCSwitch& cswitch) const override
         {
            cswitch = m_cswitch;
            return Core::ERROR_NONE;
         }

         uint32_t Is22kHz(bool& is_22khz) const override
         {
            is_22khz = m_is22khz;
            return Core::ERROR_NONE;
         }

         uint32_t Is12V(bool& is_12v) const override
         {
            is_12v = m_is12v;
            return Core::ERROR_NONE;
         }

         uint32_t IsPulsePosition(bool& is_pulse_pos) const override
         {
            is_pulse_pos = m_is_pulse_pos;
            return Core::ERROR_NONE;
         }

         uint32_t IsDiseqcPosition(bool& is_diseqc_pos) const override
         {
            is_diseqc_pos = m_is_diseqc_pos;
            return Core::ERROR_NONE;
         }

         uint32_t IsSmatv(bool& is_smatv) const override
         {
            is_smatv = m_is_smatv;
            return Core::ERROR_NONE;
         }

         uint32_t DiseqcRepeats(uint8_t& repeats) const override
         {
            repeats = m_diseqc_repeats;
            return Core::ERROR_NONE;
         }

         uint32_t USwitch(uint8_t& u_switch) const override
         {
            u_switch = m_uswitch;
            return Core::ERROR_NONE;
         }

         uint32_t UnicableChannel(uint8_t& channel) const override
         {
            channel = m_unicable_channel;
            return Core::ERROR_NONE;
         }

         uint32_t UnicableFreq(uint32_t& freq) const override
         {
            freq = m_unicable_freq;
            return Core::ERROR_NONE;
         }

         BEGIN_INTERFACE_MAP(LnbImpl)
            INTERFACE_ENTRY(Exchange::IDTV::ILnb)
         END_INTERFACE_MAP

      private:
         std::string m_name;
         ILnb::LnbType m_type;
         ILnb::LnbPower m_power;
         ILnb::DiseqcTone m_tone;
         ILnb::DiseqcCSwitch m_cswitch;
         bool m_is22khz;
         bool m_is12v;
         bool m_is_pulse_pos;
         bool m_is_diseqc_pos;
         bool m_is_smatv;
         uint8_t m_diseqc_repeats;
         uint8_t m_uswitch;
         uint8_t m_unicable_channel;
         uint32_t m_unicable_freq;
      }; // class LnbImpl

      class SatelliteImpl : public IDTV::ISatellite
      {
      public:
         class IteratorImpl : public IDTV::ISatellite::IIterator
         {
         public:
            IteratorImpl() = delete;
            IteratorImpl(const IteratorImpl&) = delete;
            IteratorImpl& operator=(const IteratorImpl&) = delete;

            IteratorImpl(const std::list<IDTV::ISatellite*>& container)
            {
               std::list<IDTV::ISatellite*>::const_iterator index = container.begin();
               while (index != container.end())
               {
                  IDTV::ISatellite* satellite = (*index);
                  satellite->AddRef();
                  m_list.push_back(satellite);
                  index++;
               }

               m_iterator = m_list.begin();
            }

            ~IteratorImpl() override
            {
               while (m_list.size() != 0)
               {
                  m_list.front()->Release();
                  m_list.pop_front();
               }
            }

         public:
            uint32_t Current(IDTV::ISatellite*& satellite) const override
            {
               uint32_t result = Core::ERROR_INVALID_RANGE;

               if (m_iterator != m_list.end())
               {
                  satellite = (*m_iterator);
                  satellite->AddRef();
                  result = Core::ERROR_NONE;
               }

               return result;
            }

            uint32_t Next() override
            {
               uint32_t result = Core::ERROR_INVALID_RANGE;

               if (m_iterator != m_list.end())
               {
                  m_iterator++;
                  result = Core::ERROR_NONE;
               }

               return result;
            }

            uint32_t Reset() override
            {
               m_iterator = m_list.begin();
               return Core::ERROR_NONE;
            }

            BEGIN_INTERFACE_MAP(SatelliteImpl::IteratorImpl)
               INTERFACE_ENTRY(Exchange::IDTV::ISatellite::IIterator)
            END_INTERFACE_MAP

         private:
            std::list<IDTV::ISatellite*> m_list;
            std::list<IDTV::ISatellite*>::iterator m_iterator;
         }; // class ISatellite::IteratorImpl

      public:
         SatelliteImpl() = delete;
         SatelliteImpl(const SatelliteImpl&) = delete;
         SatelliteImpl& operator=(const SatelliteImpl&) = delete;

         SatelliteImpl(const void *sat_ptr)
         {
            U16BIT nchar;
            U8BIT *name = STB_ConvertStringToUTF8(ADB_GetSatelliteName((void *)sat_ptr), &nchar, TRUE, 0);
            if (name != NULL)
            {
               /* Ignore the leading UTF-8 indicator byte */
               m_satname = string((char *)name + 1);
               STB_ReleaseUnicodeString(name);
            }

            int16_t longitude = ADB_GetSatelliteLongitude((void *)sat_ptr);
         
            if (ADB_GetSatelliteDirection((void *)sat_ptr))
            {
               // Longitude value is east, so the value is positive
               m_longitude = longitude;
            }
            else
            {
               m_longitude = -longitude;
            }

            void *lnb_ptr = ADB_GetSatelliteLNB((void *)sat_ptr);
            if (lnb_ptr != NULL)
            {
               ADB_LNB_SETTINGS settings;

               if (ADB_GetLNBSettings(lnb_ptr, &settings) && (settings.name != NULL))
               {
                  m_lnbname = string((char *)settings.name);
               }
            }
         }

         uint32_t Name(string& name) const override
         {
            name = m_satname;
            return Core::ERROR_NONE;
         }

         uint32_t Longitude(int16_t& longitude) const override
         {
            longitude = m_longitude;
            return Core::ERROR_NONE;
         }

         uint32_t Lnb(string& lnb_name) const override
         {
            lnb_name = m_lnbname;
            return Core::ERROR_NONE;
         }

         BEGIN_INTERFACE_MAP(SatelliteImpl)
            INTERFACE_ENTRY(Exchange::IDTV::ISatellite)
         END_INTERFACE_MAP

      private:
         std::string m_satname;
         int16_t m_longitude;
         std::string m_lnbname;
      }; // class SatelliteImpl

      class StatusImpl : public IDTV::IStatus
      {
      public:
         StatusImpl() = delete;
         StatusImpl(const StatusImpl&) = delete;
         StatusImpl& operator=(const StatusImpl&) = delete;

         StatusImpl(const int32_t handle)
         {
            U8BIT path = (U8BIT)handle;

            m_tuner = STB_DPGetPathTuner(path);
            m_demux = STB_DPGetPathDemux(path);

            void *service = ADB_GetTunedService(path);
            if (service != NULL)
            {
               U16BIT onet_id, trans_id, serv_id;

               m_pmtpid = ADB_GetServicePmtPid(service);
               m_lcn = ADB_GetServiceLcn(service);

               ADB_GetServiceIds(service, &onet_id, &trans_id, &serv_id);
               m_dvburi = std::to_string(onet_id) + "." + std::to_string(trans_id) +
                  "." + std::to_string(serv_id);
            }
            else
            {
               m_pmtpid = DVB_INVALID_ID;
               m_lcn = 0;
               m_dvburi = _T("");
            }
         }

         uint32_t Tuner(uint8_t& tuner) const override
         {
            tuner = m_tuner;
            return Core::ERROR_NONE;
         }

         uint32_t Demux(uint8_t& demux) const override
         {
            demux = m_demux;
            return Core::ERROR_NONE;
         }

         uint32_t PmtPid(uint16_t& pid) const override
         {
            pid = m_pmtpid;
            return Core::ERROR_NONE;
         }

         uint32_t Dvburi(string& dvburi) const override
         {
            dvburi = m_dvburi;
            return Core::ERROR_NONE;
         }

         uint32_t Lcn(uint16_t& lcn) const override
         {
            lcn = m_lcn;
            return Core::ERROR_NONE;
         }

         BEGIN_INTERFACE_MAP(StatusImpl)
            INTERFACE_ENTRY(Exchange::IDTV::IStatus)
         END_INTERFACE_MAP

      private:
         uint8_t m_tuner;
         uint8_t m_demux;
         uint16_t m_pmtpid;
         std::string m_dvburi;
         uint16_t m_lcn;
      }; // class StatusImpl

      class EitEventImpl : public IDTV::IEitEvent
      {
      public:
         class IteratorImpl : public IDTV::IEitEvent::IIterator
         {
         public:
            IteratorImpl() = delete;
            IteratorImpl(const IteratorImpl&) = delete;
            IteratorImpl& operator=(const IteratorImpl&) = delete;

            IteratorImpl(const std::list<IDTV::IEitEvent*>& container)
            {
               std::list<IDTV::IEitEvent*>::const_iterator index = container.begin();
               while (index != container.end())
               {
                  IDTV::IEitEvent* event = (*index);
                  event->AddRef();
                  m_list.push_back(event);
                  index++;
               }

               m_iterator = m_list.begin();
            }

            ~IteratorImpl() override
            {
               while (m_list.size() != 0)
               {
                  m_list.front()->Release();
                  m_list.pop_front();
               }
            }

         public:
            uint32_t Current(IDTV::IEitEvent*& event) const override
            {
               uint32_t result = Core::ERROR_INVALID_RANGE;

               if (m_iterator != m_list.end())
               {
                  event = (*m_iterator);
                  event->AddRef();
                  result = Core::ERROR_NONE;
               }

               return result;
            }

            uint32_t Next() override
            {
               uint32_t result = Core::ERROR_INVALID_RANGE;

               if (m_iterator != m_list.end())
               {
                  m_iterator++;
                  result = Core::ERROR_NONE;
               }

               return result;
            }

            uint32_t Reset() override
            {
               m_iterator = m_list.begin();
               return Core::ERROR_NONE;
            }

            BEGIN_INTERFACE_MAP(EitEventImpl::IteratorImpl)
               INTERFACE_ENTRY(Exchange::IDTV::IEitEvent::IIterator)
            END_INTERFACE_MAP

         private:
            std::list<IDTV::IEitEvent*> m_list;
            std::list<IDTV::IEitEvent*>::iterator m_iterator;
         }; // class IEitEvent::IteratorImpl

         class ContentDataImpl : public IDTV::IEitEvent::IContentData
         {
         public:
            class IteratorImpl : public IDTV::IEitEvent::IContentData::IIterator
            {
            public:
               IteratorImpl() = delete;
               IteratorImpl(const IteratorImpl&) = delete;
               IteratorImpl& operator=(const IteratorImpl&) = delete;

               IteratorImpl(const std::list<IDTV::IEitEvent::IContentData*>& container)
               {
                  std::list<IDTV::IEitEvent::IContentData*>::const_iterator index = container.begin();
                  while (index != container.end())
                  {
                     IDTV::IEitEvent::IContentData* data = (*index);
                     data->AddRef();
                     m_list.push_back(data);
                     index++;
                  }

                  m_iterator = m_list.begin();
               }

               ~IteratorImpl() override
               {
                  while (m_list.size() != 0)
                  {
                     m_list.front()->Release();
                     m_list.pop_front();
                  }
               }

            public:
               uint32_t Current(IDTV::IEitEvent::IContentData*& data) const override
               {
                  uint32_t result = Core::ERROR_INVALID_RANGE;

                  if (m_iterator != m_list.end())
                  {
                     data = (*m_iterator);
                     data->AddRef();
                     result = Core::ERROR_NONE;
                  }

                  return result;
               }

               uint32_t Next() override
               {
                  uint32_t result = Core::ERROR_INVALID_RANGE;

                  if (m_iterator != m_list.end())
                  {
                     m_iterator++;
                     result = Core::ERROR_NONE;
                  }

                  return result;
               }

               uint32_t Reset() override
               {
                  m_iterator = m_list.begin();
                  return Core::ERROR_NONE;
               }

               BEGIN_INTERFACE_MAP(ContentDataImpl::IteratorImpl)
                  INTERFACE_ENTRY(Exchange::IDTV::IEitEvent::IContentData::IIterator)
               END_INTERFACE_MAP

            private:
               std::list<IDTV::IEitEvent::IContentData*> m_list;
               std::list<IDTV::IEitEvent::IContentData*>::iterator m_iterator;
            }; // class IEitEvent::IContentData::IteratorImpl

         public:
            ContentDataImpl() = delete;
            ContentDataImpl(const ContentDataImpl&) = delete;
            ContentDataImpl& operator=(const ContentDataImpl&) = delete;

            ContentDataImpl(const uint8_t value) : m_value(value)
            {
            }

            uint32_t Value(uint8_t& value) const override
            {
               value = m_value;
               return Core::ERROR_NONE;
            }

            BEGIN_INTERFACE_MAP(ContentDataImpl)
               INTERFACE_ENTRY(Exchange::IDTV::IEitEvent::IContentData)
            END_INTERFACE_MAP

         private:
            uint8_t m_value;
         }; // class IEitEvent::ContentDataImpl

      public:
         EitEventImpl() = delete;
         EitEventImpl(const EitEventImpl&) = delete;
         EitEventImpl& operator=(const EitEventImpl&) = delete;

         EitEventImpl(const void *event)
         {
            U8BIT *str = ADB_GetEventName((void *)event);
            if (str != NULL)
            {
               /* Name is provided as UTF-8 so ignore the leading indicator byte */
               m_name = string((char *)str + 1);
               STB_ReleaseUnicodeString(str);
            }

            m_starttime = STB_GCConvertToTimestamp(ADB_GetEventStartDateTime((void *)event));

            U32DHMS dhms = ADB_GetEventDuration((void *)event);
            m_duration = ((DHMS_DAYS(dhms) * 24 + DHMS_HOUR(dhms)) * 60 + DHMS_MINS(dhms)) * 60 + DHMS_SECS(dhms);

            m_eventid = ADB_GetEventId((void *)event);

            str = ADB_GetEventDescription((void *)event);
            if (str != NULL)
            {
               /* Description is provided as UTF-8 so ignore the leading indicator byte */
               m_description = string((char *)str + 1);
               STB_ReleaseUnicodeString(str);
            }

            m_hassubs = (ADB_GetEventSubtitlesAvailFlag((void *)event) ? true : false);
            m_hasad = (ADB_GetEventAudioDescriptionFlag((void *)event) ? true : false);
            m_rating = ADB_GetEventParentalAge((void *)event);
            m_hasextendedinfo = (ADB_GetEventHasExtendedDescription((void *)event) ? true : false);

            U8BIT num_bytes;
            U8BIT *data = ADB_GetEventContentData((void *)event, &num_bytes);
            if (data != NULL)
            {
               IDTV::IEitEvent::IContentData *item;

               for (U8BIT num = 0; num < num_bytes; num++)
               {
                  item = Core::Service<ContentDataImpl>::Create<IDTV::IEitEvent::IContentData>((const uint8_t)data[num]);
                  item->AddRef();
                  m_contentdata.push_back(item);
               }
            }
         }

         ~EitEventImpl()
         {
            while (m_contentdata.size() != 0)
            {
               m_contentdata.front()->Release();
               m_contentdata.pop_front();
            }
         }

         uint32_t Name(string& name) const override
         {
            name = m_name;
            return Core::ERROR_NONE;
         }

         uint32_t StartTime(uint32_t& start_time) const override
         {
            start_time = m_starttime;
            return Core::ERROR_NONE;
         }

         uint32_t Duration(uint32_t& duration) const override
         {
            duration = m_duration;
            return Core::ERROR_NONE;
         }

         uint32_t EventId(uint16_t& event_id) const override
         {
            event_id = m_eventid;
            return Core::ERROR_NONE;
         }

         uint32_t ShortDescription(string& description) const override
         {
            description = m_description;
            return Core::ERROR_NONE;
         }

         uint32_t HasSubtitles(bool& has_subs) const override
         {
            has_subs = m_hassubs;
            return Core::ERROR_NONE;
         }

         uint32_t HasAudioDescription(bool& has_ad) const override
         {
            has_ad = m_hasad;
            return Core::ERROR_NONE;
         }

         uint32_t ParentalRating(uint8_t& rating) const override
         {
            rating = m_rating;
            return Core::ERROR_NONE;
         }

         uint32_t ContentData(IDTV::IEitEvent::IContentData::IIterator*& content_data) const override
         {
            content_data = Core::Service<ContentDataImpl::IteratorImpl>::Create<IDTV::IEitEvent::IContentData::IIterator>(m_contentdata);
            return Core::ERROR_NONE;
         }

         uint32_t HasExtendedInfo(bool& extended_info) const override
         {
            extended_info = m_hasextendedinfo;
            return Core::ERROR_NONE;
         }

         BEGIN_INTERFACE_MAP(EitEventImpl)
            INTERFACE_ENTRY(Exchange::IDTV::IEitEvent)
         END_INTERFACE_MAP

      private:
         std::string m_name;
         uint32_t m_starttime;
         uint32_t m_duration;
         uint16_t m_eventid;
         std::string m_description;
         bool m_hassubs;
         bool m_hasad;
         uint8_t m_rating;
         std::list<IDTV::IEitEvent::IContentData*> m_contentdata;
         bool m_hasextendedinfo;
      }; // class EitEventImpl

      class EitExtendedEventImpl : public IDTV::IEitExtendedEvent
      {
      public:
         class EitExtendedEventItemImpl : public IDTV::IEitExtendedEvent::IEitExtendedEventItem
         {
         public:
            class IteratorImpl : public IDTV::IEitExtendedEvent::IEitExtendedEventItem::IIterator
            {
            public:
               IteratorImpl() = delete;
               IteratorImpl(const IteratorImpl&) = delete;
               IteratorImpl& operator=(const IteratorImpl&) = delete;

               IteratorImpl(const std::list<IDTV::IEitExtendedEvent::IEitExtendedEventItem*>& container)
               {
                  std::list<IDTV::IEitExtendedEvent::IEitExtendedEventItem*>::const_iterator index = container.begin();
                  while (index != container.end())
                  {
                     IDTV::IEitExtendedEvent::IEitExtendedEventItem* item = (*index);
                     item->AddRef();
                     m_list.push_back(item);
                     index++;
                  }

                  m_iterator = m_list.begin();
               }

               ~IteratorImpl() override
               {
                  while (m_list.size() != 0)
                  {
                     m_list.front()->Release();
                     m_list.pop_front();
                  }
               }

            public:
               uint32_t Current(IDTV::IEitExtendedEvent::IEitExtendedEventItem*& item) const override
               {
                  uint32_t result = Core::ERROR_INVALID_RANGE;

                  if (m_iterator != m_list.end())
                  {
                     item = (*m_iterator);
                     item->AddRef();
                     result = Core::ERROR_NONE;
                  }

                  return result;
               }

               uint32_t Next() override
               {
                  uint32_t result = Core::ERROR_INVALID_RANGE;

                  if (m_iterator != m_list.end())
                  {
                     m_iterator++;
                     result = Core::ERROR_NONE;
                  }

                  return result;
               }

               uint32_t Reset() override
               {
                  m_iterator = m_list.begin();
                  return Core::ERROR_NONE;
               }

               BEGIN_INTERFACE_MAP(EitExtendedEventItemImpl::IteratorImpl)
                  INTERFACE_ENTRY(Exchange::IDTV::IEitExtendedEvent::IEitExtendedEventItem::IIterator)
               END_INTERFACE_MAP

            private:
               std::list<IDTV::IEitExtendedEvent::IEitExtendedEventItem*> m_list;
               std::list<IDTV::IEitExtendedEvent::IEitExtendedEventItem*>::iterator m_iterator;
            }; // class EitExtendedEventImpl::EitExtendedEventItemImpl::IteratorImpl

            EitExtendedEventItemImpl() = delete;
            EitExtendedEventItemImpl(const EitExtendedEventItemImpl&) = delete;
            EitExtendedEventItemImpl& operator=(const EitExtendedEventItemImpl&) = delete;

            EitExtendedEventItemImpl(const ADB_EVENT_ITEMIZED_INFO *extended_item)
            {
               /* Strings are provided as UTF-8 so ignore the leading indicator bytes */
               m_description = string((char *)extended_item->item_description + 1);
               m_item = string((char *)extended_item->item + 1);
            }

            uint32_t Description(string& desc) const override
            {
               desc = m_description;
               return Core::ERROR_NONE;
            }

            uint32_t Item(string& item) const override
            {
               item = m_item;
               return Core::ERROR_NONE;
            }

            BEGIN_INTERFACE_MAP(EitExtendedEventItemImpl)
               INTERFACE_ENTRY(Exchange::IDTV::IEitExtendedEvent::IEitExtendedEventItem)
            END_INTERFACE_MAP

         private:
            std::string m_description;
            std::string m_item;
         }; // class EitExtendedEventImpl::EitExtendedEventItemImpl

      public:
         EitExtendedEventImpl() = delete;
         EitExtendedEventImpl(const EitExtendedEventImpl&) = delete;
         EitExtendedEventImpl& operator=(const EitExtendedEventImpl&) = delete;

         EitExtendedEventImpl(const void *event)
         {
            U8BIT *desc = ADB_GetEventExtendedDescription((void *)event);
            if (desc != NULL)
            {
               /* Description is provided as UTF-8 so ignore the leading indicator byte */
               m_description = string((char *)desc + 1);
               STB_ReleaseUnicodeString(desc);
            }

            U16BIT num_items;
            ADB_EVENT_ITEMIZED_INFO* items = ADB_GetEventItemizedDescription((void *)event, &num_items);
            if (items != NULL)
            {
               IDTV::IEitExtendedEvent::IEitExtendedEventItem *item;

               for (U16BIT num = 0; num < num_items; num++)
               {
                  item = Core::Service<EitExtendedEventItemImpl>::Create<IDTV::IEitExtendedEvent::IEitExtendedEventItem>(&items[num]);
                  item->AddRef();
                  m_itemlist.push_back(item);
               }

               ADB_ReleaseEventItemizedInfo(items, num_items);
            }
         }

         ~EitExtendedEventImpl()
         {
            while (m_itemlist.size() != 0)
            {
               m_itemlist.front()->Release();
               m_itemlist.pop_front();
            }
         }

         uint32_t Description(string& desc) const override
         {
            desc = m_description;
            return Core::ERROR_NONE;
         }

         uint32_t Items(IEitExtendedEventItem::IIterator*& items) const override
         {
            items = Core::Service<EitExtendedEventItemImpl::IteratorImpl>::Create<IDTV::IEitExtendedEvent::IEitExtendedEventItem::IIterator>(m_itemlist);
            return Core::ERROR_NONE;
         }

         BEGIN_INTERFACE_MAP(EitExtendedEventImpl)
            INTERFACE_ENTRY(Exchange::IDTV::IEitExtendedEvent)
         END_INTERFACE_MAP

      private:
         std::string m_description;
         std::list<IDTV::IEitExtendedEvent::IEitExtendedEventItem*> m_itemlist;
      }; // class EitExtendedEventImpl

      class SignalInfoImpl : public IDTV::ISignalInfo
      {
      public:
         SignalInfoImpl() = delete;
         SignalInfoImpl(const SignalInfoImpl&) = delete;
         SignalInfoImpl& operator=(const SignalInfoImpl&) = delete;

         SignalInfoImpl(const int32_t handle)
         {
            U8BIT path = (U8BIT)handle;
            U8BIT tuner = STB_DPGetPathTuner(path);

            if (tuner != INVALID_RES_ID)
            {
               m_locked = (ACTL_IsTuned(path) ? true : false);
            }
            else
            {
               m_locked = false;
            }

            if (m_locked)
            {
               /* Get the current signal strength and quality */
               m_strength = STB_TuneGetSignalStrength(tuner);
               m_quality = STB_TuneGetDataIntegrity(tuner);
            }
            else
            {
               /* Use the signal strength and quality stored with the transport for the current path */
               void *transport = ADB_GetTunedTransport(path);

               m_strength = ADB_GetTransportTunedStrength(transport);
               m_quality = ADB_GetTransportTunedQuality(transport);
            }
         }

         uint32_t Locked(bool& locked) const override
         {
            locked = m_locked;
            return Core::ERROR_NONE;
         }

         uint32_t Strength(uint8_t& strength) const override
         {
            strength = m_strength;
            return Core::ERROR_NONE;
         }

         uint32_t Quality(uint8_t& quality) const override
         {
            quality = m_quality;
            return Core::ERROR_NONE;
         }

         BEGIN_INTERFACE_MAP(SignalInfoImpl)
            INTERFACE_ENTRY(Exchange::IDTV::ISignalInfo)
         END_INTERFACE_MAP

      private:
         bool m_locked;
         uint8_t m_strength;
         uint8_t m_quality;
      }; // class SignalInfoImpl

      class ComponentImpl : public IDTV::IComponent
      {
      public:
         class IteratorImpl : public IDTV::IComponent::IIterator
         {
         public:
            IteratorImpl() = delete;
            IteratorImpl(const IteratorImpl&) = delete;
            IteratorImpl& operator=(const IteratorImpl&) = delete;

            IteratorImpl(const std::list<IDTV::IComponent*>& container)
            {
               std::list<IDTV::IComponent*>::const_iterator index = container.begin();
               while (index != container.end())
               {
                  IDTV::IComponent* component = (*index);
                  component->AddRef();
                  m_list.push_back(component);
                  index++;
               }

               m_iterator = m_list.begin();
            }

            ~IteratorImpl() override
            {
               while (m_list.size() != 0)
               {
                  m_list.front()->Release();
                  m_list.pop_front();
               }
            }

         public:
            uint32_t Current(IDTV::IComponent*& component) const override
            {
               uint32_t result = Core::ERROR_INVALID_RANGE;

               if (m_iterator != m_list.end())
               {
                  component = (*m_iterator);
                  component->AddRef();
                  result = Core::ERROR_NONE;
               }

               return result;
            }

            uint32_t Next() override
            {
               uint32_t result = Core::ERROR_INVALID_RANGE;

               if (m_iterator != m_list.end())
               {
                  m_iterator++;
                  result = Core::ERROR_NONE;
               }

               return result;
            }

            uint32_t Reset() override
            {
               m_iterator = m_list.begin();
               return Core::ERROR_NONE;
            }

            BEGIN_INTERFACE_MAP(ComponentImpl::IteratorImpl)
               INTERFACE_ENTRY(Exchange::IDTV::IComponent::IIterator)
            END_INTERFACE_MAP

         private:
            std::list<IDTV::IComponent*> m_list;
            std::list<IDTV::IComponent*>::iterator m_iterator;
         }; // class IComponent::IteratorImpl

         class TagImpl : public IDTV::IComponent::ITag
         {
         public:
            class IteratorImpl : public IDTV::IComponent::ITag::IIterator
            {
            public:
               IteratorImpl() = delete;
               IteratorImpl(const IteratorImpl&) = delete;
               IteratorImpl& operator=(const IteratorImpl&) = delete;

               IteratorImpl(const std::list<IDTV::IComponent::ITag*>& container)
               {
                  std::list<IDTV::IComponent::ITag*>::const_iterator index = container.begin();
                  while (index != container.end())
                  {
                     IDTV::IComponent::ITag* tag = (*index);
                     tag->AddRef();
                     m_list.push_back(tag);
                     index++;
                  }

                  m_iterator = m_list.begin();
               }

               ~IteratorImpl() override
               {
                  while (m_list.size() != 0)
                  {
                     m_list.front()->Release();
                     m_list.pop_front();
                  }
               }

            public:
               uint32_t Current(IDTV::IComponent::ITag*& tag) const override
               {
                  uint32_t result = Core::ERROR_INVALID_RANGE;

                  if (m_iterator != m_list.end())
                  {
                     tag = (*m_iterator);
                     tag->AddRef();
                     result = Core::ERROR_NONE;
                  }

                  return result;
               }

               uint32_t Next() override
               {
                  uint32_t result = Core::ERROR_INVALID_RANGE;

                  if (m_iterator != m_list.end())
                  {
                     m_iterator++;
                     result = Core::ERROR_NONE;
                  }

                  return result;
               }

               uint32_t Reset() override
               {
                  m_iterator = m_list.begin();
                  return Core::ERROR_NONE;
               }

               BEGIN_INTERFACE_MAP(TagImpl::IteratorImpl)
                  INTERFACE_ENTRY(Exchange::IDTV::IComponent::ITag::IIterator)
               END_INTERFACE_MAP

            private:
               std::list<IDTV::IComponent::ITag*> m_list;
               std::list<IDTV::IComponent::ITag*>::iterator m_iterator;
            }; // class IComponent::ITag::IteratorImpl

         public:
            TagImpl() = delete;
            TagImpl(const TagImpl&) = delete;
            TagImpl& operator=(const TagImpl&) = delete;

            TagImpl(const uint8_t value) : m_value(value)
            {
            }

            uint32_t Tag(uint8_t& value) const override
            {
               value = m_value;
               return Core::ERROR_NONE;
            }

            BEGIN_INTERFACE_MAP(TagImpl)
               INTERFACE_ENTRY(Exchange::IDTV::IComponent::ITag)
            END_INTERFACE_MAP

         private:
            uint8_t m_value;
         }; // class IComponent::TagImpl

      public:
         ComponentImpl() = delete;
         ComponentImpl(const ComponentImpl&) = delete;
         ComponentImpl& operator=(const ComponentImpl&) = delete;

         ComponentImpl(const void *stream)
         {
            ADB_STREAM_TYPE stream_type = ADB_GetStreamType((void *)stream);

            switch (stream_type)
            {
               case ADB_VIDEO_STREAM:
                  m_type = IDTV::IComponent::ComponentType::VIDEO;
                  m_codec = IDTV::IComponent::CodecType::MPEG2;
                  break;
               case ADB_H264_VIDEO_STREAM:
                  m_type = IDTV::IComponent::ComponentType::VIDEO;
                  m_codec = IDTV::IComponent::CodecType::H264;
                  break;
               case ADB_H265_VIDEO_STREAM:
                  m_type = IDTV::IComponent::ComponentType::VIDEO;
                  m_codec = IDTV::IComponent::CodecType::H265;
                  break;
               case ADB_AVS_VIDEO_STREAM:
                  m_type = IDTV::IComponent::ComponentType::VIDEO;
                  m_codec = IDTV::IComponent::CodecType::AVS; 
                  break;
               case ADB_AUDIO_STREAM:
                  m_type = IDTV::IComponent::ComponentType::AUDIO;
                  m_codec = IDTV::IComponent::CodecType::MPEG1;
                  break;
               case ADB_AAC_AUDIO_STREAM:
                  m_type = IDTV::IComponent::ComponentType::AUDIO;
                  m_codec = IDTV::IComponent::CodecType::AAC;
                  break;
               case ADB_HEAAC_AUDIO_STREAM:
                  m_type = IDTV::IComponent::ComponentType::AUDIO;
                  m_codec = IDTV::IComponent::CodecType::HEAAC;
                  break;
               case ADB_HEAACv2_AUDIO_STREAM:
                  m_type = IDTV::IComponent::ComponentType::AUDIO;
                  m_codec = IDTV::IComponent::CodecType::HEAACV2;
                  break;
               case ADB_AC3_AUDIO_STREAM:
                  m_type = IDTV::IComponent::ComponentType::AUDIO;
                  m_codec = IDTV::IComponent::CodecType::AC3;
                  break;
               case ADB_EAC3_AUDIO_STREAM:
                  m_type = IDTV::IComponent::ComponentType::AUDIO;
                  m_codec = IDTV::IComponent::CodecType::EAC3;
                  break;
               case ADB_SUBTITLE_STREAM:
                  m_type = IDTV::IComponent::ComponentType::SUBTITLES;
                  break;
               case ADB_TTEXT_STREAM:
                  m_type = IDTV::IComponent::ComponentType::TELETEXT;
                  break;
               default:
                  m_type = IDTV::IComponent::ComponentType::DATA;
                  break;
            }

            m_pid = ADB_GetStreamPID((void *)stream);

            U8BIT num_tags = ADB_GetStreamNumTags((void *)stream);
            IDTV::IComponent::ITag *item;

            for (U8BIT index = 0; index < num_tags; index++)
            {
               item = Core::Service<TagImpl>::Create<IDTV::IComponent::ITag>(ADB_GetStreamTag((void *)stream, index));
               item->AddRef();
               m_tags.push_back(item);
            }

            if (m_type == IDTV::IComponent::ComponentType::AUDIO)
            {
               m_language = DTVImpl::GetLangString(ADB_GetAudioStreamLangCode((void *)stream));

               switch (ADB_GetAudioStreamType((void *)stream))
               {
                  case ADB_AUDIO_TYPE_UNDEFINED:
                     m_audiotype = IDTV::IComponent::AudType::AUDIOTYPE_UNDEFINED;
                     break;
                  case ADB_AUDIO_TYPE_CLEAN_EFFECTS:
                     m_audiotype = IDTV::IComponent::AudType::AUDIOTYPE_CLEAN;
                     break;
                  case ADB_AUDIO_TYPE_FOR_HEARING_IMPAIRED:
                     m_audiotype = IDTV::IComponent::AudType::AUDIOTYPE_HEARINGIMPAIRED;
                     break;
                  case ADB_AUDIO_TYPE_FOR_VISUALLY_IMPAIRED:
                     m_audiotype = IDTV::IComponent::AudType::AUDIOTYPE_VISUALLYIMPAIRED;
                     break;
                  default:
                     m_audiotype = IDTV::IComponent::AudType::AUDIOTYPE_UNKNOWN;
                     break;
               }

               switch (ADB_GetAudioStreamMode((void *)stream))
               {  
                  case AUDIO_STEREO:
                     m_audiomode = IDTV::IComponent::AudMode::AUDIOMODE_STEREO;
                     break;
                  case AUDIO_LEFT:
                     m_audiomode = IDTV::IComponent::AudMode::AUDIOMODE_LEFT;
                     break;
                  case AUDIO_RIGHT:
                     m_audiomode = IDTV::IComponent::AudMode::AUDIOMODE_RIGHT;
                     break;
                  case AUDIO_MONO:
                     m_audiomode = IDTV::IComponent::AudMode::AUDIOMODE_MONO;
                     break;
                  case AUDIO_MULTICHANNEL:
                     m_audiomode = IDTV::IComponent::AudMode::AUDIOMODE_MULTICHANNEL;
                     break;
                  case AUDIO_UNDEF:
                  default:
                     m_audiomode = IDTV::IComponent::AudMode::AUDIOMODE_UNDEFINED;
                     break;
               }
            }
            else if (m_type == IDTV::IComponent::ComponentType::SUBTITLES)
            {
               m_language = DTVImpl::GetLangString(ADB_GetSubtitleStreamLangCode((void *)stream));

               switch (ADB_GetSubtitleStreamType((void *)stream))
               {
                  case ADB_SUBTITLE_TYPE_DVB:
                     m_subformat = IDTV::IComponent::SubFormat::FORMAT_DEFAULT;
                     break;
                  case ADB_SUBTITLE_TYPE_DVB_4_3:
                     m_subformat = IDTV::IComponent::SubFormat::FORMAT_4_3;
                     break;
                  case ADB_SUBTITLE_TYPE_DVB_16_9:
                     m_subformat = IDTV::IComponent::SubFormat::FORMAT_16_9;
                     break;
                  case ADB_SUBTITLE_TYPE_DVB_221_1:
                     m_subformat = IDTV::IComponent::SubFormat::FORMAT_221_1;
                     break;
                  case ADB_SUBTITLE_TYPE_DVB_HD:
                     m_subformat = IDTV::IComponent::SubFormat::FORMAT_HD;
                     break;
                  case ADB_SUBTITLE_TYPE_DVB_HARD_HEARING:
                     m_subformat = IDTV::IComponent::SubFormat::FORMAT_HARDOFHEARING;
                     break;
                  case ADB_SUBTITLE_TYPE_DVB_HARD_HEARING_4_3:
                     m_subformat = IDTV::IComponent::SubFormat::FORMAT_HARDOFHEARING4_3;
                     break;
                  case ADB_SUBTITLE_TYPE_DVB_HARD_HEARING_16_9:
                     m_subformat = IDTV::IComponent::SubFormat::FORMAT_HARDOFHEARING16_9;
                     break;
                  case ADB_SUBTITLE_TYPE_DVB_HARD_HEARING_221_1:
                     m_subformat = IDTV::IComponent::SubFormat::FORMAT_HARDOFHEARING221_1;
                     break;
                  case ADB_SUBTITLE_TYPE_DVB_HARD_HEARING_HD:
                     m_subformat = IDTV::IComponent::SubFormat::FORMAT_HARDOFHEARINGHD;
                     break;
               }

               m_composition = ADB_GetSubtitleStreamCompositionPage((void *)stream);
               m_ancillary = ADB_GetSubtitleStreamAncillaryPage((void *)stream);
            }
            else if (m_type == IDTV::IComponent::ComponentType::TELETEXT)
            {
               m_language = DTVImpl::GetLangString(ADB_GetSubtitleStreamLangCode((void *)stream));
               m_ttexttype = ADB_GetTtextStreamType((void *)stream);
               m_magazine = ADB_GetTtextStreamMagazine((void *)stream);
               m_page = ADB_GetTtextStreamPage((void *)stream);
            }
         }

         ~ComponentImpl()
         {
            while (m_tags.size() != 0)
            {
               m_tags.front()->Release();
               m_tags.pop_front();
            }
         }

         uint32_t Type(IDTV::IComponent::ComponentType& type) const override
         {
            type = m_type;
            return Core::ERROR_NONE;
         }

         uint32_t Tags(IDTV::IComponent::ITag::IIterator*& tags) const override
         {
            tags = Core::Service<TagImpl::IteratorImpl>::Create<IDTV::IComponent::ITag::IIterator>(m_tags);
            return Core::ERROR_NONE;
         }

         uint32_t Pid(uint16_t& pid) const override
         {
            pid = m_pid;
            return Core::ERROR_NONE;
         }

         uint32_t Codec(CodecType& codec) const override
         {
            uint32_t result = Core::ERROR_BAD_REQUEST;

            if ((m_type == IDTV::IComponent::ComponentType::VIDEO) ||
               (m_type == IDTV::IComponent::ComponentType::AUDIO))
            {
               codec = m_codec;
               result = Core::ERROR_NONE;
            }

            return result;
         }

         uint32_t Language(string& lang) const override
         {
            uint32_t result = Core::ERROR_BAD_REQUEST;

            if ((m_type == IDTV::IComponent::ComponentType::AUDIO) ||
               (m_type == IDTV::IComponent::ComponentType::SUBTITLES) ||
               (m_type == IDTV::IComponent::ComponentType::TELETEXT))
            {
               lang = m_language;
               result = Core::ERROR_NONE;
            }

            return result;
         }

         uint32_t AudioType(IDTV::IComponent::AudType& type) const override
         {
            uint32_t result = Core::ERROR_BAD_REQUEST;

            if (m_type == IDTV::IComponent::ComponentType::AUDIO)
            {
               type = m_audiotype;
               result = Core::ERROR_NONE;
            }

            return result;
         }

         uint32_t AudioMode(IDTV::IComponent::AudMode& mode) const override
         {
            uint32_t result = Core::ERROR_BAD_REQUEST;

            if (m_type == IDTV::IComponent::ComponentType::AUDIO)
            {
               mode = m_audiomode;
               result = Core::ERROR_NONE;
            }

            return result;
         }

         uint32_t SubtitleFormat(IDTV::IComponent::SubFormat& format) const override
         {
            uint32_t result = Core::ERROR_BAD_REQUEST;

            if (m_type == IDTV::IComponent::ComponentType::SUBTITLES)
            {
               format = m_subformat;
               result = Core::ERROR_NONE;
            }

            return result;
         }

         uint32_t CompositionPage(uint16_t& page) const override
         {
            uint32_t result = Core::ERROR_BAD_REQUEST;

            if (m_type == IDTV::IComponent::ComponentType::SUBTITLES)
            {
               page = m_composition;
               result = Core::ERROR_NONE;
            }

            return result;
         }

         uint32_t AncillaryPage(uint16_t& page) const override
         {
            uint32_t result = Core::ERROR_BAD_REQUEST;

            if (m_type == IDTV::IComponent::ComponentType::SUBTITLES)
            {
               page = m_ancillary;
               result = Core::ERROR_NONE;
            }

            return result;
         }

         uint32_t TeletextType(uint8_t& type) const override
         {
            uint32_t result = Core::ERROR_BAD_REQUEST;

            if (m_type == IDTV::IComponent::ComponentType::TELETEXT)
            {
               type = m_ttexttype;
               result = Core::ERROR_NONE;
            }

            return result;
         }

         uint32_t TeletextMagazine(uint8_t& magazine) const override
         {
            uint32_t result = Core::ERROR_BAD_REQUEST;

            if (m_type == IDTV::IComponent::ComponentType::TELETEXT)
            {
               magazine = m_magazine;
               result = Core::ERROR_NONE;
            }

            return result;
         }

         uint32_t TeletextPage(uint8_t& page) const override
         {
            uint32_t result = Core::ERROR_BAD_REQUEST;

            if (m_type == IDTV::IComponent::ComponentType::TELETEXT)
            {
               page = m_page;
               result = Core::ERROR_NONE;
            }

            return result;
         }

         BEGIN_INTERFACE_MAP(ComponentImpl)
            INTERFACE_ENTRY(Exchange::IDTV::IComponent)
         END_INTERFACE_MAP

      private:
         IDTV::IComponent::ComponentType m_type;
         uint16_t m_pid;
         std::list<IDTV::IComponent::ITag*> m_tags;
         IDTV::IComponent::CodecType m_codec;
         std::string m_language;
         IDTV::IComponent::AudType m_audiotype;
         IDTV::IComponent::AudMode m_audiomode;
         IDTV::IComponent::SubFormat m_subformat;
         uint16_t m_composition;
         uint16_t m_ancillary;
         uint8_t m_ttexttype;
         uint8_t m_magazine;
         uint8_t m_page;
      }; // class ComponentImpl

      class TransportImpl : public IDTV::ITransport
      {
      public:
         TransportImpl() = delete;
         TransportImpl(const TransportImpl&) = delete;
         TransportImpl& operator=(const TransportImpl&) = delete;

         TransportImpl(const void *transport)
         {
            switch (ADB_GetTransportSignalType((void *)transport))
            {
               case SIGNAL_QAM:
               {
                  U32BIT frequency;
                  E_STB_DP_CMODE mod;
                  U16BIT symbol_rate;
                  IDTV::IDvbcTuningParams::ModulationType modulation;

                  m_tuner = IDTV::TunerType::DVBC;

                  ADB_GetTransportCableTuningParams((void *)transport, &frequency, &mod, &symbol_rate);

                  switch (mod)
                  {
                     case MODE_QAM_4:
                        modulation = IDvbcTuningParams::ModulationType::MOD_4QAM;
                        break;
                     case MODE_QAM_8:
                        modulation = IDvbcTuningParams::ModulationType::MOD_8QAM;
                        break;
                     case MODE_QAM_16:
                        modulation = IDvbcTuningParams::ModulationType::MOD_16QAM;
                        break;
                     case MODE_QAM_32:
                        modulation = IDvbcTuningParams::ModulationType::MOD_32QAM;
                        break;
                     case MODE_QAM_64:
                        modulation = IDvbcTuningParams::ModulationType::MOD_64QAM;
                        break;
                     case MODE_QAM_128:
                        modulation = IDvbcTuningParams::ModulationType::MOD_128QAM;
                        break;
                     case MODE_QAM_256:
                        modulation = IDvbcTuningParams::ModulationType::MOD_256QAM;
                        break;
                     case MODE_QAM_AUTO:
                     default:
                        modulation = IDvbcTuningParams::ModulationType::MOD_AUTO;
                        break;
                  }

                  m_dvbc_params = Core::Service<DvbcTuningParamsImpl>::Create<IDTV::IDvbcTuningParams>(frequency,
                     symbol_rate, modulation);
                  break;
               }
               case SIGNAL_QPSK:
               {
                  U32BIT frequency;
                  E_STB_DP_POLARITY pol;
                  U16BIT symbol_rate;
                  E_STB_DP_FEC dvbs_fec;
                  BOOLEAN dvb_s2;
                  E_STB_DP_MODULATION mod;
                  IDvbsTuningParams::PolarityType polarity;
                  IDvbsTuningParams::FecType fec;
                  IDvbsTuningParams::ModulationType modulation;

                  m_tuner = IDTV::TunerType::DVBS;

                  ADB_GetTransportSatTuningParams((void *)transport, &frequency, &pol, &symbol_rate, &dvbs_fec,
                     &dvb_s2, &mod);

                  switch(pol)
                  {
                     case POLARITY_HORIZONTAL:
                        polarity = IDvbsTuningParams::PolarityType::HORIZONTAL;
                        break;
                     case POLARITY_VERTICAL:
                        polarity = IDvbsTuningParams::PolarityType::VERTICAL;
                        break;
                     case POLARITY_LEFT:
                        polarity = IDvbsTuningParams::PolarityType::LEFT;
                        break;
                     case POLARITY_RIGHT:
                        polarity = IDvbsTuningParams::PolarityType::RIGHT;
                        break;
                  }

                  switch (mod)
                  {
                     case MOD_QPSK:
                        modulation = IDvbsTuningParams::ModulationType::MOD_QPSK;
                        break;
                     case MOD_8PSK:
                        modulation = IDvbsTuningParams::ModulationType::MOD_8PSK;
                        break;
                     case MOD_16QAM:
                        modulation = IDvbsTuningParams::ModulationType::MOD_16QAM;
                        break;
                     case MOD_AUTO:
                     default:
                        modulation = IDvbsTuningParams::ModulationType::MOD_AUTO;
                        break;
                  }

                  switch(dvbs_fec)
                  {
                     case FEC_1_2:
                        fec = IDvbsTuningParams::FecType::FEC1_2;
                        break;
                     case FEC_2_3:
                        fec = IDvbsTuningParams::FecType::FEC2_3;
                        break;
                     case FEC_3_4:
                        fec = IDvbsTuningParams::FecType::FEC3_4;
                        break;
                     case FEC_5_6:
                        fec = IDvbsTuningParams::FecType::FEC5_6;
                        break;
                     case FEC_7_8:
                        fec = IDvbsTuningParams::FecType::FEC7_8;
                        break;
                     case FEC_1_4:
                        fec = IDvbsTuningParams::FecType::FEC1_4;
                        break;
                     case FEC_1_3:
                        fec = IDvbsTuningParams::FecType::FEC1_3;
                        break;
                     case FEC_2_5:
                        fec = IDvbsTuningParams::FecType::FEC2_5;
                        break;
                     case FEC_8_9:
                        fec = IDvbsTuningParams::FecType::FEC8_9;
                        break;
                     case FEC_9_10:
                        fec = IDvbsTuningParams::FecType::FEC9_10;
                        break;
                     case FEC_3_5:
                        fec = IDvbsTuningParams::FecType::FEC3_5;
                        break;
                     case FEC_4_5:
                        fec = IDvbsTuningParams::FecType::FEC4_5;
                        break;
                     case FEC_AUTOMATIC:
                     default:
                        fec = IDvbsTuningParams::FecType::FEC_AUTO;
                        break;
                  }

                  std::string satname;

                  void *satellite = ADB_GetTransportSatellite((void *)transport);
                  if (satellite != NULL)
                  {
                     U16BIT nchar;
                     U8BIT *name = STB_ConvertStringToUTF8(ADB_GetSatelliteName(satellite), &nchar, TRUE, 0);
                     if (name != NULL)
                     {
                        /* Ignore the leading UTF-8 indicator byte */
                        satname = string((char *)name + 1);
                        STB_ReleaseUnicodeString(name);
                     }
                  }

                  m_dvbs_params = Core::Service<DvbsTuningParamsImpl>::Create<IDTV::IDvbsTuningParams>(satname,
                     frequency, polarity, symbol_rate, fec, modulation, (dvb_s2 ? true : false));
                  break;
               }
               case SIGNAL_COFDM:
               {
                  E_STB_DP_TTYPE terr_type;
                  U32BIT frequency;
                  E_STB_DP_TMODE mode;
                  E_STB_DP_TBWIDTH bw;
                  U8BIT plp_id;
                  IDvbtTuningParams::BandwidthType bandwidth;
                  IDvbtTuningParams::OfdmModeType ofdm;
                  bool dvbt2;

                  m_tuner = IDTV::TunerType::DVBT;

                  ADB_GetTransportTerrestrialTuningParams((void *)transport, &terr_type, &frequency, &mode,
                     &bw, &plp_id);

                  switch (bw)
                  {
                     case TBWIDTH_5MHZ:
                        bandwidth = IDvbtTuningParams::BandwidthType::BW_5MHZ;
                        break;
                     case TBWIDTH_6MHZ:
                        bandwidth = IDvbtTuningParams::BandwidthType::BW_6MHZ;
                        break;
                     case TBWIDTH_7MHZ:
                        bandwidth = IDvbtTuningParams::BandwidthType::BW_7MHZ;
                        break;
                     case TBWIDTH_8MHZ:
                        bandwidth = IDvbtTuningParams::BandwidthType::BW_8MHZ;
                        break;
                     case TBWIDTH_10MHZ:
                        bandwidth = IDvbtTuningParams::BandwidthType::BW_10MHZ;
                        break;
                     case TBWIDTH_UNDEFINED:
                     default:
                        bandwidth = IDvbtTuningParams::BandwidthType::BW_UNDEFINED;
                        break;
                  }

                  switch(mode)
                  {
                     case MODE_COFDM_1K:
                        ofdm = IDvbtTuningParams::OfdmModeType::OFDM_1K;
                        break;
                     case MODE_COFDM_2K:
                        ofdm = IDvbtTuningParams::OfdmModeType::OFDM_2K;
                        break;
                     case MODE_COFDM_4K:
                        ofdm = IDvbtTuningParams::OfdmModeType::OFDM_4K;
                        break;
                     case MODE_COFDM_8K:
                        ofdm = IDvbtTuningParams::OfdmModeType::OFDM_8K;
                        break;
                     case MODE_COFDM_16K:
                        ofdm = IDvbtTuningParams::OfdmModeType::OFDM_16K;
                        break;
                     case MODE_COFDM_32K:
                        ofdm = IDvbtTuningParams::OfdmModeType::OFDM_32K;
                        break;
                     case MODE_COFDM_UNDEFINED:
                     default:
                        ofdm = IDvbtTuningParams::OfdmModeType::OFDM_UNDEFINED;
                        break;
                  }

                  dvbt2 = (terr_type == TERR_TYPE_DVBT2);

                  m_dvbt_params = Core::Service<DvbtTuningParamsImpl>::Create<IDTV::IDvbtTuningParams>(frequency,
                     bandwidth, ofdm, dvbt2, plp_id);
                  break;
               }
               case SIGNAL_NONE:
               default:
               {
                  m_tuner = IDTV::TunerType::NONE;
                  break;
               }
            }

            m_onet_id = ADB_GetTransportOriginalNetworkId((void *)transport);
            m_trans_id = ADB_GetTransportTid((void *)transport);
            m_strength = ADB_GetTransportTunedStrength((void *)transport);
            m_quality = ADB_GetTransportTunedQuality((void *)transport);
         }

         uint32_t Tuner(TunerType& type) const override
         {
            type = m_tuner;
            return Core::ERROR_NONE;
         }

         uint32_t OriginalNetworkId(uint16_t& onet_id) const override
         {
            onet_id = m_onet_id;
            return Core::ERROR_NONE;
         }

         uint32_t TransportId(uint16_t& trans_id) const override
         {
            trans_id = m_trans_id;
            return Core::ERROR_NONE;
         }

         uint32_t SignalStrength(uint8_t& strength) const override
         {
            strength = m_strength;
            return Core::ERROR_NONE;
         }

         uint32_t SignalQuality(uint8_t& quality) const override
         {
            quality = m_quality;
            return Core::ERROR_NONE;
         }

         uint32_t TuningParamsDvbc(IDvbcTuningParams*& params) const override
         {
            params = m_dvbc_params;
            return Core::ERROR_NONE;
         }

         uint32_t TuningParamsDvbs(IDvbsTuningParams*& params) const override
         {
            params = m_dvbs_params;
            return Core::ERROR_NONE;
         }

         uint32_t TuningParamsDvbt(IDvbtTuningParams*& params) const override
         {
            params = m_dvbt_params;
            return Core::ERROR_NONE;
         }

         BEGIN_INTERFACE_MAP(TransportImpl)
            INTERFACE_ENTRY(Exchange::IDTV::ITransport)
         END_INTERFACE_MAP

      private:
         IDTV::TunerType m_tuner;
         uint16_t m_onet_id;
         uint16_t m_trans_id;
         uint8_t m_strength;
         uint8_t m_quality;
         IDTV::IDvbcTuningParams *m_dvbc_params;
         IDTV::IDvbsTuningParams *m_dvbs_params;
         IDTV::IDvbtTuningParams *m_dvbt_params;
      }; // class TransportImpl

   public:
      uint32_t Initialize(PluginHost::IShell* framework) override;

      uint32_t Register(IDTV::INotification* notification) override;
      uint32_t Unregister(IDTV::INotification* notification) override;

      uint32_t GetNumberOfCountries(uint8_t& response) const override;
      uint32_t GetCountryList(IDTV::ICountry::IIterator*& countries) const override;
      uint32_t GetCountry(uint32_t& code) const override;
      uint32_t SetCountry(const uint32_t code) override;

      uint32_t GetLnbList(ILnb::IIterator*& lnbs) const override;
      uint32_t GetSatelliteList(ISatellite::IIterator*& satellites) const override;

      uint32_t GetNumberOfServices(uint16_t &count) const override;
      uint32_t GetServiceList(IService::IIterator*& services) const override;
      uint32_t GetServiceList(const IDTV::TunerType tuner_type, IService::IIterator*& services) const override;
      uint32_t GetServiceList(const uint16_t onet_id, const uint16_t trans_id, IService::IIterator*& services) const override;

      uint32_t GetNowNextEvents(const uint16_t onet_id, const uint16_t trans_id, const uint16_t serv_id,
         IEitEvent*& now_event, IEitEvent*& next_event) const override;
      uint32_t GetScheduleEvents(const uint16_t onet_id, const uint16_t trans_id, const uint16_t serv_id,
         const uint32_t start_utc, const uint32_t end_utc, IEitEvent::IIterator*& events) const override;

      uint32_t GetStatus(const int32_t handle, IStatus*& status) const override;

      uint32_t GetServiceInfo(const uint16_t onet_id, const uint16_t trans_id, const uint16_t serv_id,
         IService*& service_info) const override;
      uint32_t GetServiceComponents(const uint16_t onet_id, const uint16_t trans_id, const uint16_t serv_id,
         IComponent::IIterator*& components) const override;
      uint32_t GetTransportInfo(const uint16_t onet_id, const uint16_t trans_id,
         ITransport*& transport_info) const override;
      uint32_t GetExtendedEventInfo(const uint16_t onet_id, const uint16_t trans_id, const uint16_t serv_id,
         const uint16_t event_id, IEitExtendedEvent*& info) const override;
      uint32_t GetSignalInfo(const int32_t handle, ISignalInfo*& info) const override;

      uint32_t AddLnb(const string lnb_name, const ILnb::LnbType type, const ILnb::LnbPower power,
         const ILnb::DiseqcTone tone, const ILnb::DiseqcCSwitch cswitch, const bool is22khz, const bool is12v,
         const bool is_pulse_pos, const bool is_diseqc_pos, const bool is_smatv, const uint8_t diseqc_repeats,
         const uint8_t uswitch, const uint8_t unicable_channel, const uint32_t unicable_freq) override;
      uint32_t AddSatellite(const string sat_name, const int16_t longitude, const string lnb_name) override;

      uint32_t StartServiceSearch(const IDTV::TunerType tuner_type, const IDTV::ServiceSearchType search_type,
         const bool retune) override;
      uint32_t StartServiceSearch(const ServiceSearchType search_type, const bool retune,
         const uint32_t freq_hz, const uint16_t symbol_rate,
         const IDvbcTuningParams::ModulationType modulation) override;
      uint32_t StartServiceSearch(const ServiceSearchType search_type, const bool retune,
         const uint32_t freq_hz, const IDvbtTuningParams::BandwidthType bandwidth,
         const IDvbtTuningParams::OfdmModeType mode, const bool dvbt2, const uint8_t plp_id) override;
      uint32_t StartServiceSearch(const ServiceSearchType search_type, const bool retune,
         const string sat_name, const uint32_t freq_khz, const IDvbsTuningParams::PolarityType polarity,
         const uint16_t symbol_rate, const IDvbsTuningParams::FecType fec,
         const IDvbsTuningParams::ModulationType modulation, const bool dvbs2) override;
      uint32_t FinishServiceSearch(const TunerType tuner_type, const bool save_changes) override;

      uint32_t StartPlaying(const string dvburi, const bool monitor_only, int32_t& play_handle) override;
      uint32_t StartPlaying(const uint16_t lcn, const bool monitor_only, int32_t& play_handle) override;
      uint32_t StopPlaying(const int32_t play_handle) override;

   private:
      class Config : public Core::JSON::Container
      {
         public:
            Config() : Core::JSON::Container(),
               SubtitleProcessing(false),
               TeletextProcessing(false)
            {
                Add(_T("subtitleprocessing"), &SubtitleProcessing);
                Add(_T("teletextprocessing"), &TeletextProcessing);
            }

            ~Config() override = default;

         public:
            Core::JSON::Boolean SubtitleProcessing;
            Core::JSON::Boolean TeletextProcessing;
      };

   private:
      std::list<Exchange::IDTV::INotification*> m_notification_callbacks;
      Core::CriticalSection m_notification_mutex;

      static void DvbEventHandler(U32BIT event, void *event_data, U32BIT data_size);

      void NotifySearchEvent(void);
      void NotifyServiceEvent(IDTV::INotification::ServiceEventType event_type, const void *service,
         const void *now_event = nullptr);

      E_STB_DP_SIGNAL_TYPE GetDvbSignalType(IDTV::TunerType tuner_type) const;
      E_STB_DP_LNB_TYPE GetDvbLnbType(ILnb::LnbType lnb_type) const;
      E_STB_DP_LNB_POWER GetDvbLnbPower(ILnb::LnbPower lnb_power) const;
      E_STB_DP_DISEQC_TONE GetDvbDiseqcTone(ILnb::DiseqcTone diseqc_tone) const;
      E_STB_DP_DISEQC_CSWITCH GetDvbDiseqcCSwitch(ILnb::DiseqcCSwitch diseqc_cswitch) const;

      void* FindSatellite(const char *satellite_name) const;

      static string GetLangString(const U32BIT lang_code);
   };
}
}

