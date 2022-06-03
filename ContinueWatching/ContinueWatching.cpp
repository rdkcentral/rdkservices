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

 /**
 * @file ContinueWatching.cpp
 * @brief Thunder Plugin based Implementation for ContinueWatching service API's (RDK-25833).
 */

/**
  @mainpage ContinueWatching

  <b>ContinueWatching</b> The Continue Watching Service will provide
  *  a method for applications on the STB to store a token for
  * retrieval by XRE.Applications like netflix will use this to store a token
  * that XRE will retrieve and use the data to gather data from the
  * OTT provider that can be used to populate a continue watching panel in the UI.
  * This service will be enabled/disabled using an TR181 parameter.
  */
#include <iomanip>

#include "ContinueWatching.h"

#if !defined(DISABLE_SECAPI)
#include "sec_security.h"
#include "sec_security_comcastids.h"
#endif
#include "rfcapi.h"     // for TR181 queries.
#include "cJSON.h"
#include "openssl/sha.h"
#include "base64.h"
#include "UtilsJsonRpc.h"
#define SEC_OBJECTID_CW_NETFLIX_STORAGE_KEY     0x0361000003610001ULL
#define SEC_OBJECTID_CW_YOUTUBE_STORAGE_KEY     0x0361000003610002ULL
#define SEC_OBJECTID_CW_HLU_STORAGE_KEY         0x0361000003610003ULL

#define CW_TR181_PARAMETER                      "Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.OTT_Token.Enable"
#define CW_ENV_PARAMETER                        "ENABLE_OTT_TOKEN"

#define CONTINUEWATCHING_MAJOR_VERSION 1
#define CONTINUEWATCHING_MINOR_VERSION 0


namespace WPEFramework {

	namespace Plugin {

	/*
	 *Register ContinueWatching module as wpeframework plugin
	 **/
        SERVICE_REGISTRATION(ContinueWatching, CONTINUEWATCHING_MAJOR_VERSION, CONTINUEWATCHING_MINOR_VERSION);

        ContinueWatching* ContinueWatching::_instance = nullptr;

		ContinueWatching::ContinueWatching()
		: PluginHost::JSONRPC()
		, m_apiVersionNumber((uint32_t)-1)
		{
			ContinueWatching::_instance = this;
			//Register all the APIs
			Register("getApplicationToken", &ContinueWatching::getApplicationToken, this);
			Register("setApplicationToken", &ContinueWatching::setApplicationToken, this);
			Register("deleteApplicationToken", &ContinueWatching::deleteApplicationToken, this);
			setApiVersionNumber(1);
		}

		ContinueWatching::~ContinueWatching()
		{
		}

		void ContinueWatching::Deinitialize(PluginHost::IShell* /* service */)
		{
			ContinueWatching::_instance = nullptr;
		}

		/**
		 *   @brief API to retrieve Application Token
		 *
		 *   @param[in]  applicationName - Name of the Application
		 *   @param[out] application_token - Token value of the Application
		 *   @return Core::ERROR_NONE
		 *
		 *Request: {"jsonrpc":"2.0","id":"3","method":"org.rdk.ContinueWatching.1.getApplicationToken","params":{ "applicationName":"netflix"}}
		 *Response success: {"jsonrpc":"2.0","id":3,"result":{"application_token":[{<token value>}],"success":true}}
		 *Response Failure: {"jsonrpc":"2.0","id":3,"result":{"application_token":[{}],"success":false}}
		 */
		uint32_t ContinueWatching::getApplicationToken(const JsonObject& parameters, JsonObject& response)
		{
            LOGINFOMETHOD();

			JsonObject token;
			JsonArray appToken;
			Core::OptionalType<Core::JSON::Error> error;
			std::lock_guard<std::mutex> lock(m_mutex);
			string appName = parameters["applicationName"].String();
			LOGINFO("appName:: %s  \n",appName.c_str());
			bool ret=false;
			string tokenData = getAppToken(appName.c_str());
			if(!(tokenData.empty())){
				if (!token.FromString(tokenData, error)) {
        	        		LOGERR("getApplicationToken :Failed to parse tokendata");      
				}
				else{
					ret=true;
					LOGINFO("getApplicationToken response OK  \n");
				}
			}
			else{
				LOGERR("getApplicationToken Error \n");
			}
			appToken.Add(JsonValue(token));
			response["application_token"]=appToken;
			returnResponse(ret);
		}

		/**
		 *   @brief API to set an Application Token
		 *
		 *   @param[in]  applicationName - Name of the Application
		 *   @param[in]  application_token - Token value of the Application
		 *   @return Core::ERROR_NONE
		 *
		 *Request: {"jsonrpc":"2.0","id":"3","method":"org.rdk.ContinueWatching.1.setApplicationToken","params": {"applicationName":<App name>,"application_token":{<token value>}}}
		 *Response success: {"jsonrpc":"2.0", "id":3, "result":{"success":true}}
		 *Response Failure: {"jsonrpc":"2.0","id":3,"result":{"success":false}}
		 */
		uint32_t ContinueWatching::setApplicationToken(const JsonObject& parameters, JsonObject& response)
		{
            LOGINFOMETHOD();
			std::lock_guard<std::mutex> lock(m_mutex);
			string appName = parameters["applicationName"].String();
			LOGINFO("appName %s  \n",appName.c_str());
			string tokenData = parameters["application_token"].String();
			LOGINFO("Token %s  \n",tokenData.c_str());
			bool result=setAppToken(appName.c_str(),tokenData.c_str());
			if(result){
				LOGINFO("setApplicationToken response OK  \n");
			}
			else{
				LOGERR("setApplicationToken response ERR\n");
			}
 			returnResponse(result);

		}

		/**
		 *   @brief API to delete an Application Token
		 *
		 *   @param[in]  applicationName - Name of the Application
		 *   @return Core::ERROR_NONE
		 *
		 *Request: {"jsonrpc":"2.0","id":"3","method":"org.rdk.ContinueWatching.1.deleteApplicationToken","params":{"applicationName":<app name>}}
		 *Response success: {"jsonrpc":"2.0", "id":3, "result":{"success":true}}
		 *Response Failure: {"jsonrpc":"2.0","id":3,"result":{"success":false}}
		 */
		uint32_t ContinueWatching::deleteApplicationToken(const JsonObject& parameters, JsonObject& response)
		{
            LOGINFOMETHOD();
			std::lock_guard<std::mutex> lock(m_mutex);
			string ApplicationName = parameters["applicationName"].String();
			bool result=deleteAppToken(ApplicationName);
			if(result){
				LOGINFO("deleteApplicationToken response OK  \n");
			}
			else{
				LOGERR("deleteApplicationToken response ERR\n");
			}
			returnResponse(result);
		}

		/**
		 * @brief This function is used to get the application token.
		 *
		 * @param[in] strApplicationName Application Name String.
		 *
		 * @return string
		 */
		std::string ContinueWatching::getAppToken(std::string strApplicationName)
		{
			try
			{
				ContinueWatchingImplFactory continueWatchingImplFactory;
				ContinueWatchingImpl *continueWatchingImpl = NULL;
				continueWatchingImpl = continueWatchingImplFactory.createContinueWatchingImpl(strApplicationName);
				if (!continueWatchingImpl)
				{
					LOGERR("Application name not matched. Return empty string \n");
					return "";
				}

				std::string tokenData = continueWatchingImpl->getApplicationToken();
				delete continueWatchingImpl;
				continueWatchingImpl = NULL;
				LOGINFO(" tokenData %s \n",tokenData.c_str());
				return tokenData;
			}
			catch (...) {
				LOGWARN("exception occurred during getAppToken \n");
				return "";
			}
		}

		/**
		 * @brief This function is used to set the application token.
		 *
		 * @param[in] strApplicationName Variable of application name String.
		 * @param[in] token Variable of token value string.
		 *
		* @return true.
		*/
		bool ContinueWatching::setAppToken(string strApplicationName ,string token)
		{
			try
			{
				if (token.size() < 1)
					return false;

				ContinueWatchingImplFactory continueWatchingImplFactory;
				ContinueWatchingImpl *continueWatchingImpl = NULL;
				continueWatchingImpl = continueWatchingImplFactory.createContinueWatchingImpl(strApplicationName);
				if (!continueWatchingImpl)
					return false;

				bool result = continueWatchingImpl->setApplicationToken(token);
				delete continueWatchingImpl;
				continueWatchingImpl = NULL;
				return result;
			}
			catch (...) {
				LOGWARN("exception occurred during setAppToken 4\n");
				return false;
			}
		}

		/**
		 * @brief This function is used to delete the application token.
		 *
		 * @param[in] strApplicationName Variable of application name String.
		 *
		 * @return true
		 */
		bool ContinueWatching::deleteAppToken(std::string strApplicationName)
		{
			try
			{
				ContinueWatchingImplFactory continueWatchingImplFactory;
				ContinueWatchingImpl *continueWatchingImpl = NULL;
				continueWatchingImpl = continueWatchingImplFactory.createContinueWatchingImpl(strApplicationName);
				if (!continueWatchingImpl)
					return false;

				bool result = continueWatchingImpl->deleteApplicationToken();
				delete continueWatchingImpl;
				continueWatchingImpl = NULL;
				return result;
			}
			catch (...) {
				return false;
			}
		}

		/**
		 * @brief Class ContinueWatchingImpl Constructor.
		 *
		 * @return None.
		 */
		ContinueWatchingImpl::ContinueWatchingImpl()
		{
		}

		/**
		 * @brief Class ContinueWatchingImpl Destructor.
		 *
		 * @return None.
		 */

		ContinueWatchingImpl::~ContinueWatchingImpl()
		{
		}

		/**
		 * @brief Class ContinueWatchingImplFactory Constructor.
		 *
		 * @return None.
		 */
		ContinueWatchingImplFactory::ContinueWatchingImplFactory()
		{
		}

		/**
		 * @brief Class ContinueWatchingImplFactory Destructor.
		 *
		 * @return None.
		*/
		ContinueWatchingImplFactory::~ContinueWatchingImplFactory()
		{
		}

		/**
		 * @brief This function is used to encrypt the data.
		 *
		 * @param[in] clearData Pointer of uint8_t, it is a plane data.
		 * @param[in] clearDataLength variable of int, it is a plane data length.
		 * @param[out] protectedData Pointer of uint8_t, it is a cipher data.
		 * @param[out] protectedDataLength variable of int, it is a cipher data length.
		 * @param[out] bytesWritten ref of int, it defines no of bytes written.
		 *
		 * @return True if it able to encrypt data else false.
		 */
		bool ContinueWatchingImpl::encryptData(uint8_t* clearData, int clearDataLength, uint8_t* protectedData, int protectedDataLength, int &bytesWritten)
		{
		#if !defined(DISABLE_SECAPI)
			Sec_ProcessorHandle *sec_proc = NULL;
			Sec_KeyHandle *sec_key = NULL;
			Sec_StorageLoc loc = SEC_STORAGELOC_FILE;
			Sec_Result result = SEC_RESULT_SUCCESS;
			Sec_CipherHandle *sec_cipher = NULL;
			SEC_BYTE iv[SEC_CIPHER_IV_MAX_LEN] = { 0 };
			Sec_CipherAlgorithm algorithm = SEC_CIPHERALGORITHM_AES_CBC_PKCS7_PADDING;
			Sec_CipherMode mode = SEC_CIPHERMODE_ENCRYPT;
			bool retVal = true;

			if(!tr181FeatureEnabled()) {
				LOGWARN("%s Feature DISABLED\n", __FUNCTION__);
				return false;
			}

			try
			{
				result = SecProcessor_GetInstance_Directories(&sec_proc, "/opt/drm", "/opt/drm/servicemgr");
				if(result != SEC_RESULT_SUCCESS)
					throw "Failure to get SecAPI Processor Instance!";

				if(!SecKey_IsProvisioned(sec_proc, mSecObjectId)) {
					result = SecKey_Generate(sec_proc, mSecObjectId, SEC_KEYTYPE_AES_128,loc);
					if(result != SEC_RESULT_SUCCESS)
						throw "Failure to generate new key!";
				}

				result = SecKey_GetInstance(sec_proc, mSecObjectId, &sec_key);
				if(result != SEC_RESULT_SUCCESS)
					throw "Failure to get SecAPI key handle!";

				result = SecCipher_GetInstance(sec_proc, algorithm, mode, sec_key, iv, &sec_cipher);
				if(result != SEC_RESULT_SUCCESS)
					throw "Failure to get SecAPI cipher handler!";

				result = SecCipher_Process(sec_cipher, clearData, clearDataLength, 1, protectedData, protectedDataLength, (SEC_SIZE*)&bytesWritten);
				if(result != SEC_RESULT_SUCCESS)
				throw "Failure to process the data!";
			}
			catch (const char* msg) {
				LOGWARN("%s %s result = %d\n", __FUNCTION__,msg, result);
				retVal = false;
			}

			if(sec_cipher) {
				SecCipher_Release(sec_cipher);
				sec_cipher = NULL;
			}

			if(sec_key) {
				SecKey_Release(sec_key);
				sec_key = NULL;
			}

			if(sec_proc) {
				SecProcessor_Release(sec_proc);
				sec_proc = NULL;
			}
			return retVal;
		#else
			LOGERR("SecAPI is not enabled on this platform.\n");
		return false;
		#endif
		}

		/**
		 * @brief This function is used to decrypt the data.
		 *
		 * @param[in] protectedData Pointer of uint8_t, it is a cipher data.
		 * @param[in] protectedDataLength variable of int, it is a cipher data length.
		 * @param[out] clearData Pointer of uint8_t, it is a plane data.
		 * @param[out] clearDataLength variable of int, it is a plane data length.
		 * @param[out] bytesWritten ref of int, it defines no of bytes written.
		 *
		 * @return True if it able to decrypt data else false.
		 */
		bool ContinueWatchingImpl::decryptData(uint8_t* protectedData, int protectedDataLength, uint8_t* clearData, int clearDataLength, int &bytesWritten)
		{
			#if !defined(DISABLE_SECAPI)
			Sec_ProcessorHandle *sec_proc = NULL;
			Sec_KeyHandle *sec_key = NULL;
			Sec_StorageLoc loc = SEC_STORAGELOC_FILE;
			Sec_Result result = SEC_RESULT_SUCCESS;
			Sec_CipherHandle *sec_cipher = NULL;
			SEC_BYTE iv[SEC_CIPHER_IV_MAX_LEN] = { 0 };
			Sec_CipherAlgorithm algorithm = SEC_CIPHERALGORITHM_AES_CBC_PKCS7_PADDING;
			Sec_CipherMode mode = SEC_CIPHERMODE_DECRYPT;
			bool retVal = true;

			if(!tr181FeatureEnabled()) {
				LOGWARN("%s Feature DISABLED\n", __FUNCTION__);
				return false;
			}

			try
			{
				result = SecProcessor_GetInstance_Directories(&sec_proc, "/opt/drm", "/opt/drm/servicemgr");
				if(result != SEC_RESULT_SUCCESS){
					throw "Failure to get SecAPI Processor Instance!";
				}

				if(!SecKey_IsProvisioned(sec_proc, mSecObjectId)) {
					result = SecKey_Generate(sec_proc, mSecObjectId, SEC_KEYTYPE_AES_128, loc);
					if(result != SEC_RESULT_SUCCESS){
						throw "Failure to generate new key!";
					}
				}

				result = SecKey_GetInstance(sec_proc, mSecObjectId, &sec_key);
				if(result != SEC_RESULT_SUCCESS){
					throw "Failure to get SecAPI key handler!";
				}

				result = SecCipher_GetInstance(sec_proc, algorithm, mode, sec_key, iv, &sec_cipher);
				if(result != SEC_RESULT_SUCCESS){
					throw "Failure to get SecAPI cipher handler!";
				}

				result = SecCipher_Process(sec_cipher, protectedData, protectedDataLength, 1, clearData, clearDataLength, (SEC_SIZE*)&bytesWritten);
				if(result != SEC_RESULT_SUCCESS){
					throw "Failure to process the data!";
				}
 			}
			catch (const char* msg) {
				LOGWARN(" msg %s ",msg);
				retVal = false;
			}

			if(sec_cipher) {
				SecCipher_Release(sec_cipher);
				sec_cipher = NULL;
			}

			if(sec_key) {
				SecKey_Release(sec_key);
				sec_key = NULL;
			}

			if(sec_proc) {
				SecProcessor_Release(sec_proc);
				sec_proc = NULL;
			}

			return retVal;
		#else
			LOGWARN("%s SecAPI is not enabled on this platform.\n", __FUNCTION__);
			return false;
		#endif
		}

		/**
		 * @brief This function is used to write the protectedData into file.
		 *
		 * @param[in] protectedData Variable of string.
		 *
		 * @return True if it able to write data into file.
		 */
		bool ContinueWatchingImpl::writeToJson(std::string protectedData)
		{
			char *jsonDoc = NULL;
			char *jsonOut = NULL;
			cJSON *root = NULL;
			bool createNewJsonDoc = true;

			FILE *file = fopen(CW_LOCAL_FILE, "r");
			if (file) {
				fseek(file, 0, SEEK_END);
				long numbytes = ftell(file);
				jsonDoc = (char*)malloc(sizeof(char)*(numbytes + 1));
				if(jsonDoc == NULL) {
					fclose(file);
					return false;
				}

				fseek(file, 0, SEEK_SET);
				fread(jsonDoc, numbytes, 1, file);
				fclose(file);
				file = NULL;
				jsonDoc[numbytes] = 0;

				root = cJSON_Parse(jsonDoc);
				cJSON *tokens = cJSON_GetObjectItem(root, "tokens");
				if (tokens) {
					int i;
					bool tokenUpdated = false;
					int tokensCount = cJSON_GetArraySize(tokens);
					createNewJsonDoc = false;
					for (i = 0; i < tokensCount; i++) {
						cJSON *token = cJSON_GetArrayItem(tokens, i);
						cJSON *item = cJSON_GetObjectItem(token, "applicationName");
						if (item) {
							//updating token data
							if (strcmp(item->valuestring, mStrApplicationName.c_str()) == 0) {
								tokenUpdated = true;
								cJSON_ReplaceItemInObject(token, "encryptedData", cJSON_CreateString(protectedData.c_str()));
				 				break;
							}
						}
					}

					//Adding new entry to list
					if(!tokenUpdated) {
						cJSON *jsonItem = cJSON_CreateObject();

						cJSON_AddItemToObject(jsonItem, "applicationName", cJSON_CreateString(mStrApplicationName.c_str()));
						cJSON_AddItemToObject(jsonItem, "encryptedData", cJSON_CreateString(protectedData.c_str()));
						cJSON_AddItemToArray(tokens, jsonItem);
					}
				}
				free(jsonDoc);
				jsonDoc = NULL;
			}

			if (createNewJsonDoc) {
				cJSON *tokenArray;
				cJSON *jsonItem;

				cJSON_Delete(root);
				root  = cJSON_CreateObject();
				tokenArray=  cJSON_CreateArray();
				cJSON_AddItemToObject(root, "tokens", tokenArray);
				jsonItem = cJSON_CreateObject();
				cJSON_AddItemToObject(jsonItem, "applicationName", cJSON_CreateString(mStrApplicationName.c_str()));
				cJSON_AddItemToObject(jsonItem, "encryptedData", cJSON_CreateString(protectedData.c_str()));
				cJSON_AddItemToArray(tokenArray, jsonItem);
			}

			jsonOut = cJSON_Print(root);
			cJSON_Delete(root);

			file = fopen(CW_LOCAL_FILE, "w");
			if (!file) {
				free(jsonOut);
				jsonOut = NULL;
				return false;
			}

			fprintf(file, "%s", jsonOut);

			fclose (file);
			file = NULL;
			free(jsonOut);
			jsonOut = NULL;

			return true;
		}

		/**
		 * @brief This function is used to read the protectedData from file.
		 *
		 * @return protectedData.
		 */
		std::string ContinueWatchingImpl::readFromJson()
		{
			char *jsonDoc = NULL;
			std::string retVal = "";
			FILE *file = fopen(CW_LOCAL_FILE, "r");

			if (!file)
				return retVal;

			fseek(file, 0, SEEK_END);
			long numbytes = ftell(file);
			jsonDoc = (char*)malloc(sizeof(char)*(numbytes + 1));
			if(jsonDoc == NULL) {
				fclose(file);
				return retVal;
			}

			fseek(file, 0, SEEK_SET);
			fread(jsonDoc, 1, numbytes, file);
			fclose(file);
			jsonDoc[numbytes] = '\0';

			cJSON *root = cJSON_Parse(jsonDoc);
			cJSON *tokens = cJSON_GetObjectItem(root, "tokens");
			int tokensCount = cJSON_GetArraySize(tokens);
			int i;

			for (i = 0; i < tokensCount; i++) {
				cJSON *token = cJSON_GetArrayItem(tokens, i);
				cJSON *item = cJSON_GetObjectItem(token, "applicationName");
				if (item) {
					if (strcmp(item->valuestring, mStrApplicationName.c_str()) == 0) {
						cJSON *encrypteDataItem = cJSON_GetObjectItem(token, "encryptedData");
						if (encrypteDataItem){
							retVal = encrypteDataItem->valuestring;
						}
						break;
					}
				}
			}
			cJSON_Delete(root);
			root = NULL;
			free(jsonDoc);
			root = NULL;

			return retVal;
		}

		/**
		 * @brief This function is used to delete the token from file.
		 *
		 * @return True.
		 */
		bool ContinueWatchingImpl::deleteToken()
		{
			char *jsonDoc = NULL;
			char *jsonOut = NULL;
			cJSON *root = NULL;
			bool retVal = false;

			if(!tr181FeatureEnabled()) {
				LOGWARN("Feature DISABLED...\n");
				return false;
			}

			FILE *file = fopen(CW_LOCAL_FILE, "r");
			if (!file)
				return retVal;

			fseek(file, 0, SEEK_END);
			long numbytes = ftell(file);
			jsonDoc = (char*)malloc(sizeof(char)*(numbytes + 1));
			if(jsonDoc==NULL) {
				fclose(file);
				return false;
			}
			fseek(file, 0, SEEK_SET);
			fread(jsonDoc, numbytes, 1, file);
			fclose(file);
			file = NULL;
			jsonDoc[numbytes] = 0;

			root = cJSON_Parse(jsonDoc);
			cJSON *tokens = cJSON_GetObjectItem(root, "tokens");
			if (tokens) {
				int i;
				int tokensCount = cJSON_GetArraySize(tokens);
				for (i = 0; i < tokensCount; i++) {
					cJSON *token = cJSON_GetArrayItem(tokens, i);
					cJSON *item = cJSON_GetObjectItem(token, "applicationName");
					if (item) {
						if (strcmp(item->valuestring, mStrApplicationName.c_str()) == 0) {
							retVal = true;
							cJSON_DeleteItemFromArray(tokens, i);
							break;
						}
					}
				}
			}
			free(jsonDoc);
			jsonDoc = NULL;

			if (!retVal) {
				cJSON_Delete(root);
				return retVal;
			}

			jsonOut = cJSON_Print(root);
			cJSON_Delete(root);

			file = fopen(CW_LOCAL_FILE, "w");
			if (!file) {
				free(jsonOut);
				jsonOut = NULL;
				retVal = false;
				return retVal;
			}

			fprintf(file, "%s", jsonOut);
			fclose (file);
			file = NULL;
			free(jsonOut);
			jsonOut = NULL;
			return true;
		}

		/**
		 * @brief This function is used generate sha256 hash for given string.
		 *
		 * @return sha256 hash.
		 */
		std::string ContinueWatchingImpl::sha256(const std::string str)
		{
			unsigned char hash[SHA256_DIGEST_LENGTH];
			SHA256_CTX sha256Ctx;
			SHA256_Init(&sha256Ctx);
			SHA256_Update(&sha256Ctx, str.c_str(), str.size());
			SHA256_Final(hash, &sha256Ctx);
			std::stringstream strStream;
			/* Iterate through hash & convert each byte to 2-char wide hex */
			for(int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
				strStream << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
			}
			return strStream.str();
		}

		/**
		 * @brief This function is used to read RFC configuration data.
		 *
		 * @return True.
		 */
		bool ContinueWatchingImpl::checkTR181(const std::string& feature)
		{
			bool retVal = false;
			RFC_ParamData_t param         = {0};

			WDMP_STATUS status = getRFCParameter(const_cast<char *>("ServiceManager"), feature.c_str(), &param);
			if (status == WDMP_SUCCESS) {
				LOGINFO("%s name = %s, type = %d, value = %s\n", __FUNCTION__, param.name, param.type, param.value);

				if (!strncmp(param.value, "true", strlen("true"))) {
					retVal = true;
				}
				else {
					LOGWARN("%s is NOT enabled.\n", feature.c_str());
				}
			}
			else {
				LOGERR("getRFCParameter Failed..status %d ",__LINE__);
			}
			return retVal;
		}

		/**
		 * @brief This function is used to read RFC configuration data.
		 *
		 * @return True.
		 */
		bool ContinueWatchingImpl::tr181FeatureEnabled()
		{
                      #if defined(PLATFORM_AMLOGIC)
                                  return true;
                      #endif
			bool retVal = false;

			// Get RFC value for Continue Watching
			if(checkTR181(CW_TR181_PARAMETER)) {
				LOGINFO("checkTR181 reutrns true \n");
			retVal = true;
			}

			// Allow environmential override
			char* envFeatureControl = getenv(CW_ENV_PARAMETER);
			if(envFeatureControl && strcasecmp(envFeatureControl, "false") == 0) {
				// Disable the feature
				retVal = false;
				LOGINFO("%s: found environment setting %s, setting enabled to %d\n", __FUNCTION__, CW_ENV_PARAMETER, retVal);
			}
			else if(envFeatureControl && strcasecmp(envFeatureControl, "true") == 0) {
				// enable the feature
				retVal = true;
				LOGINFO("%s: found environment setting %s, setting enabled to %d\n", __FUNCTION__, CW_ENV_PARAMETER, retVal);
			}
			return retVal;
		}

		//netflix continue watching implementation
		/**
		 * @brief NetflixContinueWatchingImpl Class Constructor.
		 *
		 * @return None.
		*/
		NetflixContinueWatchingImpl::NetflixContinueWatchingImpl()
		{
			mStrApplicationName = NETFLIX_CONTINUEWATCHING_APP_NAME;
			#if !defined(DISABLE_SECAPI)
			mSecObjectId = SEC_OBJECTID_CW_NETFLIX_STORAGE_KEY;
			#endif
		}

		/**
		 * @brief NetflixContinueWatchingImpl Class Destructor.
		 *
		 * @return None.
		 */
		NetflixContinueWatchingImpl::~NetflixContinueWatchingImpl()
		{
		}

		/**
		 * @brief This function is used to get the application token.
		 *
		 * @return string.
		 */

		std::string NetflixContinueWatchingImpl::getApplicationToken()
		{
			string encencryptData = readFromJson();
                      if (encencryptData.empty()) {
                          return encencryptData;
                      }
			int paddingLength = 0;
			string decodedencryptedtokenbase64;

			//decode data as json file has encoded base64 data written after encryption
			std::vector<uint8_t> encencryptDataVec(encencryptData.begin(), encencryptData.end());
			uint8_t *baseencencryptdata = &encencryptDataVec[0];
			size_t workspace_size = b64_get_decoded_buffer_size(encencryptDataVec.size());
			uint8_t *workspace = (uint8_t*)malloc(workspace_size);
			if(workspace == NULL) {
				return "";
			}
			size_t num_chars = b64_decode(baseencencryptdata,(encencryptDataVec.size()), workspace);

		#if !defined(DISABLE_SECAPI)
			paddingLength = (SEC_AES_BLOCK_SIZE - num_chars % SEC_AES_BLOCK_SIZE);
		#endif

			int SIZE = num_chars + paddingLength;
			uint8_t output[SIZE];
			int bytesWritten;
			bool result;

			result = decryptData(workspace, num_chars, output, sizeof(output), bytesWritten);
			if (!result){
				LOGERR("decryptData failed..\n");
				free(workspace);
				return "";
			}

			string strOutput;
			std::stringstream list1;
			for (int i=0; i<bytesWritten; ++i){
				list1 << output[i];
			}
			strOutput = list1.str();

			int hashLength = SHA256_DIGEST_LENGTH*2;
			std::string inputHash = strOutput.substr (0,hashLength);
			std::string tokenData = strOutput.substr (hashLength, bytesWritten-hashLength);
			std::string tokenHash = sha256(tokenData);

			if (inputHash != tokenHash) {
				LOGWARN("checksum not verified \n");
				free(workspace);
				return "";
			}

			std::vector<uint8_t> dectokenVec(tokenData.begin(), tokenData.end());
			uint8_t *e = &dectokenVec[0];
			size_t decworkspace_size = b64_get_decoded_buffer_size(dectokenVec.size());
			uint8_t *decworkspace = (uint8_t*)malloc(decworkspace_size);
			if(decworkspace == NULL) {
				free(workspace);
				return "";
			}
			size_t decnum_chars = b64_decode(e, dectokenVec.size(),decworkspace);

			string dectokenbase64;
			std::stringstream listdecydec;
			for (unsigned int i=0; i<decnum_chars; ++i){
				listdecydec << decworkspace[i];
			}
			dectokenbase64 = listdecydec.str();
			free(workspace);
			free(decworkspace);
			return dectokenbase64;
		}

		/**
		 * @brief This function is used to set the application token.
		 *
		 * @param[in] token Variable of token value string.
		 *
		 * @return true.
		 */
		bool NetflixContinueWatchingImpl::setApplicationToken(string token)
		{
			bool result;
			string tokenbase64;

			std::vector<uint8_t> tokenVec(token.begin(), token.end());

			uint8_t *encodebuff = &tokenVec[0];
			size_t workspace_size = b64_get_encoded_buffer_size(tokenVec.size());
			uint8_t *workspace = (uint8_t*)malloc(workspace_size);
			if(workspace == NULL) {
				return false;
			}
			b64_encode(encodebuff,tokenVec.size(),workspace);

			std::stringstream list1;
			for (unsigned int i=0; i<workspace_size; ++i){
				list1 << workspace[i];
			}
			tokenbase64 = list1.str();

			//add hash of sha256
			string hash = sha256(tokenbase64);
			tokenbase64 = hash + tokenbase64;

			//padding length
			int paddingLength = 0;
		#if !defined(DISABLE_SECAPI)
			paddingLength = (SEC_AES_BLOCK_SIZE - tokenbase64.length() % SEC_AES_BLOCK_SIZE);
		#endif
			int SIZE = tokenbase64.length() + paddingLength;
			uint8_t cipher[SIZE];
			int bytesWritten;

			std::vector<uint8_t> forencryptVec(tokenbase64.begin(),tokenbase64.end());
			uint8_t *buff = &forencryptVec[0];
			result = encryptData(buff, forencryptVec.size(), cipher, sizeof(cipher), bytesWritten);
			if (!result)
			{
				free(workspace);
				return false;
			}


			size_t jsonworkspace_size = b64_get_encoded_buffer_size(sizeof(cipher));
			uint8_t *jsonworkspace = (uint8_t*)malloc(jsonworkspace_size);
			if(jsonworkspace == NULL) {
				free(workspace);
				return false;
			}
			b64_encode(cipher,sizeof(cipher),jsonworkspace);

			string jsontokenbase64;
			std::stringstream listwritetofile;
			for (unsigned int i=0; i<jsonworkspace_size; ++i){
				listwritetofile << jsonworkspace[i];
			}
			jsontokenbase64 = listwritetofile.str();
			result = writeToJson(jsontokenbase64);
			free(workspace);
			free(jsonworkspace);
			return result;
		}

		/**
		 * @brief This function is used to delete the application token.
		 *
		 * @return true.
		 */
		bool NetflixContinueWatchingImpl::deleteApplicationToken()
		{
			return deleteToken();
		}

		/*
		 * @brief This function is used to create ContinueWatchingImpl object based on strApplicationName.
		 *
		 * @param[in] strApplicationName Variable of application name string.
		 *
		 * @return ContinueWatchingImpl*.
		 */
		ContinueWatchingImpl* ContinueWatchingImplFactory::createContinueWatchingImpl(std::string strApplicationName)
		{
			ContinueWatchingImpl* continueWatchingImpl = NULL;

			if (strApplicationName == NETFLIX_CONTINUEWATCHING_APP_NAME) {
				continueWatchingImpl = new NetflixContinueWatchingImpl;
			}
			return continueWatchingImpl;
		}

		/**
		 * @brief This function is used to get the API's Version Number.
		 *
		 * @return apiVersionNumber
		 */
		uint32_t ContinueWatching::getApiVersionNumber()
		{
			return m_apiVersionNumber;
		}

		/**
		 * @brief This function is used to set the API's Version Number.
		 *
		 * @param[in] apiVersionNumber Version number of API.
		 *
		 * @return none
		 */

		void ContinueWatching::setApiVersionNumber(unsigned int apiVersionNumber)
		{
			LOGINFO("setApiVersionNumber enter version=%d", (int)apiVersionNumber);
			m_apiVersionNumber=apiVersionNumber;
		}
	} // namespace Plugin
} // namespace WPEFramework
