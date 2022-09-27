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

#include <thread>
#include <functional>
#include <fstream>

class FileSelectListener
{
public:
    FileSelectListener(const std::string &file,
                       uint32_t bufSize,
                       std::function<void(const std::string&)> func)
            : mFile(file)
            , mBufSize(bufSize)
            , mFunc(func)
            , mStop(false)
    {
        mThread = std::shared_ptr<std::thread>(
                new std::thread(&FileSelectListener::pollLoop, this));
    }

    ~FileSelectListener()
    {
        mStop = true;
        if (mThread != nullptr && mThread->joinable()) {
            mThread->join();
        }
    }

private:
    FileSelectListener(const FileSelectListener&) = delete;
    FileSelectListener& operator=(const FileSelectListener&) = delete;

    std::string mFile;
    uint32_t mBufSize;
    std::function<void(const std::string&)> mFunc;
    bool mStop;

    std::shared_ptr<std::thread> mThread;

    inline bool file_exists(const std::string &name) {
        std::ifstream f(name.c_str());
        f.close();
        return f.good();
    }

    void pollLoop() {
        char buf[mBufSize];
        int fd;
        while (!mStop) {
            if (!file_exists(mFile)) {
                syslog(LOG_ERR, "%s is missing. Retry in 2 seconds", mFile.c_str());
                std::this_thread::sleep_for(std::chrono::seconds(1));
                continue;
            }

            fd = open(mFile.c_str(), O_RDONLY);
            if (fd < 0) {
                syslog(LOG_ERR, "Could not open %s", mFile.c_str());
                continue;
            }

            while (!mStop) {
                fd_set rfds;
                struct timeval timeout;

                FD_ZERO(&rfds);
                FD_SET(fd, &rfds);

                /* Initialize the timeout data structure. */
                timeout.tv_sec = 2;
                timeout.tv_usec = 0;

                int res = select(fd + 1, &rfds, nullptr, nullptr, &timeout);
                if (res < 0) {
                    syslog(LOG_ERR, "Calling select on %s failed", mFile.c_str());
                    break;
                }

                if (!FD_ISSET(fd, &rfds)) {
                    continue;
                }
                lseek(fd, 0, SEEK_SET);

                res = read(fd, buf, mBufSize);
                if (res < 0) {
                    if (errno == ENOTCONN || errno == EBADF || errno == ECONNRESET) {
                        syslog(LOG_ERR, "Socket error for %s", mFile.c_str());
                        break;
                    } else {
                        syslog(LOG_ERR, "Failed to read data from %s", mFile.c_str());
                        std::this_thread::sleep_for(std::chrono::seconds(2));
                        continue;
                    }
                } else {
                    mFunc(std::string(buf, res));
                }
            }
            close(fd);
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }
};
