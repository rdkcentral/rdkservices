#ifndef AUDIO_PLAYER
#define AUDIO_PLAYER
#include <atomic>
#include <gst/gst.h>
#include <gst/audio/audio.h>
#include <string>
#include "BufferQueue.h"
#include "IWebSocketClient.h"
#include "SecurityParameters.h"
#include "SoC_abstraction.h"
#include <condition_variable>
#include <mutex>
#include <thread>
#include <vector>

//Default Values
// in Percentage
#define DEFAULT_PRIM_VOL_LEVEL 25
#define MAX_PRIM_VOL_LEVEL 100
#define DEFAULT_PLAYER_VOL_LEVEL 100
class SAPEventCallback {
public:
    SAPEventCallback() {}
    virtual ~SAPEventCallback() {}
    virtual void onSAPEvent(uint32_t id,std::string message) { (void)id; }
};

enum AudioType
{
    AudioType_None,
    PCM,
    MP3,
    WAV
};

enum SourceType
{
    SourceType_None,
    DATA,
    HTTPSRC,
    FILESRC,
    WEBSOCKET
};

enum WSStatus
{
    CONNECTED,
    DISCONNECTED,
    NETWORKERROR
};

enum PlayMode
{
    PlayMode_None,
    SYSTEM,
    APP
};

enum State
{
    READY,
    PAUSED,
    PLAYING,
    PLAYBACKERROR
};

class AudioPlayer
{
    private:
    GstElement  *m_pipeline;
    GstElement  *m_audioSink;
    GstElement  *m_audioVolume;
    GstElement  *m_capsfilter;
    GstElement  *m_audioCutter;
    int m_primVolume;
    int m_prevPrimVolume; 
    int m_thisVolume; //Player Volume
    int m_prevThisVolume;
    double m_thresHold;
    double m_thresHold_dB;
    int m_detectTimeMs;
    int m_holdTimeMs;
    int m_duckPercent;
    static GMainLoop   *m_main_loop;
    static GThread     *m_main_loop_thread;
    static SAPEventCallback *m_callback;
    int objectIdentifier;
    std::atomic<bool> m_isPaused;
    bool m_running;
    std::atomic<bool> appsrc_firstpacket;    
    std::mutex m_queueMutex;
    std::mutex m_playMutex;
    std::mutex m_apiMutex;
    std::condition_variable m_condition;
    std::string m_url;
    guint       m_busWatch;  
    gint64      m_duration;
    std::thread *m_thread;
    impl::WebSocketClientPtr webClient;
    impl::SecurityParameters m_secParams;
    std::atomic_bool m_fallbackToUnsecuredConnection{false};
    BufferQueue *bufferQueue;
    GstElement  *m_source;
    AudioType audioType;
    SourceType sourceType;
    PlayMode playMode;
    WSStatus wsStatus;
    std::atomic<State> state;
    //PCM audio caps
    std::string m_PCMFormat;
    std::string m_Layout;
    int  m_Rate;
    int  m_Channels;
    void createPipeline(bool smartVolumeControl);    
    void resetPipeline();
    void resetPipelineForSmartVolumeControl(bool smartVolumeEnable);
    void destroyPipeline();
    void setVolume( int Vol);
    void setPrimaryVolume( int Vol);
    void setDetectTime( int detectTime);
    void setHoldTime( int holdTime);
    void setThreshold( double thresHold);
    void setThresholdDB( double thresHold_dB);
    bool waitForStatus(GstState expected_state, uint32_t timeout_ms);
    GstCaps * getPCMAudioCaps( const std::string format, int rate, int channels, const std::string layout);

    public:
    AudioPlayer() {}
    AudioPlayer(AudioType,SourceType,PlayMode,int objectIdentifier);
    ~AudioPlayer();
    void Play(std::string url);
    void PlayBuffer(const char*,int);
    bool Resume();
    bool Pause();
    void Stop();
    void SetMixerLevels(int primVol, int thisVol);
    void SetSmartVolControl(bool smartVolumeEnable,double threshold, int detectTimeMs, int  holdTimeMs, int duckPercent);
    AudioType getAudioType();
    PlayMode  getPlayMode();
    SourceType getSourceType();
    void push_data(const void *ptr,int length);
    void wsConnectionStatus(WSStatus status);
    bool handleMessage(GstMessage*);
    gboolean PushDataAppSrc();
    static void Init(SAPEventCallback *callback);
    static void DeInit();
    static int GstBusCallback(GstBus *bus, GstMessage *message, gpointer data); 
    static void event_loop();
    int getObjectIdentifier();
    std::string getUrl();
    bool isPlaying();
    bool configPCMCaps(const std::string format, int rate, int channels, const std::string layout);
    void configWsSecParams(const impl::SecurityParameters& secParams);
};
#endif
