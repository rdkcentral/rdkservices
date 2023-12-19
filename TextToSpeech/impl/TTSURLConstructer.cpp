#include "TTSURLConstructer.h"
#include <curl/curl.h>
#include <unistd.h>

static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

namespace TTS
{

TTSURLConstructer::TTSURLConstructer() { 

}

TTSURLConstructer::~TTSURLConstructer() {

}

std::string TTSURLConstructer::constructURL(TTSConfiguration &config, std::string text, bool isFallback, bool isLocal) {
    if(!(config.apiKey().empty()) && !isLocal) {
          TTSLOG_INFO("Device using remote sky endpoint");
          return httppostURL(config, text, isFallback);
     } else {
          TTSLOG_INFO("Device using %s endpoint", isLocal? "Local":"Remote");
          return httpgetURL(config, text, isFallback, isLocal);
     }
}

std::string TTSURLConstructer::httpgetURL(TTSConfiguration &config, std::string text, bool isfallback, bool isLocal) {
    // EndPoint URL
    std::string ttsRequest;
    ttsRequest.append(isLocal ? config.localEndPoint() : config.secureEndPoint());

    // Voice
    if(!config.voice().empty()) {
        ttsRequest.append("voice=");
        ttsRequest.append(isLocal ? config.localVoice() : config.voice());
    }

    // Language
    if(!config.language().empty()) {
        ttsRequest.append("&language=");
        ttsRequest.append(config.language());
    }

    if(!isLocal && ((config.endPointType().compare("TTS2")) == 0)) {
        ttsRequest.append("&speaking_rate=");
        ttsRequest.append(config.speechRate());
    } else {
        // Rate / speed
        ttsRequest.append("&rate=");
        ttsRequest.append(std::to_string(config.rate() > 100 ? 100 : config.rate()));
    }

    // Sanitize String
    std::string sanitizedString;
    sanitizeString((isfallback ? config.getFallbackValue() : text), sanitizedString);

    ttsRequest.append("&text=");
    ttsRequest.append(sanitizedString);
    return ttsRequest;
}

std::string  TTSURLConstructer::httppostURL(TTSConfiguration &config, std::string text, bool isFallback) {
    std::string ttsRequest;
    CURL *curl = curl_easy_init();

    if(curl) {
        CURLcode res;
        struct curl_slist *list = NULL;
        std::string readBuffer;
        JsonObject jsonConfig;
        JsonObject parameters;
        std::string post_data;

        if(isFallback) {
            jsonConfig["input"] = config.getFallbackValue();
        } else {
            jsonConfig["input"] = text;
        } 

        jsonConfig["language"] = config.language();
        jsonConfig["voice"] = config.voice();
        jsonConfig["encoding"] = "mp3";
        jsonConfig["speed"] = config.speechRate();
        jsonConfig.ToString(post_data);
        TTSLOG_INFO("gcd postdata :%s\n",post_data.c_str());

        curl_easy_setopt(curl, CURLOPT_URL,config.secureEndPoint().c_str());
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 2L);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_data.c_str());
        list = curl_slist_append(list, "content-type: application/json");
        list = curl_slist_append(list, (std::string("x-api-key: ") +
                                            config.apiKey()).c_str() );
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, list);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

        res = curl_easy_perform(curl);
        if ( res != CURLE_OK ) {
            TTSLOG_ERROR("TTS: Error in interacting with endpoint. CURL error is:  %s\n", curl_easy_strerror(res));
            if( config.isFallbackEnabled() && isFallback == false) {
                TTSLOG_INFO("RDK TTS: Device is not connected with Internet, hence speaking fallback text in place of actual text: %s",text.c_str());
                ttsRequest.assign(config.getFallbackPath());
            } 
        } else {
            TTSLOG_INFO("gcd curl response :%s\n",readBuffer.c_str());
            parameters.FromString(readBuffer);
            ttsRequest.assign(parameters["url"].String());
        }
        curl_slist_free_all(list);
        curl_easy_cleanup(curl);
    }
    return ttsRequest;
}

void TTSURLConstructer::replaceIfIsolated(std::string& text, const std::string& search, const std::string& replace, bool skipIsolationCheck) {
    size_t pos = 0;
    while ((pos = text.find(search, pos)) != std::string::npos) {
        bool punctBefore = (pos == 0 || std::ispunct(text[pos-1]) || std::isspace(text[pos-1]));
        bool punctAfter = (pos+1 == text.length() || std::ispunct(text[pos+1]) || std::isspace(text[pos+1]));

        if((punctBefore && punctAfter) || skipIsolationCheck) {
            text.replace(pos, search.length(), replace);
            pos += replace.length();
        } else {
            pos += search.length();
        }
    }
}

bool TTSURLConstructer::isSilentPunctuation(const char c) {
    static std::string SilentPunctuation = "?!:;-()";
    return (SilentPunctuation.find(c) != std::string::npos);
}

void TTSURLConstructer::replaceSuccesivePunctuation(std::string& text) {
    size_t pos = 0;
    while(pos < text.length()) {
        // Remove unwanted characters
        static std::string stray = "\"";
        if(stray.find(text[pos]) != std::string::npos) {
            text.erase(pos,1);
            if(++pos == text.length())
                break;
        }

        if(ispunct(text[pos])) {
            ++pos;
            while(pos < text.length() && (isSilentPunctuation(text[pos]) || isspace(text[pos]))) {
                if(isSilentPunctuation(text[pos]))
                    text.erase(pos,1);
                else
                    ++pos;
            }
        } else {
            ++pos;
        }
    }
}

void TTSURLConstructer::curlSanitize(std::string &sanitizedString) {
    CURL *curl = curl_easy_init();
    if(curl) {
      char *output = curl_easy_escape(curl, sanitizedString.c_str(), sanitizedString.size());
      if(output) {
          sanitizedString = output;
          curl_free(output);
      }
    }
    curl_easy_cleanup(curl);
}

void TTSURLConstructer::sanitizeString(const std::string &input, std::string &sanitizedString) {
    sanitizedString = input;

    replaceIfIsolated(sanitizedString, "://", " colon slash slash ", true);
    replaceIfIsolated(sanitizedString, "$", "dollar");
    replaceIfIsolated(sanitizedString, "#", "pound");
    replaceIfIsolated(sanitizedString, "&", "and");
    replaceIfIsolated(sanitizedString, "|", "bar");
    replaceIfIsolated(sanitizedString, "/", "or");

    replaceSuccesivePunctuation(sanitizedString);

    curlSanitize(sanitizedString);

    TTSLOG_VERBOSE("In:%s, Out:%s", input.c_str(), sanitizedString.c_str());
}

}//namespace tts end
