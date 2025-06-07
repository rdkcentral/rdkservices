/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
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
 */

#pragma once

#include <string>
#include <memory>
#include <dlfcn.h>
#include <fstream>

#ifndef LIB_DFL_DIR
#define LIB_DFL_DIR "/usr/lib/WPEFramework/plugins"
#endif

namespace Utils {

class LibraryLoader 
{
public:
    enum ErrorCode {
        NO_ERROR = 0,
        LIBRARY_PATH_EMPTY,
        LIBRARY_LOAD_FAILURE
    };

    LibraryLoader() = default;
    ~LibraryLoader()
    {
        if (mLibraryHandle != nullptr) {
            dlclose(mLibraryHandle);
        }
    }

    ErrorCode Load(const std::string &libraryPath, std::string &errorMessage = std::string())
    {
        dlerror();
        if (libraryPath.empty()) {
            errorMessage = "Library path is empty.";
            return ErrorCode::LIBRARY_PATH_EMPTY;
        }

        if (mLibraryHandle != nullptr) {
            dlclose(mLibraryHandle);
            mLibraryHandle = nullptr;
        }

        libraryPath = std::string(LIB_DFL_DIR) + "/" + libraryPath;

        mLibraryHandle = dlopen(libraryPath.c_str(), RTLD_LAZY);
        if (mLibraryHandle == nullptr) {
            errorMessage = dlerror();
            return ErrorCode::LIBRARY_LOAD_FAILURE;
        }

        return ErrorCode::NO_ERROR;
    }

    template <typename T, typename... Args> std::shared_ptr<T> CreateShared(Args&&... args, std::string &errorMessage = std::string()) {
        using CreatorFunc = T* (*)(Args...);

        if (mLibraryHandle == nullptr) {
            errorMessage = "Library not loaded.";
            return nullptr;
        }

        dlerror();
        CreatorFunc func = reinterpret_cast<CreatorFunc> dlsym(mLibraryHandle, "Create");
        if (func == nullptr) {
            errorMessage = dlerror();
            return nullptr;
        }
        return std::shared_ptr<T>(func(std::forward<Args>(args)...));
    }

private:

    void* mLibraryHandle = nullptr;
};

}