#ifndef _TTS_DOWNLOADER_H_
#define _TTS_DOWNLOADER_H_
#include "TTSCommon.h"
#include "TTSConfiguration.h"
#include <string>
#include <mutex>
#include <atomic>
#include <thread>
#include <condition_variable>

namespace TTS
{

class TTSDownloader
{
   public:
   ~TTSDownloader();
   TTSDownloader(TTSConfiguration &config);
   void download(TTSConfiguration config);
   void downloadThread();
   void downloadFile(std::string tts_request);
   void saveConfiguration(std::string path);   

   private:
   TTSConfiguration &m_defaultConfig;
   TTSConfiguration m_config;
   std::thread *m_downloadThread;
   std::atomic<bool> active;
   std::atomic<bool> needDownload;
   std::mutex m_queueMutex;
   std::mutex m_objectMutex;
   std::condition_variable m_condition;
};

}
#endif
