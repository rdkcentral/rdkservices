#ifndef HELPER_HEADER
#define HELPER_HEADER
#include <string>

SourceType sourceTypeFromString(std::string str)
{
   if(!str.compare("data"))
    {
        return SourceType::DATA;
    }
    else if(!str.compare("httpsrc"))
    {
        return SourceType::HTTPSRC;
    }
    else if(!str.compare("filesrc"))
    {
        return SourceType::FILESRC;
    }
    else if(!str.compare("websocket"))
    {
        return SourceType::WEBSOCKET;
    }
    else
    {
        return SourceType::SourceType_None;
    }
}

std::string sourceTypeToString(SourceType sourceType)
{
    if(sourceType == SourceType::DATA)
    {
        return "data";
    }

    else if(sourceType == SourceType::HTTPSRC)
    {
        return "httpsrc";
    }

    else if(sourceType == SourceType::FILESRC)
    {
        return "filesrc";
    }
    else if(sourceType == SourceType::WEBSOCKET)
    {
        return "websocket";
    }
    else
    {
        return "error";
    }

}

PlayMode playModeFromString(std::string str)
{
    if(!str.compare("system"))
    {
        return PlayMode::SYSTEM;
    }
    else if(!str.compare("app"))
    {
        return PlayMode::APP;
    }
    else
    {
        return PlayMode::PlayMode_None;
    }
}

std::string playModeToString(PlayMode playMode)
{
    if(playMode == PlayMode::SYSTEM)
    {
        return "system";
    }
    else if(playMode == PlayMode::APP)
    {
        return "app";
    }
    else
    {
        return "error";
    }

}
AudioType audioTypeFromString(std::string str)
{
    if(!str.compare("pcm"))
    {
        return AudioType::PCM;
    }
    else if(!str.compare("mp3"))
    {
        return AudioType::MP3;
    }
    else if(!str.compare("wav"))
    {
        return AudioType::WAV;
    }
    else
    {
        return AudioType::AudioType_None;
    }
}

std::string audioTypeToString(AudioType audioType)
{
    if(audioType == AudioType::PCM)
    {
        return "pcm"; 
    }	    
    else if(audioType == AudioType::MP3)
    {
        return "mp3";
    }
    else if(audioType == AudioType::WAV)
    {
        return "wav";
    }
    else
    {
        return "error";
    }
}

bool extractFileProtocol(std::string &url)
{
    std::string input;
    input = url;
    size_t found = input.find("file://");
    if(found != std::string::npos)
    {
        url = input.substr(found+7); //Remove protocol part
        return true;
    }
    return false;
}
#endif
