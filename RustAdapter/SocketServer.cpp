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

Response::Response(uint32_t channel, const std::string& json_str)
: channel_id(channel), json(json_str)
{
}

class SocketMonitor
{
public:
  SocketMonitor();
  ~SocketMonitor();
  void Set(int fd);
  bool IsReadSet(int fd);
  bool IsExceptSet(int fd);
  void Clear(int fd);
  int Select(int timeout);

private:
    fd_set m_readSet;
    fd_set m_exceptSet;
    int m_fdMax;
};

SocketMonitor::SocketMonitor() 
: m_fdMax(0)
{
  FD_ZERO(&m_readSet);
  FD_ZERO(&m_exceptSet);
}

SocketMonitor::~SocketMonitor()
{

}

void SocketMonitor::Set(int fd)
{
  FD_SET(fd, &m_readSet);
  FD_SET(fd, &m_exceptSet);

  if (fd > m_fdMax)
    m_fdMax = fd;
}

bool SocketMonitor::IsReadSet(int fd)
{
  return FD_ISSET(fd, &m_readSet);
}

bool SocketMonitor::IsExceptSet(int fd)
{
  return FD_ISSET(fd, &m_exceptSet);
}

void SocketMonitor::Clear(int fd)
{
  FD_SET(fd, &m_readSet);
  FD_SET(fd, &m_exceptSet);
}

int SocketMonitor::Select(int timeout)
{
  struct timeval tv;
  
  tv.tv_sec = timeout;
  tv.tv_usec = 0;
  
  return select(m_fdMax + 1, &m_readSet, NULL, &m_exceptSet, &tv);
}

SocketServer::SocketServer()
: m_serverSocket(0)
, m_clientSocket(0)
, m_running(false)
{

}

SocketServer::~SocketServer()
{
  Close();
}

int SocketServer::Open(char const* host_ip, int port, const std::function<void (const Response&)>& reader)
{
  int sock;
  struct sockaddr_in addr;

  sock = socket(AF_INET, SOCK_STREAM, 0);

  if (sock < 0)
  {
    LOGERR("SocketServer::Open socket create failed: %s", strerror(errno));
    return -1;
  }

  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  inet_pton(AF_INET, host_ip, &addr.sin_addr);//FIXME
  addr.sin_addr.s_addr = INADDR_ANY;//FIXME

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

  LOGDBG("SocketServer::Open successfully running on port %d", port);

  return 0;
}

int SocketServer::RunThread()
{
  if (pthread_create(&m_socketThread, nullptr, SocketServer::RunThreadFunc, this) != 0)
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
    SocketMonitor mon;
    int res;

    mon.Set(m_serverSocket);
    mon.Set(m_clientSocket);

    res = mon.Select(10);

    if(!m_running)
      break;

    if (res < 0)
    {
      LOGERR("SocketServer::Open select failed: %s", strerror(errno));
      return -1;
    }

    if (res == 0)
      continue;

    if (mon.IsReadSet(m_serverSocket))
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
        LOGDBG("SocketServer::Open accepted new client");
        m_clientSocket = clsock;
      }
    }

    if (m_clientSocket)
    {
      if (mon.IsReadSet(m_clientSocket))
      {
        Response rsp;
        if (ReadResponse(rsp) >= 0)
        {
          m_reader(rsp);
        }
      }
    }
  }
  return 0;
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

int SocketServer::SendInvoke(uint32_t channel_id, const std::string& token, const std::string& json)
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

  if (ReadExact((uint8_t*)&header, (int)sizeof(ResponseHeader)) <= 0)
  {
    return -1;
  }

  rsp.channel_id = ntohl(header.channel_id);
  header.json_len = ntohl(header.json_len);

  LOGDBG("SocketServer::ReadResponse read header: channel_id=%u, json_len=%u", 
    rsp.channel_id, header.json_len);

  if (header.json_len > 0)
  {
    uint8_t* buffer;
    buffer = (uint8_t*)malloc(header.json_len+1);

    if (ReadExact(buffer, (int)header.json_len) <= 0)
    {
      free(buffer);
      return -1;
    }
    
    buffer[header.json_len] = 0;
    rsp.json = (char*)buffer;
    free(buffer);
  }
  return 0;
}
