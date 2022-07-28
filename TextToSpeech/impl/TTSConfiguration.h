#ifndef _TTS_CONFIG_H_
#define _TTS_CONFIG_H_
#include <string>
#include <mutex>

struct FallbackData
{
    std::string scenario;
    std::string value;
    std::string path;
};
namespace TTS {


class TTSConfiguration {
public:
    TTSConfiguration();
    TTSConfiguration(TTSConfiguration& obj);
    TTSConfiguration& operator = (const TTSConfiguration& obj);
    ~TTSConfiguration();

    bool setEndPoint(const std::string endpoint);
    bool setSecureEndPoint(const std::string endpoint);
    bool setApiKey(const std::string apikey);
    bool setLanguage(const std::string language);
    bool setVoice(const std::string voice);
    bool setEnabled(const bool dnabled);
    bool setVolume(const double volume);
    bool setRate(const uint8_t rate);
   
    bool isFallbackEnabled();
    void saveFallbackPath(std::string);
    const std::string getFallbackScenario();
    const std::string getFallbackPath();
    const std::string getFallbackValue();
    bool setFallBackText(FallbackData &fd);
    void setPreemptiveSpeak(const bool preemptive);

    const std::string &endPoint() { return m_ttsEndPoint; }
    const std::string &secureEndPoint() { return m_ttsEndPointSecured; }
    const std::string &apiKey() { return m_apiKey; }
    const std::string &language() { return m_language; }
    const double &volume() { return m_volume; }
    const uint8_t &rate() { return m_rate; }
    const bool enabled() { return m_enabled; }
    bool isPreemptive() { return m_preemptiveSpeaking; }
    bool loadFromConfigStore();
    bool updateConfigStore();
    const std::string voice();

    bool updateWith(TTSConfiguration &config);
    bool isValid();

    static std::map<std::string, std::string> m_others;

private:
    std::string m_ttsEndPoint;
    std::string m_ttsEndPointSecured;
    std::string m_apiKey;
    std::string m_language;
    std::string m_voice;
    double m_volume;
    uint8_t m_rate;
    bool m_preemptiveSpeaking;
    bool m_enabled;
    bool m_fallbackenabled;
    FallbackData m_data;
    std::mutex m_mutex;
};

}//end of TTS namespace
#endif
