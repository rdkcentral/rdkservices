#ifndef _TTS_URLCONSTRUCTER_H_
#define _TTS_URLCONSTRUCTER_H_
#include "TTSCommon.h"
#include "TTSConfiguration.h"
#include <string>
#include <mutex>
#include <thread>
#include <condition_variable>

namespace TTS
{

class TTSURLConstructer
{
   public:
   ~TTSURLConstructer();
   TTSURLConstructer();
   std::string constructURL(TTSConfiguration &config,std::string text,bool isfallback);
   void sanitizeString(const std::string &input, std::string &sanitizedString);
   bool isSilentPunctuation(const char c);
   void replaceSuccesivePunctuation(std::string& subject);
   void replaceIfIsolated(std::string& subject, const std::string& search, const std::string& replace);
   void curlSanitize(std::string &url);

   private:
   //TTSConfiguration &m_defaultConfig;
   
};

}
#endif

