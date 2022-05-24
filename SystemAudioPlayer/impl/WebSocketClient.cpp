#include "AudioPlayer.h"
#include <pthread.h>
#include <assert.h>
#include <cstring>
#include <chrono>

#define MAX_RX_BUFFER_BYTES (10*1024*1024)

static WebSocketClient *currentClientWorker = nullptr;

static void *WebsocketService(void *arg)
{
    WebSocketClient *wsWorker = (WebSocketClient *)arg;

    wsWorker->runWebsocketService();
    return NULL;
}

static int WebsocketEventCallback( struct lws *wsi,
    enum lws_callback_reasons reason,
    void *user,
    void *inData,
    size_t dataLen)
{
    int retVal = 0;
    switch (reason)
    {
    case LWS_CALLBACK_CLIENT_ESTABLISHED:
        SAPLOG_INFO("SAP: Websocket Connection Established");
        if (currentClientWorker)
        {
            currentClientWorker->onConnected(true);
        }
        break;

    case LWS_CALLBACK_CLOSED:
        SAPLOG_INFO("SAP: Websocket Connection Closed\n");
        if (currentClientWorker)
        {
            currentClientWorker->onConnected(false);
        }
        break;

    case LWS_CALLBACK_CLIENT_CONNECTION_ERROR:
        SAPLOG_INFO("SAP: Websocket Connection Error");
        if (currentClientWorker)
        {
            currentClientWorker->onConnectionError();
        }
        break;

    case LWS_CALLBACK_CLIENT_RECEIVE:
        if (currentClientWorker)
        {
            currentClientWorker->onNewData(inData, dataLen);
        }
        break;

    case LWS_CALLBACK_CLIENT_WRITEABLE:
        break;

    default:
        break;
    }
    return retVal;
}


WebSocketClient::WebSocketClient(AudioPlayer *player)
    : m_player(player), m_webSocket(nullptr), m_webSocketProtocols(nullptr),
      m_webSocketContext(nullptr), m_remotePort(40001),
      m_wsThreadID(), m_wsThreadStarted(false), m_wsThreadRun(false),
      m_isConnected(false)
{    
    SAPLOG_INFO("SAP: Websocket Constructor invoked");
}

WebSocketClient::~WebSocketClient()
{     
    disconnect();
    SAPLOG_INFO("SAP: Websocket Destructor");
}

bool WebSocketClient::connect(const std::string &uri)
{
    if(m_isConnected)
    {
        SAPLOG_INFO("SAP: Websocket already connected.disconnect first\n");
        return true;
    }
    assert(!m_wsThreadStarted);
    urlParser(uri);
    createWebsocketContext();
    currentClientWorker = this;
    SAPLOG_INFO("SAP: Websocket address %s..port %d\n",m_ipString.c_str(),m_remotePort);

    if (!m_webSocketContext)
    {
        return false;
    }

    lws_client_connect_info clientInfo = {0};
    clientInfo.context = m_webSocketContext;
    clientInfo.address = m_ipString.c_str();
    clientInfo.port = m_remotePort;
    clientInfo.ssl_connection = false;
    clientInfo.host = lws_canonical_hostname(m_webSocketContext);
    clientInfo.path = "";
    clientInfo.origin = "origin";
    clientInfo.protocol = m_webSocketProtocols[0].name;
    lws *wsi = lws_client_connect_via_info(&clientInfo);
    if (!wsi)
    {
        SAPLOG_ERROR("SAP: Websocket WSI Error");
        return false;
    }

    assert(!m_wsThreadStarted);
    m_wsThreadRun = true;
    if (0 == pthread_create(&m_wsThreadID, NULL, &WebsocketService, this))
    {
        m_wsThreadStarted = true;
    }
    else
    {
        SAPLOG_ERROR("SAP: Failed to create WebsocketService thread\n");
    }
    return m_isConnected;
}

void WebSocketClient::disconnect()
{
    m_wsThreadRun = false;
    if (m_wsThreadStarted)
    {
        void *value_ptr = NULL;
        int rc = pthread_join(m_wsThreadID, &value_ptr);
        if (rc != 0)
        {
            SAPLOG_INFO("Thread returned %d(%s)", rc, std::strerror(rc));
        }
        m_wsThreadStarted = false;
    }
    m_isConnected = false;
}

bool WebSocketClient::isConnected() const
{
    return m_isConnected;
}

void WebSocketClient::urlParser(std::string input)
{
    std::string url;

    size_t found = input.find("://");
    if(found != std::string::npos)
        url = input.substr(found+3); //Remove protocol part
    else
        url = input;   //Input without protocol..we will still handle

    size_t found1 =url.find_first_of(":");
    m_ipString =url.substr(0,found1);

    if(found1 != std::string::npos)
    {
        size_t found2 = url.find_first_of("/");
        m_remotePort = std::stoi(url.substr(found1+1,found2-found1-1));
        return;
    }
    SAPLOG_INFO("SAP: Websocket using default port\n");
}

void WebSocketClient::runWebsocketService()
{
    SAPLOG_INFO("SAP: Websocket thread started");

    while(m_wsThreadRun)
    {
        lws_service(m_webSocketContext, 250);
    }

    if (m_webSocketContext)
    {
        lws_context_destroy(m_webSocketContext);
    }

    if(m_webSocketProtocols)
    {
        delete[] m_webSocketProtocols;
    }

    currentClientWorker = nullptr;
    SAPLOG_INFO("SAP: Websocket thread exited");
}

void WebSocketClient::onNewData(const void *ptr, size_t len) const
{
    m_player->push_data(ptr, len);
}

void WebSocketClient::onConnectionError()
{
    m_player->wsConnectionStatus(WSStatus::NETWORKERROR);
}

void WebSocketClient::onConnected(bool status)
{
    m_isConnected = status;
    if(m_isConnected)
    {
        SAPLOG_INFO("SAP: Websocket Connected");
        m_player->wsConnectionStatus(WSStatus::CONNECTED);
    }
    else
    {
        SAPLOG_INFO("SAP: Websocket Disconnected");
        m_player->wsConnectionStatus(WSStatus::DISCONNECTED);
    }     
}

void WebSocketClient::createWebsocketContext()
{
    //create protocol info
    m_webSocketProtocols = new lws_protocols[2];
    static const char *protocol = "ws";

    m_webSocketProtocols[0].name = protocol;

    m_webSocketProtocols[0].callback = &WebsocketEventCallback;
    m_webSocketProtocols[0].per_session_data_size = 0;
    //5MB for audio
    m_webSocketProtocols[0].rx_buffer_size = 5 * 1024 * 1024;

    // NULL terminator
    m_webSocketProtocols[1].name = NULL;
    m_webSocketProtocols[1].callback = NULL;
    m_webSocketProtocols[1].per_session_data_size = 0;
    m_webSocketProtocols[1].rx_buffer_size = 0;

    //create context
    lws_context_creation_info info = {0};

    info.port = CONTEXT_PORT_NO_LISTEN;
    info.protocols = m_webSocketProtocols;

    info.gid = -1;
    info.uid = -1;
    info.ka_time = 1;    //keep alive every 1sec
    info.ka_probes = 2;   //try 2 times
    info.ka_interval = 1; //with 1s interval
    info.timeout_secs = 4;

    m_webSocketContext = lws_create_context(&info);
    if (m_webSocketContext == NULL)
    {
        SAPLOG_ERROR("SAP: Websocket Context creation error\n");
        return;
    }
    SAPLOG_INFO("SAP: Context created\n");
}

