/*
 * If not stated otherwise in this file or this component's Licenses.txt file the
 * following copyright and licenses apply:
 *
 * Copyright 2023 RDK Management
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

#include "MiracastCommon.h"

MiracastThread::MiracastThread(std::string thread_name, size_t stack_size, size_t msg_size, size_t queue_depth, void (*callback)(void *), void *user_data)
{
    MIRACASTLOG_TRACE("Entering...");
    m_thread_name = thread_name;

    m_thread_stacksize = stack_size;
    m_thread_message_size = msg_size;
    m_thread_message_count = queue_depth;
    m_thread_user_data = user_data;
    m_thread_callback = callback;

    m_g_queue = nullptr;
    m_pthread_id = 0;

    if ((0 != queue_depth) && (0 != msg_size)){
        // Create message queue
        m_g_queue = g_async_queue_new();
        // g_async_queue_ref( g_queue );
        sem_init(&m_empty_msgq_sem_obj, 0, 0);
    }

    // Create thread
    pthread_attr_init(&m_pthread_attr);
    pthread_attr_setstacksize(&m_pthread_attr, m_thread_stacksize);
    MIRACASTLOG_TRACE("Exiting...");
}

MiracastThread::~MiracastThread()
{
    MIRACASTLOG_TRACE("Entering...");

    if ( 0 != m_pthread_id ){
        // Join thread
        pthread_join(m_pthread_id, nullptr);
        m_pthread_id = 0;
        pthread_attr_destroy(&m_pthread_attr);
    }

    if ( nullptr != m_g_queue ){
        // Close message queue
        g_async_queue_unref(m_g_queue);
        sem_destroy(&m_empty_msgq_sem_obj);
        m_g_queue = nullptr;
    }
    MIRACASTLOG_TRACE("Exiting...");
}

MiracastError MiracastThread::start(void)
{
    MiracastError ret_code = MIRACAST_OK;
    MIRACASTLOG_TRACE("Entering...");
    if ( 0 != pthread_create(   &m_pthread_id,
                                &m_pthread_attr,
                                reinterpret_cast<void *(*)(void *)>(m_thread_callback),
                                m_thread_user_data))
    {
        ret_code = MIRACAST_FAIL;
    }
    MIRACASTLOG_TRACE("Exiting...");
    return ret_code;
}

void MiracastThread::send_message(void *message, size_t msg_size)
{
    MIRACASTLOG_TRACE("Entering...");
    if (nullptr != m_g_queue){
        void *buffer = malloc(msg_size);
        if (nullptr == buffer)
        {
            MIRACASTLOG_ERROR("Memory Allocation Failed for %u\n", msg_size);
            MIRACASTLOG_TRACE("Exiting...");
            return;
        }
        memset(buffer, 0x00, msg_size);
        // Send message to queue

        memcpy(buffer, message, msg_size);
        g_async_queue_push(m_g_queue, buffer);
        sem_post(&m_empty_msgq_sem_obj);
    }
    MIRACASTLOG_TRACE("Exiting...");
}

int8_t MiracastThread::receive_message(void *message, size_t msg_size, int sem_wait_timedout)
{
    int8_t status = false;
    MIRACASTLOG_TRACE("Entering...");
    if (nullptr != m_g_queue){
        if (( THREAD_RECV_MSG_WAIT_IMMEDIATE == sem_wait_timedout )||
            (THREAD_RECV_MSG_INDEFINITE_WAIT == sem_wait_timedout))
        {
            int count = 0;
	        sem_getvalue(&m_empty_msgq_sem_obj,&count);
            if ((0 < count ) || (THREAD_RECV_MSG_INDEFINITE_WAIT == sem_wait_timedout)){
                sem_wait(&m_empty_msgq_sem_obj);
                status = true;
            }
        }
        else if (0 < sem_wait_timedout)
        {
            struct timespec ts;
            clock_gettime(CLOCK_REALTIME, &ts);
            ts.tv_sec += sem_wait_timedout;

            if (-1 != sem_timedwait(&m_empty_msgq_sem_obj, &ts))
            {
                status = true;
            }
        }
        else
        {
            status = -1;
        }

        if (true == status)
        {
            void *data_ptr = static_cast<void *>(g_async_queue_pop(m_g_queue));
            if ((nullptr != message) && (nullptr != data_ptr))
            {
                memcpy(message, data_ptr, msg_size);
                free(data_ptr);
            }
        }
    }
    MIRACASTLOG_TRACE("Exiting...");
    return status;
}

std::string MiracastCommon::parse_opt_flag( std::string file_name , bool integer_check , bool debugStats )
{
    std::string return_buffer = "";
    std::ifstream parse_opt_flag_file( file_name.c_str());

    if (!parse_opt_flag_file)
    {
        if ( debugStats ){
            MIRACASTLOG_WARNING("Failed to open [%s] file",file_name.c_str());
        }
    }
    else
    {
        std::string line = "";
        if (std::getline(parse_opt_flag_file, line))
        {
            if ( debugStats ){
                MIRACASTLOG_INFO("Content in [%s] is [%s]",file_name.c_str(),line.c_str());
            }
        }
        else
        {
            if ( debugStats ){
                MIRACASTLOG_WARNING("No Content in [%s]",file_name.c_str());
            }
        }
        parse_opt_flag_file.close();

        return_buffer = line;

        if (integer_check)
        {
            if (line.empty())
            {
                integer_check = false;
            }
            else
            {
                for (char c : line) {
                    if (!isdigit(c))
                    {
                        integer_check = false;
                        break;
                    }
                }
            }

            if ( false == integer_check )
            {
                return_buffer = "";
            }
        }
    }
    return return_buffer;
}

int MiracastCommon::execute_SystemCommand( const char* system_command_buffer )
{
    int return_value = -1;
    MIRACASTLOG_TRACE("Entering ...");

    if (!system_command_buffer)
    {
        MIRACASTLOG_ERROR("NULL pointer has passed");
    }
    else
    {
	MIRACASTLOG_INFO("System command buffer[%s]",system_command_buffer);
	return_value = system(system_command_buffer);
    }
    MIRACASTLOG_TRACE("Exiting ...");
    return return_value;
}

bool MiracastCommon::execute_PopenCommand( const char* popen_command, const char* expected_char, unsigned int retry_count, std::string& popen_buffer, unsigned int interval_micro_sec )
{
    char buffer[1024] = {0};
    FILE *popen_pipe_ptr = nullptr;
    char *current_line_buffer = nullptr;
    std::size_t len = 0;
    unsigned int retry_index = 1;
    bool returnValue = false;

    MIRACASTLOG_TRACE("Entering ...");
    if ( nullptr == popen_command )
    {
        MIRACASTLOG_ERROR("#### Null Command ####");
        MIRACASTLOG_TRACE("Exiting ...");
        return false;
    }

    while ( retry_index <= retry_count )
    {
        MIRACASTLOG_INFO("#### Executing Command[%s] ####",popen_command);
        popen_pipe_ptr = popen( popen_command , "r");
        if ( nullptr == popen_pipe_ptr )
        {
            MIRACASTLOG_ERROR("popen() failed: [%s]",strerror(errno));
            ++retry_index;
            continue;
        }

        memset( buffer , 0x00 , sizeof(buffer));
        while (getline(&current_line_buffer, &len, popen_pipe_ptr) != -1)
        {
            sprintf(buffer + strlen(buffer), "%s" ,  current_line_buffer);
            MIRACASTLOG_INFO("#### popen Output[%s] ####", buffer);
        }
        pclose(popen_pipe_ptr);
        popen_pipe_ptr = nullptr;
        free(current_line_buffer);
        current_line_buffer = nullptr;
        popen_buffer = buffer;
        REMOVE_R(popen_buffer);
        REMOVE_N(popen_buffer);

        if ( nullptr != expected_char )
        {
            if ( nullptr != strstr( popen_buffer.c_str(), expected_char ))
            {
                MIRACASTLOG_INFO("POPEN command success on '%u' trial",retry_index);
                returnValue = true;
                break;
            }
            else
            {
                MIRACASTLOG_INFO("POPEN command trying for expected char[%s] on '%u' trial",expected_char,retry_index);
            }
        }
        else if (!popen_buffer.empty())
        {
            MIRACASTLOG_INFO("POPEN command success on '%u' trial",retry_index);
            returnValue = true;
            break;
        }
        usleep(interval_micro_sec);
        ++retry_index;
    }

    if ( false == returnValue && ( 1 < retry_count ))
    {
        MIRACASTLOG_ERROR("Maximum retries[%u] reached and Popen couldn't success",retry_count);
    }
    MIRACASTLOG_TRACE("Exiting ...");
    return returnValue;
}

MessageQueue::MessageQueue(int queueSize,void (*free_cb)(void *param))
{
    std::cout << "[ctor] " << std::endl;
    m_currentMsgCount = 0;
    m_maxMsgCount = queueSize;
    m_free_resource_cb = free_cb;
}

MessageQueue::~MessageQueue(void)
{
    std::cout << "[dtor] " << std::endl;
    {
        std::lock_guard<std::mutex> lk(mutexSync);
        m_isDestructing = true;
    }
    m_condNotEmpty.notify_all();
    m_condNotFull.notify_all();

    std::unique_lock<std::mutex> lk(mutexSync);
    void* userParam = nullptr;
    while (!m_internalQueue.empty())
    {
        userParam = m_internalQueue.front();
        if (nullptr != m_free_resource_cb)
        {
            std::cout << "[dtor] asked to free : " << userParam << std::endl;
            m_free_resource_cb(userParam);
        }
        m_internalQueue.pop();
        m_currentMsgCount--;
    }
    std::cout << "[dtor] done" << std::endl;
}

void MessageQueue::sendData(void* new_value,int wait_time_ms)
{
    std::unique_lock<std::mutex> lk(mutexSync);
    // Wait if the queue is full
    if (!m_condNotFull.wait_for(lk, std::chrono::milliseconds(wait_time_ms), [this] { return (( m_currentMsgCount < m_maxMsgCount ) || m_isDestructing ) ; }))
    {
        if (m_isDestructing)
        {
            std::cout << "[sendData] skipped dueto m_isDestructing" << std::endl;
        }
        else
        {
            std::cout << "[sendData] Timeout occurred while waiting to send data." << std::endl;
        }
        return;
    }

    if (m_isDestructing){
        return;
    }
    m_internalQueue.push(new_value);
    m_currentMsgCount++;
    std::cout << "[sendData] data at address: " << new_value << std::endl;
    // Notify consumer that new data is available
    m_condNotEmpty.notify_one();
}

void MessageQueue::ReceiveData(void*& value,int wait_time_ms)
{
    std::unique_lock<std::mutex> lk(mutexSync);
    // Wait if the queue is empty
    if (!m_condNotEmpty.wait_for(lk, std::chrono::milliseconds(wait_time_ms), [this] { return (( !m_internalQueue.empty()) || m_isDestructing ); }))
    {
        if (m_isDestructing)
        {
            std::cout << "[receiveData] skipped dueto m_isDestructing" << std::endl;
        }
        else
        {
            std::cout << "[receiveData] Timeout occurred while waiting to send data." << std::endl;
        }
        return;  // Timeout occurred
    }

    if (m_isDestructing){
        return;
    }

    value = m_internalQueue.front();
    m_internalQueue.pop();
    m_currentMsgCount--;
    std::cout << "[ReceiveData] data at address: " << value << std::endl;
    // Notify producer that space is available
    m_condNotFull.notify_one();
}