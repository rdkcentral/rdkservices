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

#include "socket_adaptor.h"
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <errno.h>

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <fcntl.h>
#include <unistd.h>

static const int PIPE_READ_FD = 0;
static const int PIPE_WRITE_FD = 1;
static const unsigned int MAX_CONNECTIONS = 1;

static bool g_one_time_init_complete = false;

socket_adaptor::socket_adaptor() : m_listen_fd(-1), m_write_fd(-1), m_read_fd(-1), m_num_connections(0), m_callback(nullptr)
{
	SA_INFO("Enter\n");
	if(!g_one_time_init_complete)
	{
		/*SIGPIPE must be ignored or process will exit when client closes connection*/
		struct sigaction sig_settings = { 0 };
		sig_settings.sa_handler = SIG_IGN;
		sigaction(SIGPIPE, &sig_settings, NULL);
		g_one_time_init_complete = true;
	}
	REPORT_IF_UNEQUAL(0, pipe2(m_control_pipe, O_NONBLOCK));
}

socket_adaptor::~socket_adaptor()
{
	SA_INFO("Enter\n");
	stop_listening();
	close(m_control_pipe[PIPE_WRITE_FD]);
	close(m_control_pipe[PIPE_READ_FD]);
}

int socket_adaptor::connect_socket(const std::string &path)
{
    int ret = 0;

    /*Open new UNIX socket to transfer data*/
    m_read_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if( m_read_fd != -1)
    {
        SA_INFO("Socket created.\n");
        struct sockaddr_un address;

        /* start with a clean address structure */
        memset(&address, 0, sizeof(struct sockaddr_un));
        address.sun_family = AF_UNIX;
        snprintf(address.sun_path, 108, "%s", path.c_str());
        if(connect(m_read_fd, (struct sockaddr *) &address, sizeof(struct sockaddr_un)) == 0)
        {
            SA_INFO("socket connected: %s\n", path.c_str());
        } else {
            SA_ERR("connect() failed\n");
            close(m_read_fd);
            ret = -1;
            return ret;

        }
    }
    else
    {
        SA_ERR("Could not create socket.\n");
        ret = -1;
    }
    return ret;
}

int socket_adaptor::write_data(const char * buffer, const unsigned int size)
{
	int ret = write(m_write_fd, buffer, size);
	if(0 > ret)
	{
		SA_WARN("Write SA_ERR! Closing socket. errno: 0x%x\n", errno);
		SA_ERR("socket_adaptor::data_callback() ");

		close(m_write_fd);
		lock();
		if(0 < m_write_fd)
		{
			m_write_fd = -1;
			m_num_connections--;
		}
		unlock();
	}
	else if((unsigned int)ret != size)
	{
		SA_WARN("Incomplete buffer write!\n");
	}
	return ret;
}

unsigned int socket_adaptor::fetch_data()
{
    unsigned int size_recv , total_size = 0, n = 0;
    unsigned short int CHUNK_SIZE = 4096;
    char chunk[CHUNK_SIZE];

    if(m_read_fd < 0) {
        SA_ERR("Unable to fetch data. Did you connect?");
        return -1;
    }

    m_fetch_buffer.clear();
    while(1)
    {
        memset(chunk ,0 , CHUNK_SIZE);
        if((size_recv =  read(m_read_fd , chunk , CHUNK_SIZE) ) <= 0)
        {
            break;
        }
        else
        {
            m_fetch_buffer.insert(m_fetch_buffer.end(), chunk, chunk + size_recv);
            total_size += size_recv;
            ++n;
        }
    }
    SA_WARN("%d bytes received in %u reads!\n", total_size, n);

    close(m_read_fd);
    lock();
    if(0 < m_read_fd)
    {
        m_read_fd = -1;
    }
    unlock();

    return total_size;
}

void socket_adaptor::get_data(std::vector<unsigned char>& data)
{
    if (m_fetch_buffer.empty())
    {
        if (fetch_data() < 0)
        {
            SA_WARN("Empty call.");
            return;
        }
    }
    data = m_fetch_buffer;
    m_fetch_buffer.clear();
}

unsigned int socket_adaptor::get_data(char * buffer, const unsigned int size)
{
    unsigned int total_size = m_fetch_buffer.size();
    unsigned int ret_size = 0;

    if (m_fetch_buffer.empty() || size == 0)
    {
        SA_WARN("Empty call. Forgot to fetch data?");
        return 0;
    }

    if (size < total_size)
    {
        ret_size = size;
        SA_WARN("Data won't fit in the supplied buffer, %d bytes lost\n", total_size - ret_size);
    } else {
        ret_size = total_size;
    }
    memcpy(buffer, m_fetch_buffer.data(), ret_size);
    m_fetch_buffer.clear();
    return ret_size;
}

std::string& socket_adaptor::get_path()
{
	return m_path;
}

int socket_adaptor::start_listening(const std::string &path)
{
	int ret = 0;
	m_path = path;

	/*Open new UNIX socket to transfer data*/
	m_listen_fd = socket(AF_UNIX, SOCK_STREAM | SOCK_NONBLOCK, 0);
	if(0 < m_listen_fd)
	{
		SA_INFO("Socket created.\n");

		for(int retries = 0; retries <  2; retries++) //If first attempt fails due to path in use, unlink it and try again.
		{
			struct sockaddr_un bind_path;
			bind_path.sun_family = AF_UNIX;
			memcpy(bind_path.sun_path, m_path.c_str(), sizeof(bind_path.sun_path));

			SA_INFO("Binding to path %s\n", bind_path.sun_path);
			ret = bind(m_listen_fd, (const struct sockaddr *) &bind_path, sizeof(bind_path));
			if(-1 == ret)
			{
				if(EADDRINUSE == errno)
				{
					SA_WARN("path is already in use. Using brute force.\n");
					unlink(m_path.c_str());
				}
				else
				{
					SA_ERR("Failed to bind to path. SA_ERR is %d\n", errno);
					SA_ERR("bind SA_ERR");
					close(m_listen_fd);
					m_listen_fd = -1;
					break;
				}
			}
			else
			{
				SA_INFO("Bound successfully to path.\n");
				REPORT_IF_UNEQUAL(0, listen(m_listen_fd, 3));
				m_thread = std::thread(&socket_adaptor::worker_thread, this);
				break;
			}
		}
	}
	else
	{
		SA_ERR("Could not open socket.\n");
		ret = -1;
	}
	return ret;
}

int socket_adaptor::stop_listening()
{
	SA_INFO("Enter\n");
	lock();

	/*Shut down worker thread that listens to incoming connections.*/
	if(m_thread.joinable())
	{
		int message = EXIT;
		int ret = write(m_control_pipe[PIPE_WRITE_FD], &message, sizeof(message));
		if(ret != sizeof(message))
		{
			SA_ERR("Couldn't trigger worker thread shutdown.\n");
		}
		else
		{
			SA_INFO("Waiting for worker thread to join.\n");
			unlock();
			m_thread.join();
			lock();
			SA_INFO("Worker thread has joined.\n");
		}
	}
	if(!m_path.empty())
	{
		if(0 < m_write_fd)
		{
			close(m_write_fd);
			m_write_fd = -1;
			m_num_connections--;
			SA_INFO("Closed write fd. Total active connections now is %d\n", m_num_connections);
		}
		SA_INFO("Removing named socket %s.\n", m_path.c_str());
		unlink(m_path.c_str());
		m_path.clear();
	}
	unlock();
	SA_INFO("Exit\n");
    return 0;
}

void socket_adaptor::process_new_connection()
{
	SA_INFO("Enter\n");
	lock();
	if(0 < m_write_fd)
	{
		SA_WARN("Trashing old write socket in favour of new one.\n");
		close(m_write_fd);
		m_write_fd = -1;
		m_num_connections--;
	}

	SA_INFO("Setting up a new connection.\n");
	struct sockaddr_un incoming_addr;
	int addrlen = sizeof(incoming_addr);
	m_write_fd = accept(m_listen_fd, (sockaddr *) &incoming_addr, (socklen_t *)&addrlen);

	if(0 > m_write_fd)
	{
		SA_ERR("SA_ERR accepting connection.\n");
	}
	else
	{
		m_num_connections++;
		SA_INFO("Connected to new client.\n");
	}
	unlock();

	if(m_callback)
	{
		m_callback(m_callback_data);
	}
	return;
}

void socket_adaptor::worker_thread()
{
	SA_INFO("Enter\n");
	int control_fd = m_control_pipe[PIPE_READ_FD];
	int max_fd = (m_listen_fd > control_fd ? m_listen_fd : control_fd);


	bool check_fds = true;

	while(check_fds)
	{
		fd_set poll_fd_set;
		FD_ZERO(&poll_fd_set);
		FD_SET(m_listen_fd, &poll_fd_set);
		FD_SET(control_fd, &poll_fd_set);

		int ret = select((max_fd + 1), &poll_fd_set, NULL, NULL, NULL);
		SA_INFO("Unblocking now. ret is 0x%x\n", ret);
		if(0 == ret)
		{
			SA_ERR("select() returned 0.\n");
		}
		else if(0 < ret)
		{
			//Some activity was detected. Process event further.
			if(0 != FD_ISSET(control_fd, &poll_fd_set))
			{
				control_code_t message;
				REPORT_IF_UNEQUAL((sizeof(message)), read(m_control_pipe[PIPE_READ_FD], &message, sizeof(message)));
				if(EXIT == message)
				{
					SA_INFO("Exiting monitor thread.\n");
					break;
				}
				else
				{
					process_control_message(message);
				}
			}
			if(0 != FD_ISSET(m_listen_fd, &poll_fd_set))
			{
				process_new_connection();
			}
		}
		else
		{
			SA_ERR("SA_ERR polling monitor FD!\n");
			break;
		}
	}

	SA_INFO("Exit\n");
}

void socket_adaptor::terminate_current_connection()
{
	lock();
	if(0 < m_write_fd)
	{
		close(m_write_fd);
		m_write_fd = -1;
		m_num_connections--;
		SA_INFO("Terminated current connection.\n");
	}
	unlock();
}


void socket_adaptor::lock()
{
	m_mutex.lock();
}

void socket_adaptor::unlock()
{
	m_mutex.unlock();
}

void socket_adaptor::process_control_message(control_code_t message)
{
	if(NEW_CALLBACK == message)
	{
		lock();
		auto num_connections = m_num_connections;
		auto callback = m_callback;
		auto callback_data = m_callback_data;
		unlock();

		if(callback && (0 != num_connections))
		{
			callback(callback_data);
		}
	}
}


unsigned int socket_adaptor::get_active_connections()
{
	lock();
	unsigned int num_conn = m_num_connections;
	unlock();
	return num_conn;
}

void socket_adaptor::register_data_ready_callback(socket_adaptor_cb_t callback, void *data)
{
	lock();
	m_callback = callback;
	m_callback_data = data;
	unlock();

	if(nullptr != callback)
	{
		control_code_t message = NEW_CALLBACK;
		int ret = write(m_control_pipe[PIPE_WRITE_FD], &message, sizeof(message));
		UNUSED(ret);
	}
}
