#include "WebSocket.h"

#include <cstdlib>
#include <iostream>

namespace WPEFramework {
namespace Plugin {

    ConnectionMetaData::ConnectionMetaData(int id, websocketpp::connection_hdl connectionHandle, std::string uri)
      : mIdentifier(id)
      , mHandle(connectionHandle)
      , mStatus("Connecting")
      , mURI(uri)
      , mServerResponse("N/A")
    {
        sem_init(&mEventSem, 0, 0);
    }

    ConnectionMetaData::~ConnectionMetaData()
    {
        sem_destroy(&mEventSem);
    }

    void ConnectionMetaData::onOpen(WebSocketAsioClient * webSocketClient, websocketpp::connection_hdl connectionHandle)
    {
        mStatus = "Open";

        WebSocketAsioClient::connection_ptr clientConnection = webSocketClient->get_con_from_hdl(connectionHandle);
        mServerResponse = clientConnection->get_response_header("Server");
        sem_post(&mEventSem);
    }

    void ConnectionMetaData::onFail(WebSocketAsioClient * webSocketClient, websocketpp::connection_hdl connectionHandle)
    {
        mStatus = "Failed";

        WebSocketAsioClient::connection_ptr clientConnection = webSocketClient->get_con_from_hdl(connectionHandle);
        mServerResponse = clientConnection->get_response_header("Server");
        mErrorReason = clientConnection->get_ec().message();
        sem_post(&mEventSem);
    }
    
    void ConnectionMetaData::onClose(WebSocketAsioClient * webSocketClient, websocketpp::connection_hdl connectionHandle)
    {
        mStatus = "Closed";
        WebSocketAsioClient::connection_ptr clientConnection = webSocketClient->get_con_from_hdl(connectionHandle);
        std::stringstream errorReason;
        errorReason << "close code: " << clientConnection->get_remote_close_code() << " (" 
          << websocketpp::close::status::get_string(clientConnection->get_remote_close_code()) 
          << "), close reason: " << clientConnection->get_remote_close_reason();
        mErrorReason = errorReason.str();
        sem_post(&mEventSem);
    }

    void ConnectionMetaData::onMessage(websocketpp::connection_hdl connectionHandle, WebSocketAsioClient::message_ptr serverMessage)
    {
        mStatus = "Response";
        if (serverMessage->get_opcode() == websocketpp::frame::opcode::text)
	{
            mLastMessage = serverMessage->get_payload();
        }
	else
	{
            mLastMessage = websocketpp::utility::to_hex(serverMessage->get_payload());
        }
        sem_post(&mEventSem);
    }

    websocketpp::connection_hdl ConnectionMetaData::getHandle() const
    {
        return mHandle;
    }
    
    int ConnectionMetaData::getIdentifier() const
    {
        return mIdentifier;
    }
    
    std::string ConnectionMetaData::getStatus() const
    {
        return mStatus;
    }

    std::string ConnectionMetaData::getLastMessage() const
    {
        return mLastMessage;
    }

    std::string ConnectionMetaData::getErrorReason() const
    {
        return mErrorReason;
    }

    std::string ConnectionMetaData::getURL() const
    {
        return mURI;
    }

    void ConnectionMetaData::waitForEvent()
    {
        sem_wait(&mEventSem);
    }

    WebSocketEndPoint::WebSocketEndPoint () : mNextIdentifier(0)
    {
    }

    WebSocketEndPoint::~WebSocketEndPoint()
    {
    }

    void WebSocketEndPoint::initialize()
    {
        mEndPoint.clear_access_channels(websocketpp::log::alevel::all);
        mEndPoint.clear_error_channels(websocketpp::log::elevel::all);

        mEndPoint.init_asio();
        mEndPoint.start_perpetual();

        mThread = websocketpp::lib::make_shared<websocketpp::lib::thread>(&WebSocketAsioClient::run, &mEndPoint);
    }

    void WebSocketEndPoint::deinitialize()
    {
        for (connectionList::const_iterator it = mConnectionList.begin(); it != mConnectionList.end(); ++it)
	{
            std::cout << "> Closing connection " << it->second->getIdentifier() << std::endl;
            
            websocketpp::lib::error_code ec;
            mEndPoint.close(it->second->getHandle(), websocketpp::close::status::going_away, "", ec);
            if (ec)
	    {
                std::cout << "> Error closing connection " << it->second->getIdentifier() << ": "  
                          << ec.message() << std::endl;
            }
        }
        mEndPoint.stop_perpetual();
        mThread->join();
    }

    int WebSocketEndPoint::connect(std::string const & uri, bool wait)
    {
        for (connectionList::const_iterator it = mConnectionList.begin(); it != mConnectionList.end(); ++it)
	{
            if (it->second->getURL() == uri)
            {
                std::cout << "REUSE CONNECTION FOR " << uri << std::endl;
                return it->second->getIdentifier();
            }
        }
        websocketpp::lib::error_code errorCode;

        WebSocketAsioClient::connection_ptr clientConnection = mEndPoint.get_connection(uri, errorCode);
        std::cout << "NEW CONNECTION FOR " << uri << std::endl;
        if (errorCode)
	{
            std::cout << "Failed to initialize connection. error: " << errorCode.message() << std::endl;
            return -1;
        }

        int newId = mNextIdentifier++;
        ConnectionMetaData::sharedPtr metaDataReference = websocketpp::lib::make_shared<ConnectionMetaData>(newId, clientConnection->get_handle(), uri);
        mConnectionList[newId] = metaDataReference;

        clientConnection->set_open_handler(websocketpp::lib::bind(
            &ConnectionMetaData::onOpen,
            metaDataReference,
            &mEndPoint,
            websocketpp::lib::placeholders::_1
        ));
        clientConnection->set_fail_handler(websocketpp::lib::bind(
            &ConnectionMetaData::onFail,
            metaDataReference,
            &mEndPoint,
            websocketpp::lib::placeholders::_1
        ));
        clientConnection->set_close_handler(websocketpp::lib::bind(
            &ConnectionMetaData::onClose,
            metaDataReference,
            &mEndPoint,
            websocketpp::lib::placeholders::_1
        ));
        clientConnection->set_message_handler(websocketpp::lib::bind(
            &ConnectionMetaData::onMessage,
            metaDataReference,
            websocketpp::lib::placeholders::_1,
            websocketpp::lib::placeholders::_2
        ));

        mEndPoint.connect(clientConnection);
        if (wait)
	{
            metaDataReference->waitForEvent();
	}
        if (metaDataReference->getStatus() != "Open")
	{
            std::cout << "> unable to establish connection " <<  metaDataReference->getErrorReason() << std::endl;
            mConnectionList.erase(newId);
            return -1;
        }
        return newId;
    }

    void WebSocketEndPoint::close(int id, websocketpp::close::status::value code)
    {
        websocketpp::lib::error_code ec;
        
        connectionList::iterator metaDataIterator = mConnectionList.find(id);
        if (metaDataIterator == mConnectionList.end())
	{
            std::cout << "> No connection found with id " << id << std::endl;
            return;
        }
        
        mEndPoint.close(metaDataIterator->second->getHandle(), code, "", ec);
        if (ec)
	{
            std::cout << "> Error initiating close: " << ec.message() << std::endl;
        }
        mConnectionList.erase(metaDataIterator);
    }

    bool WebSocketEndPoint::send(int id, std::string message, std::string& response)
    {
        websocketpp::lib::error_code ec;
        connectionList::iterator metaDataIterator = mConnectionList.find(id);
        if (metaDataIterator == mConnectionList.end())
	{
            std::cout << "> No connection found with id " << id << std::endl;
            return false;
        }
        
        if (metaDataIterator->second->getStatus() != "Open" && metaDataIterator->second->getStatus() != "Response")
	{
            std::cout << "> unable to send message as connection not open: " <<  std::endl;
            return false;
        }

        mEndPoint.send(metaDataIterator->second->getHandle(), message, websocketpp::frame::opcode::text, ec);
        if (ec)
	{
            std::cout << "> Error sending message: " << ec.message() << std::endl;
            return false;
        }

        metaDataIterator->second->waitForEvent();
        if (metaDataIterator->second->getStatus() != "Response")
	{
            std::cout << "> unable to receive message and state is  " <<  metaDataIterator->second->getStatus() << std::endl;
            return false;
        }
	response = metaDataIterator->second->getLastMessage();
        return true;
    }

    ConnectionMetaData::sharedPtr WebSocketEndPoint::getMetadata(int id) const
    {
        connectionList::const_iterator metaDataIterator = mConnectionList.find(id);
        if (metaDataIterator == mConnectionList.end())
	{
            return ConnectionMetaData::sharedPtr();
        }
	else
	{
            return metaDataIterator->second;
        }
    }

} // namespace Plugin
} // namespace WPEFramework
