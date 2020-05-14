/**
* If not stated otherwise in this file or this component's LICENSE
* file the following copyright and licenses apply:
*
* Copyright 2020 RDK Management
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
**/

#ifndef _socket_adaptor_H_
#define _socket_adaptor_H_
#include <fstream>
#include <string>
#include <thread>
#include <mutex>
#include <vector>
#include <syscall.h>

#define SA_INFO(fmt, ...) do { fprintf(stderr, "[%d] SA_INFO [%s:%d] %s: " fmt "\n", (int)syscall(SYS_gettid), __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__); fflush(stderr); } while (0)
#define SA_WARN(fmt, ...) do { fprintf(stderr, "[%d] SA_WARN [%s:%d] %s: " fmt "\n", (int)syscall(SYS_gettid), __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__); fflush(stderr); } while (0)
#define SA_ERR(fmt, ...) do { fprintf(stderr, "[%d] SA_ERROR [%s:%d] %s: " fmt "\n", (int)syscall(SYS_gettid), __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__); fflush(stderr); } while (0)

#ifndef REPORT_IF_UNEQUAL
#define REPORT_IF_UNEQUAL(lhs, rhs) do { \
    if((lhs) != (rhs)) SA_ERR("Unexpected error!\n");}while(0);
#endif

#define UNUSED(expr)(void)(expr)

typedef void (*socket_adaptor_cb_t)(void * data);

class socket_adaptor
{
	public:
	typedef enum
	{
		EXIT = 0,
		NEW_CALLBACK,
		CODE_MAX
	} control_code_t;

	private:
	std::string m_path;
	int m_listen_fd;
	int m_write_fd;
	int m_read_fd;
	int m_control_pipe[2];
    std::vector<unsigned char> m_fetch_buffer;
	unsigned int m_num_connections;
	std::thread m_thread;
	std::mutex m_mutex;
	socket_adaptor_cb_t m_callback;
	void * m_callback_data;

	void process_new_connection();
	void process_control_message(control_code_t message);
	int stop_listening();
	void lock();
	void unlock();
	void worker_thread();

	public:
	socket_adaptor();
	~socket_adaptor();
    /**
     *  @brief This api connects to the socket for reading data
     *
     * @param[in] path  socket to connect to.
     *
     *  @return Returns 0 on success or -1 o
     */
    int connect_socket(const std::string &path);

    /**
     *  @brief This function makes the audiocapturemgr listen for incoming unix domain connections to the given path.
     *
     *  @param[in] path  binding path.
     *
     *  @return Returns 0 on success, appropiate error code otherwise.
     */
	int start_listening(const std::string &path);

    /**
     *  @brief This api returns the data path.
     *
     *  It is the path of unix domain server that ip-out clients must connect to in order to receive real-time audio data.
     *
     *  @return Returns the data path in string.
     */
	std::string& get_path();

    /**
     *  @brief This api invokes unix write() to write data to the socket.
     *
     *  @param[in] buffer Data buffer.
     *  @param[in] size   Size of the buffer
     *
     *  @return Returns 0 on success, appropiate errorcode otherwise.
     */
	int write_data(const char * buffer, const unsigned int size);

    /**
     *  @brief This api invokes unix read() to read all data from the socket into the internal buffer
     *
     *
     *  @return Returns length of the data or -1 in case of an error
     */
    unsigned int fetch_data();

    /**
     *  @brief This api provides the previously fetched data
     *
     *
     *  @return void
     */
    void get_data(std::vector<unsigned char>& data);

    /**
    *  @brief This api provides the previously fetched data
     *
     *
     *  @return Returns 0 in case of success, positive number of bytes left in case of the supplied buffer is less than all data or -1 in case of an error
     */
    unsigned int get_data(char * buffer, const unsigned int size);

    /**
     *  @brief This api invokes  close() to terminate the current connection.
     */
	void terminate_current_connection(); //handy to simulate EOS at the other end.

    /**
     *  @brief This api returns the number of active connections.
     *
     *  @return Returns the number of active connections.
     */
	unsigned int get_active_connections();
	void register_data_ready_callback(socket_adaptor_cb_t cb, void *data);
};
#endif //_socket_adaptor_H_
