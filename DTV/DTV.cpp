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

extern "C"
{
   // DVB include files
   #include <stbhwc.h>
   #include <stbdpc.h>
   #include <stberc.h>

   #include <app.h>
   #include <ap_dbacc.h>
   #include <ap_cntrl.h>
   #include <ap_cfg.h>
};

namespace WPEFramework
{
   namespace Plugin
   {
      using namespace JsonData::DTV;

      SERVICE_REGISTRATION(DTV, 1, 0);

      //static Core::ProxyPoolType<Web::TextBody> _textBodies(2);
      static Core::ProxyPoolType<Web::JSONBodyType<DTV::Data>> jsonResponseDataFactory(1);
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

      /* encapsulated class Thread  */
      const string DTV::Initialize(PluginHost::IShell *service)
      {
         ASSERT(service != nullptr);
         ASSERT(_dtv == nullptr);
         ASSERT(_service == nullptr);
         ASSERT(_connectionId == 0);

         string message;

         _service = service;
         _service->AddRef();
         _skipURL = _service->WebPrefix().length();

         Config config;
         config.FromString(_service->ConfigLine());

         _service->Register(&_notification);

         _dtv = service->Root<Core::IUnknown>(_connectionId, 2000, _T("DTV"));
         if (_dtv != nullptr)
         {

            RegisterAll();

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
               SYSLOG(Logging::Startup, (_T("DTV initialised")));

               ACFG_COUNTRY_CONFIG country_config;

               memset(&country_config, 0, sizeof(ACFG_COUNTRY_CONFIG));

               country_config.country_name = country_name;
               country_config.country_code = COUNTRY_CODE_USERDEFINED;
               country_config.ter_rf_channel_table_ptr = dvbt_tuning_table;
               country_config.num_ter_rf_channels = sizeof(dvbt_tuning_table) / sizeof(dvbt_tuning_table[0]);

               SYSLOG(Logging::Startup, (_T("DTV %u freqs to scan"), country_config.num_ter_rf_channels));

               ACFG_SetCountryConfig(COUNTRY_CODE_USERDEFINED, &country_config);
               ACFG_SetCountry(COUNTRY_CODE_USERDEFINED);
            }
            else
            {
               SYSLOG(Logging::Startup, (_T("DTV::Initialize: Failed to init DVBCore")));
               message = _T("DVBCore could not be initialised");
            }
         }
         else
         {
            SYSLOG(Logging::Startup, (_T("DTV::Initialize: Failed to create _dtv")));
            message = _T("DTV plugin could not be initialised");
         }

         if (message.length() != 0) {
            Deinitialize(service);
         }

         return message;
      }

      void DTV::Deinitialize(PluginHost::IShell *service)
      {
         ASSERT(_service == service);

         SYSLOG(Logging::Shutdown, (string(_T("DTV::Deinitialize"))));
         // Make sure the Activated and Deactivated are no longer called before we
         // start cleaning up..
         _service->Unregister(&_notification);

         if (_dtv != nullptr) {

            UnregisterAll();

            // Stop processing:
            RPC::IRemoteConnection* connection = service->RemoteConnection(_connectionId);

            VARIABLE_IS_NOT_USED uint32_t result = _dtv->Release();
            _dtv = nullptr;

            // It should have been the last reference we are releasing,
            // so it should endup in a DESTRUCTION_SUCCEEDED, if not we
            // are leaking...
            ASSERT(result == Core::ERROR_DESTRUCTION_SUCCEEDED);

            // If this was running in a (container) process...
            if (connection != nullptr) {
               // Lets trigger the cleanup sequence for
               // out-of-process code. Which will guard
               // that unwilling processes, get shot if
               // not stopped friendly :-)
               connection->Terminate();
               connection->Release();
            }
         }

         _connectionId = 0;
         _service->Release();
         _service = nullptr;
         SYSLOG(Logging::Shutdown, (string(_T("DTV de-initialised"))));
      }

      string DTV::Information() const
      {
         // No additional info to report.
         return (string());
      }

      void DTV::Inbound(WPEFramework::Web::Request &request)
      {
         Core::TextSegmentIterator index(Core::TextFragment(request.Path, _skipURL,
            static_cast<uint32_t>(request.Path.length() - _skipURL)), false, '/');

         // By default, we are in front of any element, jump onto the first element, which is if, there is something an empty slot.
         index.Next();

         if ((request.Verb == Web::Request::HTTP_PUT) || (request.Verb == Web::Request::HTTP_POST))
         {
            request.Body(jsonResponseDataFactory.Element());
         }
      }

      Core::ProxyType<Web::Response> DTV::Process(const Web::Request &request)
      {
         ASSERT(_skipURL <= request.Path.length());
         TRACE(Trace::Information, (string(_T("Received DTV request"))));

         Core::ProxyType<Web::Response> result(PluginHost::Factories::Instance().Response());

         Core::TextSegmentIterator index(Core::TextFragment(request.Path, _skipURL, request.Path.length() - _skipURL), false, '/');

         // Skip the first item, which will be a '/'
         index.Next();

         result->ErrorCode = Web::STATUS_BAD_REQUEST;
         result->Message = "Unknown error";

         if (request.Verb == Web::Request::HTTP_POST)
         {
            result = PostMethod(index, request);
         }
         else if (request.Verb == Web::Request::HTTP_PUT)
         {
            result = PutMethod(index, request);
         }
         else if (request.Verb == Web::Request::HTTP_GET)
         {
            result = GetMethod(index);
         }

         return result;
      }

      Core::ProxyType<Web::Response> DTV::GetMethod(Core::TextSegmentIterator& index)
      {
         Core::ProxyType<Web::Response> result(PluginHost::Factories::Instance().Response());

         result->ErrorCode = Web::STATUS_BAD_REQUEST;
         result->Message = _T("Unsupported GET request.");

         // The next item should be the request
         if (index.Next())
         {
            SYSLOG(Logging::Notification, (_T("DTV::GetMethod: request=%s"), index.Current().Text().c_str()));

            if (index.Current() == _T("NumberOfCountries"))
            {
               Core::ProxyType<Web::JSONBodyType<DTV::Data>> response(jsonResponseDataFactory.Element());

               if (GetNumberOfCountries(response->NumberOfCountries) == Core::ERROR_NONE)
               {
                  result->ErrorCode = Web::STATUS_OK;
                  result->Body(response);
               }
            }
            else if (index.Current() == _T("CountryList"))
            {
               Core::ProxyType<Web::JSONBodyType<DTV::Data>> response(jsonResponseDataFactory.Element());

               if (GetCountryList(response->CountryList) == Core::ERROR_NONE)
               {
                  result->ErrorCode = Web::STATUS_OK;
                  result->Body(response);
               }
            }
            else if (index.Current() == _T("Country"))
            {
               Core::ProxyType<Web::JSONBodyType<DTV::Data>> response(jsonResponseDataFactory.Element());

               if (GetCountry(response->CountryCode) == Core::ERROR_NONE)
               {
                  result->ErrorCode = Web::STATUS_OK;
                  result->Body(response);
               }
            }
            else if (index.Current() == _T("NumberOfServices"))
            {
               Core::ProxyType<Web::JSONBodyType<DTV::Data>> response(jsonResponseDataFactory.Element());

               if (GetNumberOfServices(response->NumberOfServices) == Core::ERROR_NONE)
               {
                  result->ErrorCode = Web::STATUS_OK;
                  result->Body(response);
               }
            }
            else if (index.Current() == _T("ServiceList"))
            {
               Core::ProxyType<Web::JSONBodyType<DTV::Data>> response(jsonResponseDataFactory.Element());
               string tuner_type;

               /* If no tuner type is given, or is defined as 'none', then all services should be returned */
               if (index.Next())
               {
                  tuner_type = index.Current().Text();
               }
               else
               {
                  tuner_type = _T("none");
               }

               if (GetServiceList(tuner_type, response->ServiceList) == Core::ERROR_NONE)
               {
                  result->ErrorCode = Web::STATUS_OK;
                  result->Body(response);
               }
            }
            else
            {
               SYSLOG(Logging::Notification, (_T("DTV::GetMethod: not supported")));
            }

            if (result->ErrorCode == Web::STATUS_OK)
            {
               result->Message = _T(index.Current().Text().c_str());
            }
         }
         else
         {
            SYSLOG(Logging::Notification, (_T("DTV::GetMethod: no method found")));
         }

         return result;
      }

      Core::ProxyType<Web::Response> DTV::PutMethod(Core::TextSegmentIterator& index, const Web::Request& request)
      {
         Core::ProxyType<Web::Response> result(PluginHost::Factories::Instance().Response());

         result->ErrorCode = Web::STATUS_BAD_REQUEST;
         result->Message = _T("Unsupported PUT request.");

         // The next item should be the request
         if (index.Next())
         {
            SYSLOG(Logging::Notification, (_T("DTV::PutMethod: request=%s, args=%s"), index.Current().Text().c_str(),
               index.Remainder().Text().c_str()));

            if (index.Current() == _T("Country"))
            {
               /* URL to set the country config is of the form ../DTV/Country/<country_code>
                * where <country_code> is a decimal value */
               if (index.Next())
               {
                  Core::JSON::DecUInt32 code = Core::NumberType<Core::JSON::DecUInt32>(index.Current());

                  if (SetCountry(code) == Core::ERROR_NONE)
                  {
                     result->ErrorCode = Web::STATUS_OK;
                  }
               }
               else
               {
                  SYSLOG(Logging::Notification, (_T("DTV::PutMethod: No code given for Country")));
               }
            }
            else if (index.Current() == _T("FinishServiceSearch"))
            {
               /* Finish the service search using ../DTV/FinishServiceSearch/<tunertype> */
               if (index.Next())
               {
                  FinishServiceSearchParamsData finish_search;
                  Core::JSON::Boolean response;

                  finish_search.Tunertype = Core::EnumerateType<TunertypeType>(index.Current()).Value();
                  finish_search.Savechanges = true;

                  if (FinishServiceSearch(finish_search, response) == Core::ERROR_NONE)
                  {
                     result->ErrorCode = Web::STATUS_OK;
                  }
               }
               else
               {
                  SYSLOG(Logging::Notification, (_T("DTV::PutMethod: No tuner type given for FinishServiceSearch")));
               }
            }
            else
            {
               SYSLOG(Logging::Notification, (_T("DTV::PutMethod: not supported")));
            }

            if (result->ErrorCode == Web::STATUS_OK)
            {
               result->Message = _T(index.Current().Text().c_str());
            }
         }
         else
         {
            SYSLOG(Logging::Notification, (_T("DTV::PutMethod: no method found")));
         }

         return result;
      }

      Core::ProxyType<Web::Response> DTV::PostMethod(Core::TextSegmentIterator& index, const Web::Request& request)
      {
         Core::ProxyType<Web::Response> result(PluginHost::Factories::Instance().Response());

         result->ErrorCode = Web::STATUS_BAD_REQUEST;
         result->Message = _T("Unsupported POST request.");

         // The next item should be the request
         if (index.Next())
         {
            SYSLOG(Logging::Notification, (_T("DTV::PostMethod: request=%s, args=%s"), index.Current().Text().c_str(),
               index.Remainder().Text().c_str()));

            if (index.Current() == _T("StartServiceSearch"))
            {
               /* Start a service search with the given tuner and search types using a URL of the
                * format: ../DTV/StartServiceSearch/<tunertype>/<searchtype> */
               if (index.Next())
               {
                  StartServiceSearchParamsData search_params;

                  search_params.Tunertype = Core::EnumerateType<TunertypeType>(index.Current()).Value();

                  if (index.Next())
                  {
                     Core::JSON::Boolean response;

                     search_params.Searchtype = Core::EnumerateType<StartServiceSearchParamsData::SearchtypeType>(index.Current()).Value();

                     search_params.Retune = true;

                     if (StartServiceSearch(search_params, response))
                     {
                        result->ErrorCode = Web::STATUS_OK;
                     }
                     else
                     {
                        SYSLOG(Logging::Notification, (_T("DTV::PostMethod: Failed to start service search")));
                        result->Message = _T("Failed to start service search");
                     }
                  }
                  else
                  {
                     SYSLOG(Logging::Notification, (_T("DTV::PostMethod: No search type given for StartServiceSearch")));
                     result->Message = _T("No search type given for StartServiceSearch");
                  }
               }
               else
               {
                  SYSLOG(Logging::Notification, (_T("DTV::PostMethod: No tuner type given for StartServiceSearch")));
                  result->Message = _T("No tuner type given for StartServiceSearch");
               }
            }
            else
            {
               SYSLOG(Logging::Notification, (_T("DTV::PostMethod: not supported")));
            }

            if (result->ErrorCode == Web::STATUS_OK)
            {
               result->Message = _T(index.Current().Text().c_str());
            }
         }
         else
         {
            SYSLOG(Logging::Notification, (_T("DTV::PostMethod: no method found")));
         }

         return result;
      }

      void DTV::Deactivated(RPC::IRemoteConnection *connection)
      {
         if (connection->Id() == _connectionId)
         {
            ASSERT(_service != nullptr);
            PluginHost::WorkerPool::Instance().Submit(PluginHost::IShell::Job::Create(_service,
               PluginHost::IShell::DEACTIVATED, PluginHost::IShell::FAILURE));
         }
      }

      void DTV::DvbEventHandler(U32BIT event, void *event_data, U32BIT data_size)
      {
         switch (event)
         {
            case STB_EVENT_SEARCH_FAIL:
            case STB_EVENT_SEARCH_SUCCESS:
            case UI_EVENT_UPDATE:
            {
               //STB_SPDebugWrite("DTV::DvbEventHandler: event=0x%08x\n", event);
               DTV::instance()->NotifySearchStatus();
               break;
            }

            case APP_EVENT_SERVICE_UPDATED:
            {
               DTV::instance()->NotifyService(EventtypeType::SERVICEUPDATED, _T("serviceupdated"), *(void **)event_data);
               break;
            }

            case APP_EVENT_SERVICE_ADDED:
            {
               DTV::instance()->NotifyService(EventtypeType::SERVICEADDED, _T("serviceadded"), *(void **)event_data);
               break;
            }

            case APP_EVENT_SERVICE_DELETED:
            {
               DTV::instance()->NotifyService(EventtypeType::SERVICEDELETED, _T("servicedeleted"), *(void **)event_data);
               break;
            }

            case APP_EVENT_SERVICE_VIDEO_CODEC_CHANGED:
            case APP_EVENT_SERVICE_VIDEO_PID_UPDATE:
            {
               DTV::instance()->NotifyService(EventtypeType::VIDEOCHANGED, _T("videochanged"), *(void **)event_data);
               break;
            }

            case APP_EVENT_SERVICE_AUDIO_CODEC_CHANGED:
            case APP_EVENT_SERVICE_AUDIO_PID_UPDATE:
            {
               DTV::instance()->NotifyService(EventtypeType::AUDIOCHANGED, _T("audiochanged"), *(void **)event_data);
               break;
            }

            case APP_EVENT_SERVICE_SUBTITLE_UPDATE:
            {
               DTV::instance()->NotifyService(EventtypeType::SUBTITLESCHANGED, _T("subtitleschanged"), *(void **)event_data);
               break;
            }

            case APP_EVENT_SERVICE_EIT_NOW_UPDATE:
            {
               /* Service, and hence event info, can only be provided if the service is available */
               if (event_data != NULL)
               {
                  DTV::instance()->NotifyEventChanged(*(void **)event_data);
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

      void DTV::NotifySearchStatus(void)
      {
         SearchstatusParamsData params;

         params.Eventtype = EventtypeType::SERVICESEARCHSTATUS;
         params.Handle = ACTL_GetServiceSearchPath();
         params.Finished = ACTL_IsSearchComplete() ? true : false;

         if (params.Finished)
         {
            params.Progress = 100;
         }
         else
         {
            params.Progress = ACTL_GetSearchProgress();
         }

         string message(_T("{\"eventtype\": \"ServiceSearchStatus\""));
         message += _T(", \"finished\": ");
         message += (params.Finished ? _T("true") : _T("false"));
         message += _T(", \"handle\": ") + std::to_string(params.Handle.Value());
         message += _T(", \"progress\": ") + std::to_string(params.Progress.Value());
         message += _T("}");

         _service->Notify(message);

         EventSearchStatus(params);
      }

      void DTV::NotifyService(EventtypeType event_type, const string& event_name, void *service)
      {
         ServiceupdatedParamsInfo params;

         params.Eventtype = event_type;
         ExtractDvbServiceInfo(params.Service, service);

         string message(_T("{\"eventtype\":\""));
         message += event_name;
         message += _T("\", \"service\":");
         message += CreateJsonForService(params.Service);
         message += _T("}");

         _service->Notify(message);

         EventService(event_name, params);
      }

      void DTV::NotifyEventChanged(void *service)
      {
         if (service != NULL)
         {
            void *now;

            ADB_GetNowNextEvents(service, &now, NULL);

            if (now != NULL)
            {
               EventchangedParamsData params;

               params.Eventtype = EventtypeType::EVENTCHANGED;
               ExtractDvbServiceInfo(params.Service, service);
               ExtractDvbEventInfo(params.Event, now);

               ADB_ReleaseEventData(now);

               string message(_T("{\"eventtype\":\"EventChanged\""));
               message += _T(", \"service\":");
               message += CreateJsonForService(params.Service);
               message += _T(", \"event\":");
               message += CreateJsonForEITEvent(params.Event);
               message += _T("}");

               _service->Notify(message);

               EventEventChanged(params);
            }
         }
      }

      string DTV::CreateJsonForService(ServiceInfo& service) const
      {
         string message(_T("{\"fullname\":\"") + service.Fullname.Value());
         message += _T("\", \"shortname\":\"") + service.Shortname.Value();
         message += _T("\", \"dvburi\":\"") + service.Dvburi.Value();
         message += _T("\", \"servicetype\":\"") + string(service.Servicetype.Data());
         message += _T("\", \"lcn\":") + std::to_string(service.Lcn.Value());
         message += _T(", \"scrambled\":");
         message += (service.Scrambled ? _T("true") : _T("false"));
         message += _T(", \"hascadescriptor\":");
         message += (service.Hascadescriptor ? _T("true") : _T("false"));
         message += _T(", \"hidden\":");
         message += (service.Hidden ? _T("true") : _T("false"));
         message += _T(", \"selectable\":");
         message += (service.Selectable ? _T("true") : _T("false"));
         message += _T(", \"runningstatus\":\"") + string(service.Runningstatus.Data());
         message += _T("\"}");

         return(message);
      }

      string DTV::CreateJsonForEITEvent(EiteventInfo& event) const
      {
         string message(_T("{\"name\":\"") + event.Name.Value());
         message += _T("\", \"starttime\":") + std::to_string(event.Starttime.Value());
         message += _T(", \"duration\":") + std::to_string(event.Duration.Value());
         message += _T(", \"eventid\":") + std::to_string(event.Eventid.Value());
         message += _T(", \"shortdescription\":\"") + CreateJsonString(event.Shortdescription.Value());
         message += _T("\", \"hassubtitles\":");
         message += (event.Hassubtitles ? _T("true") : _T("false"));
         message += _T(", \"hasaudiodescription\":");
         message += (event.Hasaudiodescription ? _T("true") : _T("false"));
         message += _T(", \"parentalrating\":") + std::to_string(event.Parentalrating.Value());

         message += _T(", \"contentdata\":[");

         uint16_t content_len = event.Contentdata.Length();
         if (content_len != 0)
         {
            message += Core::ToString(event.Contentdata[0]);

            for (uint16_t i = 1; i < content_len; i++)
            {
               message += _T(",") + Core::ToString(event.Contentdata[i]);
            }
         }

         message += _T("], \"hasextendedinfo\":");
         message += (event.Hasextendedinfo ? _T("true") : _T("false"));
         message += _T("}");

         return(message);
      }

      string DTV::CreateJsonString(const string& input_string) const
      {
         string output_string(input_string);
         size_t pos = 0;

         while ((pos = output_string.find("\"", pos)) != string::npos)
         {
            output_string = output_string.replace(pos, 1, "\\\"");
            pos += 2;
         }

         return(output_string);
      }
   }
}

