#include "TTSDownloader.h"
#include "TTSURLConstructer.h"
#include <curl/curl.h>
#include <unistd.h>

#define CONFIG_PATH "http://localhost:50050/TTS_fallback.mp3"
#define CONFIG_FILE "/opt/www/TTS_fallback.mp3"


static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

namespace TTS
{

TTSDownloader::TTSDownloader(TTSConfiguration &config):
   m_defaultConfig(config)
{
   m_downloadThread = NULL;
   needDownload = false;
   active = false;
   TTSLOG_WARNING("Constructer TTSDownloader\n");
}

TTSDownloader::~TTSDownloader()
{
    if(m_downloadThread)
    {
        {
            std::lock_guard<std::mutex> lock(m_queueMutex);
            needDownload = true;
            active = false;
            m_condition.notify_one();
        }
        m_downloadThread->join();
        delete m_downloadThread;
        m_downloadThread = NULL;
    }
     TTSLOG_INFO("TTSDownloader::~TTSDownloader()\n");
}


void TTSDownloader::download(TTSConfiguration config)
{
    TTSLOG_INFO("TTSDownloader::download \n");
    m_objectMutex.lock();
    m_config = config;  
    m_objectMutex.unlock();  
    if(!m_downloadThread)
    {
	active = true;
        TTSLOG_INFO("TTSDownloader::download new download thread\n");
        m_downloadThread = new std::thread(&TTSDownloader::downloadThread, this);
    }
    needDownload = true;
    m_condition.notify_one();
    TTSLOG_INFO("TTSDownloader::download notify for a new download\n");
}


void TTSDownloader::downloadThread()
{
   while(active)
   {	   
       std::unique_lock<std::mutex> lock(m_queueMutex);
       m_condition.wait(lock,[this]{return needDownload.load();});

       if(active == false)
       {
           //invoked from destructor..no download
           break;
       }
       needDownload = false;
       std::string tts_request;
       TTSURLConstructer url;
       m_objectMutex.lock();
       tts_request = url.constructURL(m_config,"",true);
       m_objectMutex.unlock();
       while(tts_request.empty())
       {
          sleep(60);
          //looks like no internet..wait for 1 min and retry
          m_objectMutex.lock();
          tts_request = url.constructURL(m_config,"",true);
          m_objectMutex.unlock();
       }
       if((tts_request.compare("null")) == 0)
       {
          //got response ..invalid x-api-key set by app
          TTSLOG_INFO("TTSDownloader::download got invalid response from server\n");
          continue;
       }
       TTSLOG_INFO("TTSDownloader::download going to download file from location %s\n",tts_request.c_str());
       downloadFile(tts_request);
    }
}

void TTSDownloader::downloadFile(std::string tts_request)
{
   CURL *curl;
   CURLcode res;
   curl = curl_easy_init();
   if(curl)
   {
       char outfilename[FILENAME_MAX] = CONFIG_FILE;
       FILE *fp;
       fp = fopen(outfilename,"wb");
       curl_easy_setopt(curl, CURLOPT_URL, tts_request.c_str());
       curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, NULL);
       curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
       res = curl_easy_perform(curl);
       if ( res != CURLE_OK )
       {
          TTSLOG_INFO("TTSDownloader::download file download failed..needs re-download\n");
          needDownload = true; //need re-download
       }
       else
       {
          saveConfiguration(CONFIG_PATH);
       }
       curl_easy_cleanup(curl);
       fclose(fp);
    }
}

void TTSDownloader::saveConfiguration(std::string path)
{
   TTSLOG_INFO("TTSDownloader saveconfiguration path %s\n",path.c_str());
   m_objectMutex.lock();
   m_config.saveFallbackPath(path);
   m_config.updateConfigStore();
   m_defaultConfig.saveFallbackPath(path);
   m_objectMutex.unlock();
}

}//end of namespace TTS
