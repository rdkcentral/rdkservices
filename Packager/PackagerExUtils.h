
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

#include <thread>
#include <queue>
#include <functional>
#include <mutex>
#include <condition_variable>

#include "utils.h"

#include "PackagerExImplementation.h"

//#define FOO_T  std::function<uint32_t (JobMeta_t)>

#define FOO_T  std::function<void (const string&, const string&, const string&, const string&, const string&)>

//TODO: make configurable and scoped
//

#define TMP_FILENAME    "/opt/tmpApp.tgz"
#define APPS_ROOT_PATH  "/opt/dac_apps"

namespace WPEFramework {
namespace Plugin {

    class PackagerImplementation; // fwd

    class PackageInfoEx; //fileEndsWith

    typedef struct JobMeta_
    {
        //public:
       
        // JobMeta_t(const JobMeta_t&) = delete;
        // JobMeta_t& operator=(const JobMeta_t&) = delete;
        // ~JobMeta_t() {};

        JobMeta_() {};
        JobMeta_( const uint32_t tid, 
                   const string   &pid, 
                   const string   &tt, 
                   const string   &u,
                   const string   &tk, 
                   const string   &ll)

        : taskId(tid), pkgId(pid), type(tt), url(u), token(tk), listener(ll)
        {}

        uint32_t taskId;
        string   pkgId; 
        string   type; 
        string   url;
        string   token; 
        string   listener;

        // PackagerImplementation *ctx;
        // std::function< uint32_t (JobMeta_ &) > func;

    } JobMeta_t;

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // Threaded Job
    class JobPool
    {
       public:
            JobPool(const JobPool&) = delete;
            JobPool& operator=(const JobPool&) = delete;

            JobPool();
            ~JobPool();

            // void push(const FOO_T &job);
            void push(JobMeta_t& job);
            void done();
            void worker_func();

      private:
//            std::queue<std::function<void()>> mJobQ;

            std::queue<JobMeta_t>    mJobQ;
            // std::queue< FOO_T > mJobQ;

            std::mutex               mLock;
            std::condition_variable  mDataCondition;
            std::atomic<bool>        mAcceptJobs;
    };// CLASS - JobPool
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -


    class PackagerExUtils
    {
        public:
            enum class DACrc_t { dac_OK, dac_WARN, dac_FAIL };

            PackagerExUtils(const PackagerExUtils&) = delete;
            PackagerExUtils& operator=(const PackagerExUtils&) = delete;

            static bool initDB(const char* filename, const char* key);
            static bool createTable();

            // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
            // Clean up
            static void term();
            static void vacuum();

            // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
            // File helpers
            static bool    fileRemove(const char* f);
            static bool    fileExists(const char* f);
            static bool    fileEncrypted(const char* f);
            static bool    fileEndsWith(const string& f, const string& ext);
            static string  fileExtension(const string& f);

            static bool    removeFolder(const string& dirname);
            static bool    removeFolder(const char *dirname);

            static uint64_t folderSize(const char *d);

            static std::string getGUID();

            // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
            // SQL helpers

            static bool           hasPkgRow(const string& pkgId);
            static bool           hasPkgRow(const char* pkgId);
            static bool           addPkgRow(const PackageInfoEx* pkg);
            static PackageInfoEx* getPkgRow(const string& pkgId);
            static bool           delPkgRow(const string& pkgId);

            static int64_t        sumSizeInBytes();

            static void           showTable();

            static void           updatePkgList(PackageList_t& list);

            // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
            // House-keeping
            static void setupThreadQ(PackagerImplementation *ptr);
            static void killThreadQ();

            static void addJob(JobMeta_t &job);
            // static void addJob( FOO_T &job );

            // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
            // Download helpers
            static DACrc_t validateURL(const char *url);
            static DACrc_t downloadJSON(const char *url, const char *tempName);
            static DACrc_t downloadURL( const char *url, const char *tempName);

            // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
            // Archive helpers
            static DACrc_t extractPKG(const char *filename, const char *to_path = nullptr);

            // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
            // private data
            static void*        mData;
            static JsonObject   mPackageCfg;

        private:

            static PackagerImplementation   *mImpl;

            static JobPool                  mJobPool;
            static std::vector<std::thread> mThreadPool; // thread pool

            static Core::CriticalSection    mThreadLock;

            static const int64_t  MAX_SIZE_BYTES;
            static const int64_t  MAX_VALUE_SIZE_BYTES;
    };
  } // namespace Plugin
}  // namespace WPEFramework
