
#include "VoiceControl.h"
#include "libIBusDaemon.h"
#include <stdlib.h>
#include "UtilsJsonRpc.h"
#include "UtilsIarm.h"

#define API_VERSION_NUMBER_MAJOR 1
#define API_VERSION_NUMBER_MINOR 4
#define API_VERSION_NUMBER_PATCH 1

using namespace std;

namespace WPEFramework {

    namespace {

        static Plugin::Metadata<Plugin::VoiceControl> metadata(
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

    namespace Plugin {

        SERVICE_REGISTRATION(VoiceControl, API_VERSION_NUMBER_MAJOR, API_VERSION_NUMBER_MINOR, API_VERSION_NUMBER_PATCH);

        VoiceControl* VoiceControl::_instance = nullptr;

        VoiceControl::VoiceControl()
            : PluginHost::JSONRPC()
            , m_apiVersionNumber((uint32_t)-1)   /* default max uint32_t so everything gets enabled */
        {
            LOGINFO("ctor");
            VoiceControl::_instance = this;

            Register("getApiVersionNumber",          &VoiceControl::getApiVersionNumber,          this);

            Register("voiceStatus",                  &VoiceControl::voiceStatus,                  this);
            Register("configureVoice",               &VoiceControl::configureVoice,               this);
            Register("setVoiceInit",                 &VoiceControl::setVoiceInit,                 this);
            Register("sendVoiceMessage",             &VoiceControl::sendVoiceMessage,             this);
            Register("voiceSessionByText",           &VoiceControl::voiceSessionByText,           this);
            Register("voiceSessionTypes",            &VoiceControl::voiceSessionTypes,            this);
            Register("voiceSessionRequest",          &VoiceControl::voiceSessionRequest,          this);
            Register("voiceSessionTerminate",        &VoiceControl::voiceSessionTerminate,        this);
            Register("voiceSessionAudioStreamStart", &VoiceControl::voiceSessionAudioStreamStart, this);

            setApiVersionNumber(1);
            m_hasOwnProcess = false;
            m_maskPii = false;
        }

        VoiceControl::~VoiceControl()
        {
            //LOGINFO("dtor");
        }

        const string VoiceControl::Initialize(PluginHost::IShell*  /* service */)
        {
            InitializeIARM();
            getMaskPii_();
            // On success return empty, to indicate there is no error text.
            return (string());
        }

        void VoiceControl::Deinitialize(PluginHost::IShell* /* service */)
        {
            LOGINFO("Deinitialize");
            DeinitializeIARM();
            VoiceControl::_instance = nullptr;
        }

        void VoiceControl::InitializeIARM()
        {
            if (Utils::IARM::init())
            {
                // We have our own Linux process, so we need to connect and disconnect from the IARM Bus
                m_hasOwnProcess = true;

                IARM_Result_t res;
                IARM_CHECK( IARM_Bus_RegisterEventHandler(CTRLM_MAIN_IARM_BUS_NAME, CTRLM_VOICE_IARM_EVENT_JSON_SESSION_BEGIN,        voiceEventHandler) );
                IARM_CHECK( IARM_Bus_RegisterEventHandler(CTRLM_MAIN_IARM_BUS_NAME, CTRLM_VOICE_IARM_EVENT_JSON_STREAM_BEGIN,         voiceEventHandler) );
                IARM_CHECK( IARM_Bus_RegisterEventHandler(CTRLM_MAIN_IARM_BUS_NAME, CTRLM_VOICE_IARM_EVENT_JSON_KEYWORD_VERIFICATION, voiceEventHandler) );
                IARM_CHECK( IARM_Bus_RegisterEventHandler(CTRLM_MAIN_IARM_BUS_NAME, CTRLM_VOICE_IARM_EVENT_JSON_SERVER_MESSAGE,       voiceEventHandler) );
                IARM_CHECK( IARM_Bus_RegisterEventHandler(CTRLM_MAIN_IARM_BUS_NAME, CTRLM_VOICE_IARM_EVENT_JSON_STREAM_END,           voiceEventHandler) );
                IARM_CHECK( IARM_Bus_RegisterEventHandler(CTRLM_MAIN_IARM_BUS_NAME, CTRLM_VOICE_IARM_EVENT_JSON_SESSION_END,          voiceEventHandler) );
            }
            else
                m_hasOwnProcess = false;
        }

        //TODO(MROLLINS) - we need to install crash handler to ensure DeinitializeIARM gets called
        void VoiceControl::DeinitializeIARM()
        {
            if (m_hasOwnProcess)
            {
                IARM_Result_t res;
                IARM_CHECK( IARM_Bus_RemoveEventHandler(CTRLM_MAIN_IARM_BUS_NAME, CTRLM_VOICE_IARM_EVENT_JSON_SESSION_END,          voiceEventHandler) );
                IARM_CHECK( IARM_Bus_RemoveEventHandler(CTRLM_MAIN_IARM_BUS_NAME, CTRLM_VOICE_IARM_EVENT_JSON_STREAM_END,           voiceEventHandler) );
                IARM_CHECK( IARM_Bus_RemoveEventHandler(CTRLM_MAIN_IARM_BUS_NAME, CTRLM_VOICE_IARM_EVENT_JSON_KEYWORD_VERIFICATION, voiceEventHandler) );
                IARM_CHECK( IARM_Bus_RemoveEventHandler(CTRLM_MAIN_IARM_BUS_NAME, CTRLM_VOICE_IARM_EVENT_JSON_SERVER_MESSAGE,       voiceEventHandler) );
                IARM_CHECK( IARM_Bus_RemoveEventHandler(CTRLM_MAIN_IARM_BUS_NAME, CTRLM_VOICE_IARM_EVENT_JSON_STREAM_BEGIN,         voiceEventHandler) );
                IARM_CHECK( IARM_Bus_RemoveEventHandler(CTRLM_MAIN_IARM_BUS_NAME, CTRLM_VOICE_IARM_EVENT_JSON_SESSION_BEGIN,        voiceEventHandler) );

                m_hasOwnProcess = false;
            }
        }

        void VoiceControl::voiceEventHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len)
        {
            if (VoiceControl::_instance)
                VoiceControl::_instance->iarmEventHandler(owner, eventId, data, len);
            else
                LOGWARN("WARNING - cannot handle IARM events without a VoiceControl plugin instance!");
        }

        void VoiceControl::iarmEventHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len)
        {
            LOGINFO("Event ID %u received, data: %p, len: %u.", (unsigned)eventId, data, (unsigned)len);
            if (!strcmp(owner, CTRLM_MAIN_IARM_BUS_NAME))
            {
                ctrlm_voice_iarm_event_json_t* eventData = (ctrlm_voice_iarm_event_json_t*)data;

                if ((data == NULL) || (len <= sizeof(ctrlm_voice_iarm_event_json_t)))
                {
                    LOGERR("ERROR - got eventId(%u) with INVALID DATA: data: %p, len: %zu.", (unsigned)eventId, data, len);
                    return;
                }

                // Ensure there is a null character at the end of the data area.
                char* str = (char*)data;
                str[len - 1] = '\0';

                if (CTRLM_VOICE_IARM_BUS_API_REVISION != eventData->api_revision)
                {
                    LOGERR("ERROR - got eventId(%u) with wrong VOICE IARM API revision - should be %d, event has %d.",
                           (unsigned)eventId, CTRLM_VOICE_IARM_BUS_API_REVISION, (int)eventData->api_revision);
                    return;
                }

                switch(eventId) {
                    case CTRLM_VOICE_IARM_EVENT_JSON_SESSION_BEGIN:
                        LOGWARN("Got CTRLM_VOICE_IARM_EVENT_JSON_SESSION_BEGIN event.");

                        onSessionBegin(eventData);
                        break;

                    case CTRLM_VOICE_IARM_EVENT_JSON_STREAM_BEGIN:
                        LOGWARN("Got CTRLM_VOICE_IARM_EVENT_JSON_STREAM_BEGIN event.");

                        onStreamBegin(eventData);
                        break;

                    case CTRLM_VOICE_IARM_EVENT_JSON_KEYWORD_VERIFICATION:
                        LOGWARN("Got CTRLM_VOICE_IARM_EVENT_JSON_KEYWORD_VERIFICATION event.");

                        onKeywordVerification(eventData);
                        break;

                    case CTRLM_VOICE_IARM_EVENT_JSON_SERVER_MESSAGE:
                        LOGWARN("Got CTRLM_VOICE_IARM_EVENT_JSON_SERVER_MESSAGE event.");

                        onServerMessage(eventData);
                        break;

                    case CTRLM_VOICE_IARM_EVENT_JSON_STREAM_END:
                        LOGWARN("Got CTRLM_VOICE_IARM_EVENT_JSON_STREAM_END event.");

                        onStreamEnd(eventData);
                        break;

                    case CTRLM_VOICE_IARM_EVENT_JSON_SESSION_END:
                        LOGWARN("Got CTRLM_VOICE_IARM_EVENT_JSON_SESSION_END event.");

                        onSessionEnd(eventData);
                        break;

                    default:
                        LOGERR("ERROR - unexpected ControlMgr event: eventId: %u, data: %p, size: %zu.",
                               (unsigned)eventId, data, len);
                        break;
                }
            }
            else
            {
                LOGERR("ERROR - unexpected event: owner %s, eventId: %u, data: %p, size: %zu.",
                       owner, (unsigned)eventId, data, len);
            }
        }  // End iarmEventHandler()

        //Begin methods
        uint32_t VoiceControl::getApiVersionNumber(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            response["version"] = m_apiVersionNumber;
            returnResponse(true);
        }

        void VoiceControl::getMaskPii_()
        {
            JsonObject params;
            JsonObject result;
            voiceStatus(params, result);
            m_maskPii = result["maskPii"].Boolean();
            LOGINFO("Mask pii set to %s.", (m_maskPii ? "True" : "False"));
        }

        uint32_t VoiceControl::voiceStatus(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();

            ctrlm_voice_iarm_call_json_t*   call = NULL;
            IARM_Result_t                   res;
            string                          jsonParams;
            bool                            bSuccess = true;

            // Just pass through the input parameters, without understanding or checking them.
            parameters.ToString(jsonParams);

            // We must allocate the memory for the call structure. Determine what we will need.
            size_t totalsize = sizeof(ctrlm_voice_iarm_call_json_t) + jsonParams.size() + 1;
            call = (ctrlm_voice_iarm_call_json_t*)calloc(1, totalsize);

            if (call != NULL)
            {
                // Set the call structure members appropriately.
                call->api_revision = CTRLM_VOICE_IARM_BUS_API_REVISION;
                size_t len = jsonParams.copy(call->payload, jsonParams.size());
                call->payload[len] = '\0';
            }
            else
            {
                LOGERR("ERROR - Cannot allocate IARM structure - size: %u.", (unsigned)totalsize);
                bSuccess = false;
            }

            if (bSuccess)
            {
                // Make the IARM call to controlMgr to configure the voice settings
                res = IARM_Bus_Call(CTRLM_MAIN_IARM_BUS_NAME, CTRLM_VOICE_IARM_CALL_STATUS, (void *)call, totalsize);
                if (res != IARM_RESULT_SUCCESS)
                {
                    LOGERR("ERROR - CTRLM_VOICE_IARM_CALL_STATUS Bus Call FAILED, res: %d.", (int)res);
                    bSuccess = false;
                }
                else
                {
                    JsonObject result;

                    result.FromString(call->result);
                    bSuccess = result["success"].Boolean();
                    response = result;
                    if(bSuccess) {
                        LOGINFO("CTRLM_VOICE_IARM_CALL_STATUS call SUCCESS!");
                    } else {
                        LOGERR("ERROR - CTRLM_VOICE_IARM_CALL_STATUS returned FAILURE!");
                    }
                }
            }

            if (call != NULL)
            {
                free(call);
            }

            returnResponse(bSuccess);
        }

        uint32_t VoiceControl::configureVoice(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();

            ctrlm_voice_iarm_call_json_t*   call = NULL;
            IARM_Result_t                   res;
            string                          jsonParams;
            bool                            bSuccess = true;

            // Just pass through the input parameters, without understanding or checking them.
            parameters.ToString(jsonParams);

            // We must allocate the memory for the call structure. Determine what we will need.
            size_t totalsize = sizeof(ctrlm_voice_iarm_call_json_t) + jsonParams.size() + 1;
            call = (ctrlm_voice_iarm_call_json_t*)calloc(1, totalsize);

            if (call != NULL)
            {
                // Set the call structure members appropriately.
                call->api_revision = CTRLM_VOICE_IARM_BUS_API_REVISION;
                size_t len = jsonParams.copy(call->payload, jsonParams.size());
                call->payload[len] = '\0';
            }
            else
            {
                LOGERR("ERROR - Cannot allocate IARM structure - size: %u.", (unsigned)totalsize);
                bSuccess = false;
            }

            if (bSuccess)
            {
                // Make the IARM call to controlMgr to configure the voice settings
                res = IARM_Bus_Call(CTRLM_MAIN_IARM_BUS_NAME, CTRLM_VOICE_IARM_CALL_CONFIGURE_VOICE, (void *)call, totalsize);
                if (res != IARM_RESULT_SUCCESS)
                {
                    LOGERR("ERROR - CTRLM_VOICE_IARM_CALL_CONFIGURE_VOICE Bus Call FAILED, res: %d.", (int)res);
                    bSuccess = false;
                }
                else
                {
                    JsonObject result;

                    result.FromString(call->result);
                    bSuccess = result["success"].Boolean();
                    response = result;
                    if(bSuccess) {
                        LOGINFO("CONFIGURE_VOICE call SUCCESS!");
                    } else {
                        LOGERR("ERROR - CTRLM_VOICE_IARM_CALL_CONFIGURE_VOICE returned FAILURE!");
                    }
                }
            }

            if (call != NULL)
            {
                free(call);
            }

            returnResponse(bSuccess);
        }

        uint32_t VoiceControl::setVoiceInit(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();

            ctrlm_voice_iarm_call_json_t*   call = NULL;
            IARM_Result_t                   res;
            string                          jsonParams;
            bool                            bSuccess = true;

            // Just pass through the input parameters, without understanding or checking them.
            parameters.ToString(jsonParams);

            // We must allocate the memory for the call structure. Determine what we will need.
            size_t totalsize = sizeof(ctrlm_voice_iarm_call_json_t) + jsonParams.size() + 1;
            call = (ctrlm_voice_iarm_call_json_t*)calloc(1, totalsize);

            if (call != NULL)
            {
                // Set the call structure members appropriately.
                call->api_revision = CTRLM_VOICE_IARM_BUS_API_REVISION;
                size_t len = jsonParams.copy(call->payload, jsonParams.size());
                call->payload[len] = '\0';
            }
            else
            {
                LOGERR("ERROR - Cannot allocate IARM structure - size: %u.", (unsigned)totalsize);
                bSuccess = false;
            }

            if (bSuccess)
            {
                // Make the IARM call to controlMgr to configure the voice settings
                res = IARM_Bus_Call(CTRLM_MAIN_IARM_BUS_NAME, CTRLM_VOICE_IARM_CALL_SET_VOICE_INIT, (void *)call, totalsize);
                if (res != IARM_RESULT_SUCCESS)
                {
                    LOGERR("ERROR - CTRLM_VOICE_IARM_CALL_SET_VOICE_INIT Bus Call FAILED, res: %d.", (int)res);
                    bSuccess = false;
                }
                else
                {
                    JsonObject result;

                    result.FromString(call->result);
                    bSuccess = result["success"].Boolean();
                    response = result;
                    if(bSuccess) {
                        LOGINFO("SET_VOICE_INIT call SUCCESS!");
                    } else {
                        LOGERR("ERROR - CTRLM_VOICE_IARM_CALL_SET_VOICE_INIT returned FAILURE!");
                    }
                }
            }

            if (call != NULL)
            {
                free(call);
            }

            returnResponse(bSuccess);
        }


        uint32_t VoiceControl::sendVoiceMessage(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();

            ctrlm_voice_iarm_call_json_t*   call = NULL;
            IARM_Result_t                   res;
            string                          jsonParams;
            bool                            bSuccess = true;

            // Just pass through the input parameters, without understanding or checking them.
            parameters.ToString(jsonParams);

            // We must allocate the memory for the call structure. Determine what we will need.
            size_t totalsize = sizeof(ctrlm_voice_iarm_call_json_t) + jsonParams.size() + 1;
            call = (ctrlm_voice_iarm_call_json_t*)calloc(1, totalsize);

            if (call != NULL)
            {
                // Set the call structure members appropriately.
                call->api_revision = CTRLM_VOICE_IARM_BUS_API_REVISION;
                size_t len = jsonParams.copy(call->payload, jsonParams.size());
                call->payload[len] = '\0';
            }
            else
            {
                LOGERR("ERROR - Cannot allocate IARM structure - size: %u.", (unsigned)totalsize);
                bSuccess = false;
            }

            if (bSuccess)
            {
                // Make the IARM call to controlMgr to configure the voice settings
                res = IARM_Bus_Call(CTRLM_MAIN_IARM_BUS_NAME, CTRLM_VOICE_IARM_CALL_SEND_VOICE_MESSAGE, (void *)call, totalsize);
                if (res != IARM_RESULT_SUCCESS)
                {
                    LOGERR("ERROR - CTRLM_VOICE_IARM_CALL_SEND_VOICE_MESSAGE Bus Call FAILED, res: %d.", (int)res);
                    bSuccess = false;
                }
                else
                {
                    JsonObject result;

                    result.FromString(call->result);
                    bSuccess = result["success"].Boolean();
                    response = result;
                    if(bSuccess) {
                        LOGINFO("SEND_VOICE_MESSAGE call SUCCESS!");
                    } else {
                        LOGERR("ERROR - CTRLM_VOICE_IARM_CALL_SEND_VOICE_MESSAGE returned FAILURE!");
                    }
                }
            }

            if (call != NULL)
            {
                free(call);
            }

            returnResponse(bSuccess);
        }

        uint32_t VoiceControl::voiceSessionByText(const JsonObject& parameters, JsonObject& response) // DEPRECATED
        {
           // Translate the input parameters then call voiceSessionRequest
           JsonObject parameters_translated;

           if(!parameters.HasLabel("type")) {
              parameters_translated["type"] = "ptt_transcription";
           } else {
              std::string str_type = parameters["type"].String();
              transform(str_type.begin(), str_type.end(), str_type.begin(), ::tolower);
              if(str_type == "ptt") {
                 parameters_translated["type"] = "ptt_transcription";
              } else if(str_type == "ff") {
                 parameters_translated["type"] = "ff_transcription";
              } else if(str_type == "mic") {
                 parameters_translated["type"] = "mic_transcription";
              } else {
                 parameters_translated["type"] = "";
              }
           }
           if(parameters.HasLabel("transcription")) {
              parameters_translated["transcription"] = parameters["transcription"];
           } // else voiceSessionRequest will return an error if transcription field is not present

           return(voiceSessionRequest(parameters_translated, response));
        }

        uint32_t VoiceControl::voiceSessionTypes(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();

            ctrlm_voice_iarm_call_json_t*   call = NULL;
            IARM_Result_t                   res;
            string                          jsonParams;
            bool                            bSuccess = true;

            // Just pass through the input parameters, without understanding or checking them.
            parameters.ToString(jsonParams);

            // We must allocate the memory for the call structure. Determine what we will need.
            size_t totalsize = sizeof(ctrlm_voice_iarm_call_json_t) + jsonParams.size() + 1;
            call = (ctrlm_voice_iarm_call_json_t*)calloc(1, totalsize);

            if (call != NULL)
            {
                // Set the call structure members appropriately.
                call->api_revision = CTRLM_VOICE_IARM_BUS_API_REVISION;
                size_t len = jsonParams.copy(call->payload, jsonParams.size());
                call->payload[len] = '\0';
            }
            else
            {
                LOGERR("ERROR - Cannot allocate IARM structure - size: %u.", (unsigned)totalsize);
                bSuccess = false;
            }

            if (bSuccess)
            {
                // Make the IARM call to controlMgr to configure the voice settings
                res = IARM_Bus_Call(CTRLM_MAIN_IARM_BUS_NAME, CTRLM_VOICE_IARM_CALL_SESSION_TYPES, (void *)call, totalsize);
                if (res != IARM_RESULT_SUCCESS)
                {
                    LOGERR("ERROR - CTRLM_VOICE_IARM_CALL_SESSION_TYPES Bus Call FAILED, res: %d.", (int)res);
                    bSuccess = false;
                }
                else
                {
                    JsonObject result;

                    result.FromString(call->result);
                    bSuccess = result["success"].Boolean();
                    response = result;
                    if(bSuccess) {
                        LOGINFO("SESSION_TYPES call SUCCESS!");
                    } else {
                        LOGERR("ERROR - CTRLM_VOICE_IARM_CALL_SESSION_TYPES returned FAILURE!");
                    }
                }
            }

            if (call != NULL)
            {
                free(call);
            }

            returnResponse(bSuccess);
        }

        uint32_t VoiceControl::voiceSessionRequest(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();

            ctrlm_voice_iarm_call_json_t*   call = NULL;
            IARM_Result_t                   res;
            string                          jsonParams;
            bool                            bSuccess = true;

            // Just pass through the input parameters, without understanding or checking them.
            parameters.ToString(jsonParams);

            // We must allocate the memory for the call structure. Determine what we will need.
            size_t totalsize = sizeof(ctrlm_voice_iarm_call_json_t) + jsonParams.size() + 1;
            call = (ctrlm_voice_iarm_call_json_t*)calloc(1, totalsize);

            if (call != NULL)
            {
                // Set the call structure members appropriately.
                call->api_revision = CTRLM_VOICE_IARM_BUS_API_REVISION;
                size_t len = jsonParams.copy(call->payload, jsonParams.size());
                call->payload[len] = '\0';
            }
            else
            {
                LOGERR("ERROR - Cannot allocate IARM structure - size: %u.", (unsigned)totalsize);
                bSuccess = false;
            }

            if (bSuccess)
            {
                // Make the IARM call to controlMgr to configure the voice settings
                res = IARM_Bus_Call(CTRLM_MAIN_IARM_BUS_NAME, CTRLM_VOICE_IARM_CALL_SESSION_REQUEST, (void *)call, totalsize);
                if (res != IARM_RESULT_SUCCESS)
                {
                    LOGERR("ERROR - CTRLM_VOICE_IARM_CALL_SESSION_REQUEST Bus Call FAILED, res: %d.", (int)res);
                    bSuccess = false;
                }
                else
                {
                    JsonObject result;

                    result.FromString(call->result);
                    bSuccess = result["success"].Boolean();
                    response = result;
                    if(bSuccess) {
                        LOGINFO("SESSION_REQUEST call SUCCESS!");
                    } else {
                        LOGERR("ERROR - CTRLM_VOICE_IARM_CALL_SESSION_REQUEST returned FAILURE!");
                    }
                }
            }

            if (call != NULL)
            {
                free(call);
            }

            returnResponse(bSuccess);
        }

        uint32_t VoiceControl::voiceSessionTerminate(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();

            ctrlm_voice_iarm_call_json_t*   call = NULL;
            IARM_Result_t                   res;
            string                          jsonParams;
            bool                            bSuccess = true;

            // Just pass through the input parameters, without understanding or checking them.
            parameters.ToString(jsonParams);

            // We must allocate the memory for the call structure. Determine what we will need.
            size_t totalsize = sizeof(ctrlm_voice_iarm_call_json_t) + jsonParams.size() + 1;
            call = (ctrlm_voice_iarm_call_json_t*)calloc(1, totalsize);

            if (call != NULL)
            {
                // Set the call structure members appropriately.
                call->api_revision = CTRLM_VOICE_IARM_BUS_API_REVISION;
                size_t len = jsonParams.copy(call->payload, jsonParams.size());
                call->payload[len] = '\0';
            }
            else
            {
                LOGERR("ERROR - Cannot allocate IARM structure - size: %u.", (unsigned)totalsize);
                bSuccess = false;
            }

            if (bSuccess)
            {
                // Make the IARM call to controlMgr to configure the voice settings
                res = IARM_Bus_Call(CTRLM_MAIN_IARM_BUS_NAME, CTRLM_VOICE_IARM_CALL_SESSION_TERMINATE, (void *)call, totalsize);
                if (res != IARM_RESULT_SUCCESS)
                {
                    LOGERR("ERROR - CTRLM_VOICE_IARM_CALL_SESSION_TERMINATE Bus Call FAILED, res: %d.", (int)res);
                    bSuccess = false;
                }
                else
                {
                    JsonObject result;

                    result.FromString(call->result);
                    bSuccess = result["success"].Boolean();
                    response = result;
                    if(bSuccess) {
                        LOGINFO("SESSION_TERMINATE call SUCCESS!");
                    } else {
                        LOGERR("ERROR - CTRLM_VOICE_IARM_CALL_SESSION_TERMINATE returned FAILURE!");
                    }
                }
            }

            if (call != NULL)
            {
                free(call);
            }

            returnResponse(bSuccess);
        }

        uint32_t VoiceControl::voiceSessionAudioStreamStart(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();

            ctrlm_voice_iarm_call_json_t*   call = NULL;
            IARM_Result_t                   res;
            string                          jsonParams;
            bool                            bSuccess = true;

            // Just pass through the input parameters, without understanding or checking them.
            parameters.ToString(jsonParams);

            // We must allocate the memory for the call structure. Determine what we will need.
            size_t totalsize = sizeof(ctrlm_voice_iarm_call_json_t) + jsonParams.size() + 1;
            call = (ctrlm_voice_iarm_call_json_t*)calloc(1, totalsize);

            if (call != NULL)
            {
                // Set the call structure members appropriately.
                call->api_revision = CTRLM_VOICE_IARM_BUS_API_REVISION;
                size_t len = jsonParams.copy(call->payload, jsonParams.size());
                call->payload[len] = '\0';
            }
            else
            {
                LOGERR("ERROR - Cannot allocate IARM structure - size: %u.", (unsigned)totalsize);
                bSuccess = false;
            }

            if (bSuccess)
            {
                // Make the IARM call to controlMgr to start the audio stream
                res = IARM_Bus_Call(CTRLM_MAIN_IARM_BUS_NAME, CTRLM_VOICE_IARM_CALL_SESSION_AUDIO_STREAM_START, (void *)call, totalsize);
                if (res != IARM_RESULT_SUCCESS)
                {
                    LOGERR("ERROR - CTRLM_VOICE_IARM_CALL_SESSION_AUDIO_STREAM_START Bus Call FAILED, res: %d.", (int)res);
                    bSuccess = false;
                }
                else
                {
                    JsonObject result;

                    result.FromString(call->result);
                    bSuccess = result["success"].Boolean();
                    response = result;
                    if(bSuccess) {
                        LOGINFO("SESSION_AUDIO_STREAM_START call SUCCESS!");
                    } else {
                        LOGERR("ERROR - CTRLM_VOICE_IARM_CALL_SESSION_AUDIO_STREAM_START returned FAILURE!");
                    }
                }
            }

            if (call != NULL)
            {
                free(call);
            }

            returnResponse(bSuccess);
        }
        //End methods

        //Begin events
        void VoiceControl::onSessionBegin(ctrlm_voice_iarm_event_json_t* eventData)
        {
            JsonObject params;

            params.FromString(eventData->payload);

            sendNotify("onSessionBegin", params);
        }

        void VoiceControl::onStreamBegin(ctrlm_voice_iarm_event_json_t* eventData)
        {
            JsonObject params;

            params.FromString(eventData->payload);

            sendNotify("onStreamBegin", params);
        }

        void VoiceControl::onKeywordVerification(ctrlm_voice_iarm_event_json_t* eventData)
        {
            JsonObject params;

            params.FromString(eventData->payload);

            sendNotify("onKeywordVerification", params);
        }

        void VoiceControl::onServerMessage(ctrlm_voice_iarm_event_json_t* eventData)
        {
            JsonObject params;

            params.FromString(eventData->payload);

            sendNotify_("onServerMessage", params);
        }

        void VoiceControl::onStreamEnd(ctrlm_voice_iarm_event_json_t* eventData)
        {
            JsonObject params;

            params.FromString(eventData->payload);

            sendNotify("onStreamEnd", params);
        }

        void VoiceControl::onSessionEnd(ctrlm_voice_iarm_event_json_t* eventData)
        {
            JsonObject params;

            params.FromString(eventData->payload);

            sendNotify_("onSessionEnd", params);
        }
        //End events

        //Begin local private utility methods
        void VoiceControl::setApiVersionNumber(unsigned int apiVersionNumber)
        {
            LOGINFO("setting version: %d", (int)apiVersionNumber);
            m_apiVersionNumber = apiVersionNumber;
        }

        void VoiceControl::sendNotify_(const char* eventName, JsonObject& parameters)
        {
            if(m_maskPii)
            {
                sendNotifyMaskParameters(eventName, parameters);
            }
            else
            {
                sendNotify(eventName, parameters);
            }
        }
        //End local private utility methods

    } // namespace Plugin
} // namespace WPEFramework

