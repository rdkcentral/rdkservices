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
    std::string constructURL(TTSConfiguration &config ,std::string text, bool isFallback, bool isLocal);

    private:
    std::string httpgetURL(TTSConfiguration &config, std::string text, bool isFallback, bool isLocal);
    std::string httppostURL(TTSConfiguration &config, std::string text, bool isFallback);
    void sanitizeString(const std::string &input, std::string &sanitizedString);
    bool isSilentPunctuation(const char c);
    void replaceSuccesivePunctuation(std::string& subject);
    void replaceIfIsolated(std::string& subject, const std::string& search, const std::string& replace, bool skipIsolationCheck = false);
    void curlSanitize(std::string &url);
};

}
#endif

