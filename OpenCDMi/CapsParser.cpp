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

#include "CapsParser.h"

#define START_CHAR ")"
#define END_CHAR ","


#define LOG(level, ...) DebugLogging(level, __FUNCTION__, __LINE__, __VA_ARGS__)
static bool s_bLogTrace = false;

typedef enum eLogLevel_
{
    eTrace,
    eWarning,
    eError
} eLogLevel;

void DebugLogging(eLogLevel level, const char* function, int line, const char * format, ...)
{
#define LOG_MESSAGE_SIZE 4096

    if(!s_bLogTrace && level < eWarning) {
        // Do not log
        return;
    }

    char logMessage[LOG_MESSAGE_SIZE];
    // Generate the log string
    va_list ap;
    va_start(ap, format);
    vsnprintf(logMessage, LOG_MESSAGE_SIZE, format, ap);
    va_end(ap);

    FILE* fpOut = stdout;
    if(level == eError) {
        fpOut = stderr;
    }

    // printf for now.
    fprintf(fpOut, "CapParser: %s(%d) : %s", function, line, logMessage);
    fflush(fpOut);
    return;
}

//#define ALWAYS_LOG 1
static void _SetLogLevelTrace(bool bTrace)
{
#ifndef ALWAYS_LOG
	if(s_bLogTrace != bTrace) {
		LOG(eError, "Setting s_bLogTrace to %d\n", bTrace);
		s_bLogTrace = bTrace;
	}
#else
	s_bLogTrace = true;
#endif
}
static inline bool _GetLogLevelTrace()
{
	return s_bLogTrace;
}

namespace WPEFramework {
    namespace Plugin {

        CapsParser::CapsParser() 
        : _mediaTag("original-media-type")
        , _widthTag("width")
        , _heightTag("height")
        , _lastHash(0)
        {
            char* envTraceLog = getenv("GSTCAPS_ENABLE_TRACE_LOGGING");
            if(envTraceLog && strcasecmp(envTraceLog, "true") == 0) {
                _SetLogLevelTrace(true);
                LOG(eWarning, "using GSTCAPS_ENABLE_TRACE_LOGGING set TRACE Logging to %d\n", _GetLogLevelTrace());
            }
            else {
                _SetLogLevelTrace(false);
            }
        }

        CapsParser::~CapsParser() 
        {
        }

        void CapsParser::Parse(const uint8_t* info, uint16_t infoLength) /* override */ 
        {
            LOG(eTrace, "Got a new info string size %d (%p)\n", infoLength, info);
            if(infoLength > 0) {
                ::string infoStr((char*)info, (size_t)infoLength);

                std::hash<::string> hash_fn;
                size_t info_hash = hash_fn(infoStr);
                if(_lastHash != info_hash) {
                    LOG(eTrace, "Got a new info string %s hash = %ld\n", infoStr.c_str(), info_hash);
                    _lastHash = info_hash;
                     ::string substring =infoStr.substr(0,5);

                    // Parse the data
                    ::string result = FindMarker(infoStr, _mediaTag);
                    if(!result.empty()) {
                        SetMediaType(result);
                    }
                    else if(substring.compare("audio")==0||substring.compare("video")== 0){
                        SetMediaType(substring);
                    }
                    else {
                        LOG(eError, "No result for media type\n");
                    }
                    if(_mediaType == CDMi::Video) {
                        result = FindMarker(infoStr, _widthTag);
                        if(!result.empty()) {
                            SetWidth(result);
                        }
                        else {
                            LOG(eError, "No result for width\n");
                        }
                        result = FindMarker(infoStr, _heightTag);
                        if(!result.empty()) {
                            SetHeight(result);
                        }
                        else {
                            LOG(eError, "No result for height\n");
                        }
                    }
                    else {
                        // Audio
                        _width  = 0;
                        _height = 0;
                    }
                }
            }
        }

        void CapsParser::SetMediaType(::string& media)
        {
            if(media.find("video") != ::string::npos) {
                _mediaType = CDMi::Video;
            }
            else if(media.find("audio") != ::string::npos) {
                _mediaType = CDMi::Audio;
            }
            else {
                LOG(eError, "Found and unknown media type %s\n", media);
                _mediaType = CDMi::Unknown;
            }
        }

        void CapsParser::SetWidth(::string& width)
        {
            LOG(eTrace, "Setting width to %s\n", width.length() > 0 ? width.c_str() : "NULL");
            if(width.length() > 0) {
                _width = (uint16_t)stoi(width, NULL, 10);
            }
        }

        void CapsParser::SetHeight(::string& height)
        {
            LOG(eTrace, "Setting height to %s\n", height.length() > 0 ? height.c_str() : "NULL");
            if(height.length() > 0) {
                _height = (uint16_t)stoi(height, NULL, 10);
            }
        }

        ::string CapsParser::FindMarker(::string& data, ::string& tag) const
        {
            ::string retVal;

            size_t found = data.find(tag);
            LOG(eTrace, "Found tag <%s> in <%s> at location %d\n", 
                    tag.c_str(), data.c_str(), (int)found);
            if(found != ::string::npos) {
                // Found the marker
                // Find the end of the gst caps type identifier
                size_t start = data.find(START_CHAR, found) + 1;  // step over the ")"
                size_t end = data.find(END_CHAR, start);
                if(end == ::string::npos) {
                    // Went past the end of the string
                    end = data.length();
                }
                retVal = data.substr(start, end - start);
                LOG(eTrace, "Found substr <%s>\n", retVal.c_str());
            }
            return retVal;
        }
    }
}
