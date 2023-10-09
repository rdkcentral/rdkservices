#include "TTSDownloader.h"
#include "TTSURLConstructer.h"
#include <curl/curl.h>
#include <unistd.h>

#define CONFIG_PATH "http://localhost:50050/TTS_fallback.mp3"
#define CONFIG_FILE "/opt/www/TTS_fallback.mp3"


namespace TTS
{

TTSDownloader::TTSDownloader(TTSConfiguration &config):
   m_defaultConfig(config)
{
    m_downloadThread = NULL;
    m_needDownload = false;
    m_active = false;
    TTSLOG_WARNING("Constructer TTSDownloader\n");
}

TTSDownloader::~TTSDownloader()
{
    if(m_downloadThread)
    {
        {
            std::lock_guard<std::mutex> lock(m_queueMutex);
            m_needDownload = true;
            m_active = false;
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
	m_active = true;
        TTSLOG_INFO("TTSDownloader::download new download thread\n");
        m_downloadThread = new std::thread(&TTSDownloader::downloadThread, this);
    }
    m_needDownload = true;
    m_condition.notify_one();
    TTSLOG_INFO("TTSDownloader::download notify for a new download\n");
}


void TTSDownloader::downloadThread()
{
    while(m_active)
    {
        std::unique_lock<std::mutex> lock(m_queueMutex);
        m_condition.wait(lock,[this]{return m_needDownload.load();});

        if(m_active == false) //invoked from destructor..no download
            break;

        m_needDownload = false;
        std::string ttsRequest;
        TTSURLConstructer url;

        m_objectMutex.lock();
        ttsRequest = url.constructURL(m_config, "", true, false);
        m_objectMutex.unlock();

        if((ttsRequest.compare("null")) == 0)
        {
            //got response ..invalid x-api-key set by app
            TTSLOG_INFO("TTSDownloader::download got invalid response from server\n");
            continue;
        }

        TTSLOG_INFO("TTSDownloader::download going to download file from location %s\n", ttsRequest.c_str());
        if(!downloadFile(ttsRequest))
        {
             TTSLOG_INFO("TTSDownloader::downloadFile download failed..needs re-download\n");
             m_needDownload = true; //need re-download
             //looks like no internet/server down..wait for 1 min and retry
             sleep(60);
        }
    }
}

bool TTSDownloader::downloadFile(std::string ttsRequest)
{
    bool downloadDone = false;
    FILE *fp = fopen(CONFIG_FILE, "wb");
    if(NULL != fp)
    {
        CURL *curl = curl_easy_init();
        if(curl)
        {
            curl_easy_setopt(curl, CURLOPT_URL, ttsRequest.c_str());
            curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, NULL);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
            if (curl_easy_perform(curl) != CURLE_OK)
            {
                downloadDone = false;
            }
            else
            {
                downloadDone = true;
                saveConfiguration(CONFIG_PATH);
            }
            curl_easy_cleanup(curl);
            fclose(fp);
        }
    }
    else
    {
        TTSLOG_ERROR("TTSDownloader fopen error\n");
    }
    return downloadDone;
}

void TTSDownloader::saveConfiguration(std::string path)
{
    TTSLOG_INFO("TTSDownloader saveconfiguration path %s\n", path.c_str());
    m_objectMutex.lock();
    m_config.saveFallbackPath(path);
    m_config.updateConfigStore();
    m_defaultConfig.saveFallbackPath(path);
    m_objectMutex.unlock();
}

}//end of namespace TTS
