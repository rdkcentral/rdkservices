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
#pragma once

#include <string>
#include <functional>
#include <pthread.h>

struct Response
{
  uint32_t channel_id;
  std::string json;

  Response();
  Response(uint32_t channel, const std::string& json_str);
};

class SocketServer
{
public:
  SocketServer();
  ~SocketServer();
  int Open(char const* host_ip, int port, const std::function<void (const Response&)>& reader);
  int RunThread();
  int Run();
  void Close();
  int SendInvoke(uint32_t channel_id, const std::string& token, const std::string& json);
  int SendAttach(uint32_t channel_id, bool attach);
  int SendExit();
  int ReadResponse(Response& cmd);
private:
  int ReadExact(uint8_t* p, int len);
  static void* RunThreadFunc(void* arg);

  int m_serverSocket;
  int m_clientSocket;  
  std::function<void (const Response&)> m_reader;
  bool m_running;
  pthread_t m_socketThread;
};
