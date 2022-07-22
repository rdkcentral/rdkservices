/**
* If not stated otherwise in this file or this component's LICENSE
* file the following copyright and licenses apply:
*
* Copyright 2019 RDK Management
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

#include "ActivityMonitor.h"

#include "UtilsJsonRpc.h"


#define ACTIVITY_MONITOR_METHOD_GET_APPLICATION_MEMORY_USAGE "getApplicationMemoryUsage"
#define ACTIVITY_MONITOR_METHOD_GET_ALL_MEMORY_USAGE "getAllMemoryUsage"
#define ACTIVITY_MONITOR_METHOD_ENABLE_MONITORING "enableMonitoring"
#define ACTIVITY_MONITOR_METHOD_DISABLE_MONITORING "disableMonitoring"

#define ACTIVITY_MONITOR_EVT_ON_MEMORY_THRESHOLD "onMemoryThreshold"
#define ACTIVITY_MONITOR_EVT_ON_CPU_THRESHOLD "onCPUThreshold"

#define VERSION_TXT_FILE "/version.txt"

#define REGISTRY_FILENAME "/home/root/waylandregistryreceiver.conf"
#define REGISTRY_FILENAME_RNE "/home/root/waylandregistryrne.conf"
#define REGISTRY_FILENAME_DEV "/opt/waylandregistry.conf"

#define CALLSIGN_PARAMETER "-C"

#define API_VERSION_NUMBER_MAJOR 1
#define API_VERSION_NUMBER_MINOR 0
#define API_VERSION_NUMBER_PATCH 0

namespace WPEFramework
{
    namespace {

        static Plugin::Metadata<Plugin::ActivityMonitor> metadata(
            // Version (Major, Minor, Patch)
            API_VERSION_NUMBER_MAJOR, API_VERSION_NUMBER_MINOR, API_VERSION_NUMBER_PATCH,
            // Preconditions
            {},
            // Terminations
            {},
            // Controls
            {}
        );
    }

    namespace Plugin
    {
        SERVICE_REGISTRATION(ActivityMonitor, API_VERSION_NUMBER_MAJOR, API_VERSION_NUMBER_MINOR, API_VERSION_NUMBER_PATCH);

        ActivityMonitor* ActivityMonitor::_instance = nullptr;


        struct AppConfig
        {
            AppConfig()
            {
                pid = memoryThresholdsMB = cpuThresholdPercent = cpuThresholdSeconds = 0;
                state = STATE_NORMAL;
                cpuUsage = 0;
                memExceeded = cpuExceededPercent = 0;
                eventSent = false;
            }

            enum {
                STATE_NORMAL,
                STATE_EXCEEDED,
                STATE_RECEDED,
            };

            unsigned int pid;
            unsigned int memoryThresholdsMB;
            unsigned int cpuThresholdPercent;
            unsigned int cpuThresholdSeconds;

            int state;
            long long unsigned int cpuUsage;
            std::chrono::system_clock::time_point cpuThreshold;
            unsigned int memExceeded;
            unsigned int cpuExceededPercent;
            bool eventSent;
        };

        struct MonitorParams
        {
            double memoryIntervalSeconds;
            double cpuIntervalSeconds;
            std::list <AppConfig> config;
            long long unsigned int totalCpuUsage;
            std::chrono::system_clock::time_point lastMemCheck;
            std::chrono::system_clock::time_point lastCpuCheck;
        };

        class MemoryInfo
        {
        public:
            static bool isDevOrVBNImage();
            static void initRegistry();

            static long long unsigned int getTotalCpuUsage();

            static unsigned int parseLine(const char *line);

            static unsigned int getFreeMemory();
            static void readSmaps(const char *pid, unsigned int &pvtOut, unsigned int &sharedOut);

            static void getProcStat(const char *dirName, std::string &cmdName, unsigned int &ppid, bool calcCpu, long long unsigned int &cpuTicks);
            static std::string getCallSign(int pid);
            static void getProcInfo(bool calcMem, bool calcCpu, std::vector<unsigned int> &pidsOut, std::vector <std::string> &cmdsOut, std::vector <unsigned int> &memUsageOut, std::vector <long long unsigned int> &cpuUsageOut);

        private:
            static std::map <std::string, std::string> registry;
            static bool isRegistryLoaded;
        };

        std::map <std::string, std::string> MemoryInfo::registry;
        bool MemoryInfo::isRegistryLoaded = false;


        ActivityMonitor::ActivityMonitor()
        : PluginHost::JSONRPC()
        , m_monitorParams(NULL)
        , m_stopMonitoring(false)
        {
            ActivityMonitor::_instance = this;

            Register(ACTIVITY_MONITOR_METHOD_GET_APPLICATION_MEMORY_USAGE, &ActivityMonitor::getApplicationMemoryUsage, this);
            Register(ACTIVITY_MONITOR_METHOD_GET_ALL_MEMORY_USAGE, &ActivityMonitor::getAllMemoryUsage, this);
            Register(ACTIVITY_MONITOR_METHOD_ENABLE_MONITORING, &ActivityMonitor::enableMonitoring, this);
            Register(ACTIVITY_MONITOR_METHOD_DISABLE_MONITORING, &ActivityMonitor::disableMonitoring, this);
        }

        ActivityMonitor::~ActivityMonitor()
        {
        }

        void ActivityMonitor::Deinitialize(PluginHost::IShell* /* service */)
        {
            ActivityMonitor::_instance = nullptr;

            if (m_monitor.joinable())
                m_monitor.join();

            delete m_monitorParams;
        }

        uint32_t ActivityMonitor::getApplicationMemoryUsage(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();

            if (parameters.HasLabel("pid"))
            {
                unsigned int pidToFind = 0;
                getNumberParameter("pid", pidToFind);
                if (pidToFind > 0)
                {
                    std::vector<unsigned int> pids;
                    std::vector <std::string> cmds;
                    std::vector <unsigned int> memUsage;
                    std::vector <long long unsigned int> cpuUsage;

                    MemoryInfo::getProcInfo(true, false, pids, cmds, memUsage, cpuUsage);

                    for (unsigned int n = 0; n < pids.size(); n++)
                    {
                        if (pids[n] == pidToFind)
                        {
                            JsonObject h;
                            h["appPid"] = pidToFind;
                            h["appName"] = cmds[n];
                            h["memoryMB"] = memUsage[n];

                            response["applicationMemory"] = h;
                            returnResponse(true);
                        }
                    }
                }
            }

            returnResponse(false);
        }

        uint32_t ActivityMonitor::getAllMemoryUsage(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();

            response["freeMemoryMB"] = MemoryInfo::getFreeMemory();

            JsonArray sl;

            std::vector<unsigned int> pids;
            std::vector <std::string> cmds;
            std::vector <unsigned int> memUsage;
            std::vector <long long unsigned int> cpuUsage;

            MemoryInfo::getProcInfo(true, false, pids, cmds, memUsage, cpuUsage);

            for (unsigned int n = 0; n < pids.size(); n++)
            {
                JsonObject h;

                h["appPid"] = pids[n];
                h["appName"] = cmds[n];
                h["memoryMB"] = memUsage[n];

                sl.Add(h);
            }

            response["applicationMemory"] = sl;

            returnResponse(true);
        }

        uint32_t ActivityMonitor::enableMonitoring(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();

            threadStop();
            JsonArray configArray = parameters["config"].Array();

            if (0 == configArray.Length())
            {
                LOGWARN("Got empty list of processes to monitor");
                returnResponse(false);
            }

            float memoryIntervalSeconds = 0;
            float cpuIntervalSeconds = 0;

            try
            {
                memoryIntervalSeconds = std::stof(parameters["memoryIntervalSeconds"] .String());
                cpuIntervalSeconds = std::stof(parameters["cpuIntervalSeconds"].String());
            }
            catch (...) {}

            if (0 == memoryIntervalSeconds && 0 == cpuIntervalSeconds)
            {
                LOGWARN("Interval for both CPU and Memory usage monitoring can't be 0");
                returnResponse(false);
            }

            delete m_monitorParams;

            m_monitorParams = new MonitorParams();

            m_monitorParams->totalCpuUsage = 0;

            m_monitorParams->lastMemCheck = std::chrono::system_clock::now();
            m_monitorParams->lastCpuCheck = std::chrono::system_clock::now();

            m_monitorParams->memoryIntervalSeconds = memoryIntervalSeconds;
            m_monitorParams->cpuIntervalSeconds = cpuIntervalSeconds;

            JsonArray::Iterator index(configArray.Elements());

            while (index.Next() == true)
            {
                if (Core::JSON::Variant::type::OBJECT == index.Current().Content())
                {
                    JsonObject m = index.Current().Object();

                    AppConfig conf;

                    getNumberParameterObject(m, "appPid", conf.pid);
                    getNumberParameterObject(m, "memoryThresholdMB", conf.memoryThresholdsMB);
                    getNumberParameterObject(m, "cpuThresholdPercent", conf.cpuThresholdPercent);
                    getNumberParameterObject(m, "cpuThresholdSeconds", conf.cpuThresholdSeconds);


                    m_monitorParams->config.push_back(conf);
                }
                else
                    LOGWARN("Unexpected variant type");
            }

            if (m_monitor.joinable())
                m_monitor.join();

            {
                std::lock_guard<std::mutex> lock(m_monitoringMutex);
                m_stopMonitoring = false;
            }

            m_monitor = std::thread(threadRun, this);

            returnResponse(true);
        }

        uint32_t ActivityMonitor::disableMonitoring(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();

            if (threadStop() == -1);
                LOGWARN("Monitoring is already disabled");

            delete m_monitorParams;
            m_monitorParams = NULL;

            returnResponse(true);
        }

        bool MemoryInfo::isDevOrVBNImage()
        {
            std::vector <char> buf;
            buf.resize(1024);

            FILE *f = fopen(VERSION_TXT_FILE, "r");
            if (!f)
            {
                LOGERR("Failed to open %s: %s", VERSION_TXT_FILE, strerror(errno));
                return false;
            }

            if (!fgets(buf.data(), buf.size(), f))
            {
                LOGERR("Failed to read from %s: %s", VERSION_TXT_FILE, strerror(errno));
                fclose(f);
                return false;
            }

            fclose(f);

            std::string s(buf.data());

            size_t pos = s.find("imagename:");
            if (pos != std::string::npos)
            {
                s = s.substr(pos + strlen("imagename:"));
                s = s.substr(0, s.find_first_of("\r\n"));
            }
            else
                s = "";

            return s.find("PROD") == std::string::npos && (s.find("DEV") != std::string::npos || s.find("VBN") != std::string::npos);
        }

        void MemoryInfo::initRegistry()
        {
            const char *registryFilename = REGISTRY_FILENAME;

            if (0 == access(REGISTRY_FILENAME_DEV, R_OK) && isDevOrVBNImage())
                registryFilename = REGISTRY_FILENAME_DEV;
            else if (0 == access(REGISTRY_FILENAME_RNE, R_OK))
                registryFilename = REGISTRY_FILENAME_RNE;

            JsonObject regObj;

            Core::File file;
            file = registryFilename;

            LOGINFO("Loading registry from %s", registryFilename);

            file.Open();
            regObj.IElement::FromFile(file);

            file.Close();

            if (regObj.HasLabel("waylandapps") && Core::JSON::Variant::type::ARRAY == regObj["waylandapps"].Content())
            {
                JsonArray apps = regObj["waylandapps"].Array();
                JsonArray::Iterator index(apps.Elements());

                int cnt = 0;
                while (index.Next() == true)
                {
                    if (Core::JSON::Variant::type::OBJECT == index.Current().Content())
                    {
                        JsonObject appEntry = index.Current().Object();

                        std::string binary = appEntry["binary"].String();

                        if (binary.size() > 0 && binary[binary.size() - 1] == '%')
                            binary = binary.substr(0, binary.size() - 1);

                        size_t li = binary.find_last_of('/');
                        if (std::string::npos != li)
                            binary = binary.substr(li + 1, binary.size() - li - 1);

                        registry[binary.c_str()] = appEntry["name"].String();
                        cnt++;
                    }
                    else
                        LOGWARN("Unexpected variant type");
                }

                LOGINFO("Loaded registry, %d entries", registry.size());
            }
            else
                LOGERR("Didn't find registry data");
        }

        long long unsigned int MemoryInfo::getTotalCpuUsage()
        {
            FILE *f = fopen("/proc/stat", "r");

            std::vector <char> buf;
            buf.resize(1024);

            if (NULL == fgets(buf.data(), buf.size(), f))
            {
                LOGERR("Failed to read stat, buffer is too small");
                return 0;
            }

            fclose(f);

            long long unsigned int user = 0, nice = 0, system = 0, idle = 0;

            int vc = sscanf(buf.data(), "%*s %llu %llu %llu %llu", &user, &nice, &system, &idle);
            if (4 != vc)
                LOGERR("Failed to parse /proc/stat, number of items matched: %d", vc);

            return user +  nice + system + idle;
        }

        unsigned int MemoryInfo::parseLine(const char *line)
        {
            std::string s(line);

            unsigned int val;

            size_t begin = s.find_first_of("0123456789");
            size_t end = std::string::npos;
            if (std::string::npos != begin)
                end = s.find_first_not_of("0123456789", begin);

            if (std::string::npos != begin && std::string::npos != end)
            {
                s = s.substr(begin, end);
                val = strtoul(s.c_str(), NULL, 10);
                return val;
            }
            else
                LOGERR("Failed to parse value from %s", line);

            return 0;
        }

        unsigned int MemoryInfo::getFreeMemory()
        {
            FILE *f = fopen("/proc/meminfo", "r");
            if (NULL == f)
            {
                LOGERR("Failed to open /proc/meminfo:%s", strerror(errno));
                return 0;
            }

            std::vector <char> buf;
            buf.resize(1024);

            unsigned int total = 0;

            while (fgets(buf.data(), buf.size(), f))
            {
                if (strstr(buf.data(), "MemFree:") == buf.data() || strstr(buf.data(), "Buffers:") == buf.data() || strstr(buf.data(), "Cached:") == buf.data())
                    total += parseLine(buf.data());
            }

            fclose(f);

            return total / 1024; // From KB to MB
        }

        void MemoryInfo::readSmaps(const char *pid, unsigned int &pvtOut, unsigned int &sharedOut)
        {
            std::string smapsName = "/proc/";
            smapsName += pid;
            smapsName += "/smaps";

            FILE *f = fopen(smapsName.c_str(), "r");
            if (NULL == f)
            {
                pvtOut = sharedOut = 0;
                return;
            }

            std::vector <char> buf;
            buf.resize(1024);

            size_t shared = 0;
            size_t pvt = 0;
            size_t pss = 0;
            bool withPss = false;

            while (fgets(buf.data(), buf.size(), f))
            {
                if (strstr(buf.data(), "Shared") == buf.data())
                {
                    shared += parseLine(buf.data());
                }
                else if (strstr(buf.data(), "Private") == buf.data())
                {
                    pvt += parseLine(buf.data());
                }
                else if (strstr(buf.data(), "Pss") == buf.data())
                {
                    withPss = true;
                    pss += parseLine(buf.data());
                }
            }

            fclose(f);

            if (withPss)
                shared = pss - pvt;

            pvtOut = pvt;
            sharedOut = shared;
        }

        void MemoryInfo::getProcStat(const char *dirName, std::string &cmdName, unsigned int &ppid, bool calcCpu, long long unsigned int &cpuTicks)
        {
            std::string statName = "/proc/";
            statName += dirName;
            statName += "/stat";

            std::vector <char> buf;
            buf.resize(1024);

            size_t r = 0;
            FILE *f = fopen(statName.c_str(), "r");
            if (f)
            {
                r = fread(buf.data(), 1, buf.size(), f);
                if (buf.size() == r)
                {
                    LOGERR("Failed to read stat, buffer is too small");
                }
                fclose(f);
            }

            std::string stat(buf.data(), r);

            std::size_t p1 = stat.find_first_of("(");
            std::size_t p2 = stat.find_first_of(")");
            if (std::string::npos == p1 || std::string::npos == p2)
            {
                //LOGINFO("Failed to parse command name from stat file '%s', '%s'", statName.c_str(), stat.c_str());
                return;
            }

            cmdName = stat.substr(p1 + 1, p2 - p1 - 1);

            size_t pos = p2, cnt = 0;

            while ((pos = stat.find_first_of(" ", pos + 1)) != std::string::npos)
            {
                cnt++;
                if (2 == cnt)
                    break;
            }

            ppid = 0;

            int vc = sscanf(buf.data() + pos, "%d", &ppid);
            if (1 != vc)
                LOGERR("Failed to parse parent pid from '%s'", stat.c_str());

            if (calcCpu)
            {
                while ((pos = stat.find_first_of(" ", pos + 1)) != std::string::npos)
                {
                    cnt++;
                    if (12 == cnt)
                        break;
                }

                long long unsigned int utime = 0, stime = 0, cutime = 0, cstime = 0;

                vc = sscanf(buf.data() + pos,
                                "%llu %llu " //utime, stime
                                "%llu %llu ", //cutime, cstime
                                &utime, &stime, &cutime, &cstime);
                if (4 != vc)
                {
                    LOGERR("Failed to parse parent cpu ticks from '%s', number of items matched: %d", stat.c_str(), vc);
                }

                cpuTicks = utime + stime + cutime + cstime;
            }

        }

        std::string MemoryInfo::getCallSign(int pid)
        {
            std::string callSign = "";

            std::stringstream fileName;
            fileName << "/proc/" << pid << "/cmdline";

            std::vector <char> buf;
            buf.resize(1024);
            buf.data()[0] = 0;

            size_t r = 0;
            FILE *f = fopen(fileName.str().c_str(), "r");
            if (f)
            {
                r = fread(buf.data(), 1, buf.size(), f);
                if (buf.size() == r)
                {
                    LOGERR("Failed to read stat, buffer is too small");
                }
                fclose(f);
            }

            buf.data()[buf.size() - 1] = 0;

            int pos = 0;
            while (pos < buf.size())
            {
                if (0 == strcmp(buf.data() + pos, CALLSIGN_PARAMETER))
                {    
                    pos += strlen(buf.data() + pos) + 1;

                    if (pos < buf.size())
                        callSign = (const char *)buf.data() + pos;
                    else
                        LOGERR("Unexpected end of cmd line");

                    break;
                }

                pos += strlen(buf.data() + pos) + 1;
            }

            return callSign;
        }

        void MemoryInfo::getProcInfo(bool calcMem, bool calcCpu, std::vector<unsigned int> &pidsOut, std::vector <std::string> &cmdsOut, std::vector <unsigned int> &memUsageOut, std::vector <long long unsigned int> &cpuUsageOut)
        {
            if (!isRegistryLoaded)
            {
                MemoryInfo::initRegistry();
                isRegistryLoaded = true;
            }

            if (!calcMem && !calcCpu)
            {
                LOGERR("Nothing to do");
                return;
            }

            std::vector<std::string> cmds;
            std::vector<unsigned int> pids;
            std::vector<unsigned int> ppids;
            std::vector<long long unsigned int> cpuUsage;

            DIR *d = opendir("/proc");

            struct dirent *de;

            while ((de = readdir(d)))
            {
                if (0 == de->d_name[0])
                    continue;

                char *end;
                pid_t pid = strtoul(de->d_name, &end, 10);
                if (0 != *end)
                    continue;

                std::string cmdName;
                unsigned int ppid = 0;
                long long unsigned int cpuTicks = 0;

                MemoryInfo::getProcStat(de->d_name, cmdName, ppid, calcCpu, cpuTicks);

                cmds.push_back(cmdName);
                pids.push_back(pid);
                ppids.push_back(ppid);
                cpuUsage.push_back(cpuTicks);
            }

            std::map <unsigned int, unsigned int> pidMap;
            for (unsigned int n = 0; n < pids.size(); n++)
                pidMap[pids[n]] = n;

            std::map <unsigned int, std::vector <unsigned int>> cmdMap;

            std::map <unsigned int, std::string> pid2callSign;

            for (unsigned int n = 0; n < cmds.size(); n++)
            {
                unsigned int lastIdx = cmds.size();

                for (unsigned int pid = pids[n],idx,cnt = 0; pid != 0; pid = ppids[idx],cnt++)
                {
                    idx = pidMap[pid];
                    std::string cmd = cmds[idx];

                    if (registry.size())
                    {
                        if (registry.find(cmd) != registry.end())
                        {
                            lastIdx = idx;
                        }
                    }
                    else if (ppids[idx] == getpid()) // if there is no waylandregistryreceiver.conf, monitoring the children of WPEFramework with "-C <callsign>" parameter
                    {
                        if (pid2callSign.find(pids[idx]) == pid2callSign.end())
                        {
                            std::string callSign = getCallSign(pids[idx]);

                            if (callSign.size() > 0)
                            {    
                                pid2callSign[pids[idx]] = callSign;
                                lastIdx = idx;
                            }

                        }
                        else
                        {    
                            lastIdx = idx;
                        }
                    }

                    if (cnt >= 100)
                    {
                        LOGERR("Too many iterations for process tree");
                        lastIdx = cmds.size();
                        break;
                    }
                }

                if (lastIdx < cmds.size())
                    cmdMap[lastIdx].push_back(n);
            }

            std::map <std::string, unsigned int> cmdCount;
            if (calcMem)
            {
                for (unsigned int n = 0; n < cmds.size(); n++)
                    cmdCount[cmds[n]]++;
            }

            for (std::map <unsigned int, std::vector <unsigned int>>::const_iterator it = cmdMap.cbegin(); it != cmdMap.cend(); it++)
            {
                unsigned int memUsage = 0;
                if (calcMem)
                {
                    for (unsigned int n = 0; n < it->second.size(); n++)
                    {
                        char s[256];
                        snprintf(s, sizeof(s), "%u", pids[it->second[n]]);

                        unsigned int pvt, shared;

                        readSmaps(s, pvt, shared);
                        unsigned int cnt = cmdCount[cmds[it->second[n]]];
                        if (0 == cnt)
                        {
                            LOGERR("Commnd count for %s was 0", cmds[n].c_str());
                            cnt = 1;
                        }
                        unsigned int usage = (pvt + shared / cnt) / 1024;

                        if (registry.size() && it->first != it->second[n])
                        {
                            pidsOut.push_back(pids[it->second[n]]);
                            cmdsOut.push_back(cmds[it->second[n]]);
                            memUsageOut.push_back(usage);
                        }

                        memUsage += usage;
                    }
                }

                long long unsigned int cpu_usage = 0;
                if (calcCpu)
                {
                    for (unsigned int n = 0; n < it->second.size(); n++)
                    {
                        if (registry.size() && it->first != it->second[n])
                        {
                            if (!calcMem) // If calcMem was disabled, pid and cmd should be added here.
                            {
                                pidsOut.push_back(pids[it->second[n]]);
                                cmdsOut.push_back(cmds[it->second[n]]);
                            }

                            cpuUsageOut.push_back(cpuUsage[it->second[n]]);
                        }
                        cpu_usage += cpuUsage[it->second[n]];
                    }
                }

                pidsOut.push_back(pids[it->first]);

                if (registry.size())
                {
                    cmdsOut.push_back(cmds[it->first]);
                }
                else
                {
                    if (pid2callSign.find(pids[it->first]) != pid2callSign.end())
                        cmdsOut.push_back(pid2callSign[pids[it->first]]);
                    else
                    {
                        LOGWARN("No callSign for %s(%d)", cmds[it->first].c_str(), pids[it->first]);
                        cmdsOut.push_back(cmds[it->first]);
                    }
                }

                memUsageOut.push_back(memUsage);
                cpuUsageOut.push_back(cpu_usage);
            }
        }

        void ActivityMonitor::threadRun(ActivityMonitor *am)
        {
            am->monitoring();
        }

        int ActivityMonitor::threadStop()
        {
            if (!m_monitor.joinable())
                return -1;

            std::unique_lock<std::mutex> lock(m_monitoringMutex);
            m_stopMonitoring = true;
            m_cond.notify_one();
            lock.unlock();
            m_monitor.join();
            return 0;
        }

        void ActivityMonitor::monitoring()
        {
            if (0 == m_monitorParams->config.size())
            {
                LOGERR("Empty app list to monitor");
                return;
            }

            while (1)
            {
                std::chrono::duration<double> elapsed = std::chrono::system_clock::now() - m_monitorParams->lastMemCheck;
                bool memCheck = m_monitorParams->memoryIntervalSeconds > 0 && elapsed.count() > m_monitorParams->memoryIntervalSeconds - 0.01;

                elapsed = std::chrono::system_clock::now() - m_monitorParams->lastCpuCheck;
                bool cpuCheck = m_monitorParams->cpuIntervalSeconds > 0 && elapsed.count() > m_monitorParams->cpuIntervalSeconds  - 0.01;

                std::vector<unsigned int> pids;
                std::vector <std::string> cmds;
                std::vector <unsigned int> memUsage;
                std::vector <long long unsigned int> cpuUsage;

                MemoryInfo::getProcInfo(memCheck, cpuCheck, pids, cmds, memUsage, cpuUsage);

                long long unsigned int totalCpuUsage = 0;

                if (cpuCheck)
                {
                    totalCpuUsage = MemoryInfo::getTotalCpuUsage();
                }

                for (std::list <AppConfig>::iterator it = m_monitorParams->config.begin(); it != m_monitorParams->config.end(); it++)
                {
                    unsigned int pid = it->pid;
                    unsigned int memoryUsed = 0;

                    if (memCheck)
                    {
                        for (unsigned int n = 0; n < pids.size(); n++)
                        {
                            if (pids[n] == pid)
                            {
                                memoryUsed = memUsage[n];
                                break;
                            }
                        }

                        if (0 == memoryUsed)
                        {
                            LOGERR("Failed to determine memory usage for %u", pid);
                        }

                        if (memoryUsed >= it->memoryThresholdsMB)
                        {
                            if (0 == it->memExceeded)
                            {
                                it->memExceeded = memoryUsed;

                                JsonObject memResult;
                                memResult["appPid"] = pid;
                                memResult["threshold"] = "exceeded";
                                memResult["memoryMB"] = memoryUsed;

                                LOGWARN("MemoryThreshold event appPid = %u, threshold = exceeded, memoryMB = %u", pid, memoryUsed);
                                onMemoryThresholdOccurred(memResult);
                            }
                        }
                        else
                        {
                            if (0 != it->memExceeded && memoryUsed < it->memExceeded - it->memExceeded / 20)
                            {
                                it->memExceeded = 0;

                                JsonObject memResult;
                                memResult["appPid"] = pid;
                                memResult["threshold"] = "receded";
                                memResult["memoryMB"] = memoryUsed;

                                LOGWARN("MemoryThreshold event appPid = %u, threshold = receded, memoryMB = %u", pid, memoryUsed);

                                onMemoryThresholdOccurred(memResult);
                            }

                        }
                    }

                    if (cpuCheck)
                    {
                        long long unsigned int usage = 0;

                        if (0 != m_monitorParams->totalCpuUsage)
                        {
                            for (unsigned int n = 0; n < pids.size(); n++)
                            {
                                if (pids[n] == pid)
                                {
                                    usage = cpuUsage[n];
                                    break;
                                }
                            }

                            if (0 != it->cpuUsage)
                            {
                                unsigned int percents = 0;
                                if (usage < it->cpuUsage)
                                {
                                    LOGERR("Wrong values for previous and current cpu usage %llu:%llu for pid: %u", usage, it->cpuUsage, pid);
                                }
                                else if (totalCpuUsage <= m_monitorParams->totalCpuUsage)
                                {
                                    LOGERR("Wrong values for previous and current total cpu ticks %llu:%llu", totalCpuUsage, m_monitorParams->totalCpuUsage);
                                }
                                else
                                {
                                    percents = 100 * ( usage - it->cpuUsage) / (totalCpuUsage - m_monitorParams->totalCpuUsage);
                                }

                                if (percents >= it->cpuThresholdPercent)
                                {
                                    if (AppConfig::STATE_NORMAL == it->state)
                                    {
                                        it->state = AppConfig::STATE_EXCEEDED;
                                        it->cpuThreshold = std::chrono::system_clock::now();
                                        it->cpuExceededPercent =  percents;
                                        it->eventSent = false;
                                    }
                                    else if (AppConfig::STATE_EXCEEDED == it->state)
                                    {
                                        std::chrono::duration<double> elapsed = std::chrono::system_clock::now() - it->cpuThreshold;

                                        if (elapsed.count() >= it->cpuThresholdSeconds && !it->eventSent)
                                        {
                                            JsonObject cpuResult;
                                            cpuResult["appPid"] = pid;
                                            cpuResult["threshold"] = "exceeded";
                                            cpuResult["cpuPercent"] = percents;

                                            LOGWARN("CPUThreshold event appPid = %u, threshold = exceeded, cpuPercent = %u", pid, percents);

                                            onCPUThresholdOccurred(cpuResult);

                                            it->eventSent = true;
                                        }
                                    }
                                    else if (AppConfig::STATE_RECEDED == it->state)
                                    {
                                        it->state = AppConfig::STATE_EXCEEDED;
                                    }
                                }
                                else
                                {
                                    if (AppConfig::STATE_EXCEEDED == it->state)
                                    {

                                        if (!it->eventSent)
                                        {
                                            it->state = AppConfig::STATE_NORMAL;
                                        }
                                        else if (percents < it->cpuExceededPercent - it->cpuExceededPercent / 20)
                                        {
                                            it->state = AppConfig::STATE_RECEDED;
                                            it->cpuThreshold = std::chrono::system_clock::now();
                                        }

                                    }
                                    else if (AppConfig::STATE_RECEDED == it->state)
                                    {
                                        if (percents < it->cpuExceededPercent - it->cpuExceededPercent / 20)
                                        {
                                            std::chrono::duration<double> elapsed = std::chrono::system_clock::now() - it->cpuThreshold;

                                            if (elapsed.count() >= it->cpuThresholdSeconds)
                                            {
                                                JsonObject cpuResult;
                                                cpuResult["type"] = "CPU";
                                                cpuResult["appPid"] = pid;
                                                cpuResult["threshold"] = "receded";
                                                cpuResult["cpuPercent"] = percents;

                                                LOGWARN("CPUThreshold event appPid = %u, threshold = receded, cpuPercent = %u", pid, percents);

                                                onCPUThresholdOccurred(cpuResult);
                                                it->state = AppConfig::STATE_NORMAL;
                                            }
                                        }
                                        else
                                        {
                                            it->state = AppConfig::STATE_EXCEEDED;
                                        }

                                    }
                                }
                            }
                        }

                        it->cpuUsage = usage;
                    }
                }

                if (memCheck)
                    m_monitorParams->lastMemCheck = std::chrono::system_clock::now();

                if (cpuCheck)
                {
                    m_monitorParams->lastCpuCheck = std::chrono::system_clock::now();
                    m_monitorParams->totalCpuUsage = totalCpuUsage;
                }

                elapsed = std::chrono::system_clock::now() - m_monitorParams->lastMemCheck;
                double sleepTime = m_monitorParams->memoryIntervalSeconds > 0 ? m_monitorParams->memoryIntervalSeconds - elapsed.count() : 10000;

                elapsed = std::chrono::system_clock::now() - m_monitorParams->lastCpuCheck;
                if (m_monitorParams->cpuIntervalSeconds > 0 && m_monitorParams->cpuIntervalSeconds - elapsed.count() < sleepTime)
                    sleepTime = m_monitorParams->cpuIntervalSeconds - elapsed.count();

                if (sleepTime < 0.01)
                {
                    LOGERR("SleepTime is too low, using 0.01 seconds");
                    sleepTime = 0.01;
                }

                auto sleepfor = std::chrono::milliseconds((long)(sleepTime * 1000));
                std::unique_lock<std::mutex> lock(m_monitoringMutex);
                if (m_cond.wait_for(lock, sleepfor, [this] { return this->m_stopMonitoring; }))
                    break;
            }
        }

        void ActivityMonitor::onMemoryThresholdOccurred(const JsonObject& result)
        {
            sendNotify(ACTIVITY_MONITOR_EVT_ON_MEMORY_THRESHOLD, result);
        }

        void ActivityMonitor::onCPUThresholdOccurred(const JsonObject& result)
        {
            sendNotify(ACTIVITY_MONITOR_EVT_ON_CPU_THRESHOLD, result);
        }

    } // namespace Plugin
} // namespace WPEFramework


