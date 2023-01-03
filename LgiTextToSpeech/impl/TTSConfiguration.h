#ifndef _TTS_CONFIG_H_
#define _TTS_CONFIG_H_
#include <string>
#include <mutex>
#include <map>

struct FallbackData
{
    std::string scenario;
    std::string value;
    std::string path;
};
namespace TTS {


class TTSConfiguration {
public:
    bool setEndPoint(const std::string endpoint) {return true;};
    bool setSecureEndPoint(const std::string endpoint) {return true;};
    bool setApiKey(const std::string apikey) {return true;};
    bool setLanguage(const std::string language) {return true;};
    bool setVoice(const std::string voice) {return true;};
    bool setEnabled(const bool dnabled) {return true;};
    bool setVolume(const double volume) {return true;};
    bool setRate(const uint8_t rate) {return true;};
    bool setPrimVolDuck(const int8_t primvolduck) {return true;};
   
    bool isFallbackEnabled() {return true;};
    void saveFallbackPath(std::string){};
    const std::string getFallbackScenario(){return "";};
    const std::string getFallbackPath(){return "";};
    const std::string getFallbackValue(){return "";};
    bool setFallBackText(FallbackData &fd){return true;};
    void setPreemptiveSpeak(const bool preemptive){};

    const std::string &endPoint() { return m_ttsEndPoint; }
    const std::string &secureEndPoint() { return m_ttsEndPointSecured; }
    const std::string &apiKey() { return m_apiKey; }
    const std::string &language() { return m_language; }
    const double &volume() { return m_volume; }
    const uint8_t &rate() { return m_rate; }
    const int8_t &primVolDuck() { return m_primVolDuck; }
    const bool enabled() { return m_enabled; }
    bool isPreemptive() { return m_preemptiveSpeaking; }
    bool loadFromConfigStore(){return true;};
    bool updateConfigStore(){return true;};
    const std::string voice(){return "";};

    bool updateWith(TTSConfiguration &config){(void) config; return true;};
    bool isValid(){return true;};

    static std::map<std::string, std::string> m_others;

private:
    std::string m_ttsEndPoint;
    std::string m_ttsEndPointSecured;
    std::string m_apiKey;
    std::string m_language;
    std::string m_voice;
    double m_volume{};
    uint8_t m_rate{};
    int8_t m_primVolDuck{};
    bool m_preemptiveSpeaking{};
    bool m_enabled{};
    bool m_fallbackenabled{};
    FallbackData m_data{};
    std::mutex m_mutex;
};

}//end of TTS namespace
#endif
