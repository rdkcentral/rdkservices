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

std::string MiracastCommon::parse_opt_flag( std::string file_name , bool integer_check )
{
    std::string return_buffer = "";
    std::ifstream parse_opt_flag_file( file_name.c_str());

    if (!parse_opt_flag_file)
    {
        MIRACASTLOG_ERROR("Failed to open [%s] file\n",file_name.c_str());
    }
    else
    {
        std::string word;
        parse_opt_flag_file >> word;
        parse_opt_flag_file.close();

        return_buffer = word;

        if (integer_check)
        {
            if (word.empty())
            {
                integer_check = false;
            }
            else
            {
                for (char c : word) {
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