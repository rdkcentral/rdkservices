#ifndef WEBSOCKETCLIENT_H_
#define WEBSOCKETCLIENT_H_

#include <libwebsockets.h>
#include <string>
#include <pthread.h>
#include <atomic>
#include "AudioPlayer.h"
#include "logger.h"

class AudioPlayer;

class WebSocketClient
{
    public:
    WebSocketClient(AudioPlayer *player);
    ~WebSocketClient();
    bool connect(const std::string &uri);
    void disconnect();
    bool isConnected() const;
    void onConnected(bool status);
    void onNewData(const void *ptr, size_t len) const;
    void onConnectionError();
    void runWebsocketService();

    private:    
    void createWebsocketContext();
    void urlParser(std::string input);
    lws *m_webSocket;
    lws_protocols *m_webSocketProtocols;
    lws_context *m_webSocketContext;
    std::string m_ipString;
    int m_remotePort;
    pthread_t m_wsThreadID;
    bool m_wsThreadStarted;
    std::atomic<bool> m_wsThreadRun;
    std::atomic<bool> m_isConnected;
    AudioPlayer *m_player;
};
#endif
