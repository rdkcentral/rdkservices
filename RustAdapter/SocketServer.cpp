/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2022 RDK Management
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "SocketServer.h"
#include "Logger.h"
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

enum CommandID
{
  ID_INVOKE = 1,
  ID_ATTACH,
  ID_EXIT
};

#pragma pack(push,1)
struct InvoketHeader
{
  uint32_t command_id;
  uint32_t channel_id;
  uint32_t token_len;
  uint32_t json_len;
};
#pragma pack(pop) 

#pragma pack(push,1)
struct AttachHeader
{
  uint32_t command_id;
  uint32_t channel_id;
  uint8_t attach;
};
#pragma pack(pop)

#pragma pack(push,1)
struct ResponseHeader
{
  uint32_t channel_id;
  uint32_t json_len;
};
#pragma pack(pop) 

Response::Response()
: channel_id(0), json()
{
}

Response::Response(uint32_t channel, const string& json_str)
: channel_id(channel), json(json_str)
{
}

#define CLIENT_DISCONNECT -2

SocketServer::SocketServer()
: m_serverSocket(0)
, m_clientSocket(0)
, m_running(false)
, m_address()
, m_port(0)
{

}

SocketServer::~SocketServer()
{
  Close();
}

int SocketServer::Open(const string& address, int port, const function<void (const Response&)>& reader)
{
  int sock;
  struct sockaddr_in addr;
  int rc;

  sock = socket(AF_INET, SOCK_STREAM, 0);

  if (sock < 0)
  {
    LOGERR("SocketServer::Open socket create failed: %s", strerror(errno));
    return -1;
  }

  memset(&addr, 0, sizeof(address));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  
  rc = inet_pton(AF_INET, address.c_str(), &addr.sin_addr);

  if (rc == 0)
  {
    LOGERR("SocketServer::Open inet_pton %s invalid ip format", address.c_str());
    return -1;
  }
  else if (rc < 0)
  {
    LOGERR("SocketServer::Open inet_pton %s failed: %s", address.c_str(), strerror(errno));
    return -1;
  }
  
  LOGDBG("SocketServer::Open host_ip=%s sin_addr=%d (note that INADDR_ANY=%d)", address.c_str(), addr.sin_addr.s_addr, INADDR_ANY);

  int sock_flags = fcntl (sock, F_GETFD, 0);
  if (sock_flags < 0)
  {
    LOGERR("SocketServer::Open fcntl get failed: %s", strerror(errno));
    return -1;
  }

  sock_flags |= FD_CLOEXEC;

  if (fcntl(sock, F_SETFD, sock_flags) < 0)
  {
    LOGERR("SocketServer::Open fcntl set failed: %s", strerror(errno));
    return -1;
  }

  if (bind(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0)
  {
    LOGERR("SocketServer::Open bind failed: %s", strerror(errno));
    return -1;
  }

  if (listen(sock, 4) < 0)
  {
    LOGERR("SocketServer::Open listen failed: %s", strerror(errno));
    return -1;
  }

  m_reader = reader;
  m_serverSocket = sock;
  m_address = address;

  if (port)
  {
    m_port = port;
  }
  else
  {
    struct sockaddr_in cur;
    socklen_t len = sizeof(cur);
    if (getsockname(sock, (struct sockaddr *)&cur, &len) ==-0)
    {
      m_port = ntohs(cur.sin_port);
    }
    else
    {
      LOGERR("SocketServer::Open getsockname failed: %s", strerror(errno));
    }
  }

  LOGDBG("SocketServer::Open successfully running on port %d", m_port);
  return 0;
}

int SocketServer::RunThread()
{
  if (pthread_create(&m_thread, nullptr, SocketServer::RunThreadFunc, this) != 0)
  {
    LOGERR("SocketServer::Open failed to create thread: %s",strerror(errno));
    return -1;
  }
  return 0;
}

void* SocketServer::RunThreadFunc(void* arg)
{
  SocketServer* ss = (SocketServer*)arg;
  ss->Run();
  return nullptr;
}

int SocketServer::Run()
{
  if (m_serverSocket <= 0 || m_running)
    return -1;

  m_running = true;

  while (m_running)
  {
    fd_set m_readSet;
    fd_set m_exceptSet;
    struct timeval tv;
    int res;

    FD_ZERO(&m_readSet);
    FD_ZERO(&m_exceptSet);
    FD_SET(m_serverSocket, &m_readSet);
    FD_SET(m_serverSocket, &m_exceptSet);    
    if (m_clientSocket)
    {
      FD_SET(m_clientSocket, &m_readSet);
      FD_SET(m_clientSocket, &m_exceptSet);    
    }

    tv.tv_sec = 10;
    tv.tv_usec = 0;
    res = select((m_clientSocket > m_serverSocket ? m_clientSocket : m_serverSocket) + 1, 
                 &m_readSet, NULL, &m_exceptSet, &tv);

    if (!m_running)
      break;

    if (res < 0)
    {
      LOGERR("SocketServer::Open select failed: %s", strerror(errno));
      return -1;
    }

    if (res == 0)
      continue;

    if (FD_ISSET(m_serverSocket, &m_readSet))
    {
      int clsock;
      struct sockaddr_in claddr;
      socklen_t cllen;
      
      cllen = sizeof(claddr);
      memset(&claddr, 0, cllen);
      
      clsock = accept(m_serverSocket, (struct sockaddr*)&claddr, &cllen);
      if (clsock < 0)
      {
        LOGERR("SocketServer::Open accept failed: %s", strerror(errno));
      }
      else
      {
        LOGDBG("SocketServer::Open client connected");
        m_clientSocket = clsock;
      }
    }

    if (m_clientSocket)
    {
      if (FD_ISSET(m_clientSocket, &m_readSet))
      {
        Response rsp;
        int rc = ReadResponse(rsp);
        if (rc >= 0)
        {
          m_reader(rsp);
        }
        else if (rc == CLIENT_DISCONNECT)
        {
          LOGDBG("SocketServer::Open client disconnected");
          close(m_clientSocket);
          m_clientSocket = 0;
        }
      }
    }
  }
  return 0;
}

string SocketServer::GetAddress() const
{
  return m_address;
}

int SocketServer::GetPort() const
{
  return m_port; 
}

void SocketServer::Close()
{
  if (m_serverSocket > 0)
  {
    if (m_running)
    {
      m_running = false;
    }
    close(m_clientSocket);
    close(m_serverSocket);
    m_serverSocket = 0;
  }
}

int SocketServer::ReadExact(uint8_t* p, int len)
{
  if (m_serverSocket <= 0 || m_clientSocket <= 0)
    return -1;

  int remaining = len;

  while (remaining > 0)
  {
    int num = (int)read(m_clientSocket, p, remaining);

    if (num <= 0)
    {
        LOGERR("SocketServer::ReadExact failed: num=%d", num);

        if (num == 0)
          return CLIENT_DISCONNECT;

        return -1;
    }
    else
    {
      p += num;
      remaining -= num;
    }
  }

  if (remaining != 0)
  {
    LOGERR("SocketServer::ReadExact: failed: remaining=%d", remaining);
    return -1;
  }

  return len;
}

int SocketServer::SendInvoke(uint32_t channel_id, const string& token, const string& json)
{
  if (m_serverSocket <= 0 || m_clientSocket <= 0)
    return -1;
  
  InvoketHeader header = {
    htonl(ID_INVOKE),
    htonl(channel_id),
    htonl((uint32_t)token.length()),
    htonl((uint32_t)json.length())
  };

  int num = send(m_clientSocket, &header, sizeof(header), MSG_NOSIGNAL);
  if (num != sizeof(header))
  {
    LOGERR("SocketServer::SendInvoke failed to send header: num=%d", num);
    return -1;
  }

  if (token.length() > 0)
  {
    num = send(m_clientSocket, token.c_str(), token.length(), MSG_NOSIGNAL);
    if (num != (ssize_t)token.length())
    {
      LOGERR("SocketServer::SendInvoke failed to send token: num=%d", num);
      return -1;
    }
  }

  if (json.length() > 0)
  {
    num = send(m_clientSocket, json.c_str(), json.length(), MSG_NOSIGNAL);
    if (num != (ssize_t)json.length())
    {
      LOGERR("SocketServer::SendInvoke failed to send json: num=%d", num);
      return -1;
    }
  }
  return 0;
}

int SocketServer::SendAttach(uint32_t channel_id, bool attach)
{
  if (m_serverSocket <= 0 || m_clientSocket <= 0)
    return -1;

  AttachHeader header = {
    htonl(ID_ATTACH),
    htonl(channel_id),
    attach ? (uint8_t)1 : (uint8_t)0
  };

  int num = send(m_clientSocket, &header, sizeof(header), MSG_NOSIGNAL);
  if (num != sizeof(header))
  {
    LOGERR("SocketServer::SendAttach failed to send header: num=%d", num);
    return -1;
  }
  return 0;
}

int SocketServer::SendExit()
{
  if (m_serverSocket <= 0 || m_clientSocket <= 0)
    return -1;

  uint32_t command_id = htonl(ID_EXIT);
  int num = send(m_clientSocket, &command_id, sizeof(command_id), MSG_NOSIGNAL);
  if (num != sizeof(command_id))
  {
    LOGERR("SocketServer::SendExit failed: num=%d", num);
    return -1;
  }

  /*break out of reader thread*/
  m_running = false;

  return 0;
}

int SocketServer::ReadResponse(Response& rsp)
{
  if (m_serverSocket <= 0 || m_clientSocket <= 0)
    return -1;

  ResponseHeader header;
  int rc;

  if ((rc = ReadExact((uint8_t*)&header, (int)sizeof(ResponseHeader))) <= 0)
  {
    return rc;
  }

  rsp.channel_id = ntohl(header.channel_id);
  header.json_len = ntohl(header.json_len);

  LOGDBG("SocketServer::ReadResponse read header: channel_id=%u, json_len=%u", 
    rsp.channel_id, header.json_len);

  if (header.json_len > 0)
  {
    uint8_t* buffer;
    buffer = (uint8_t*)malloc(header.json_len+1);

    if ((rc = ReadExact(buffer, (int)header.json_len)) <= 0)
    {
      free(buffer);
      return rc;
    }
    
    buffer[header.json_len] = 0;
    rsp.json = (char*)buffer;
    free(buffer);
  }
  return 0;
}
