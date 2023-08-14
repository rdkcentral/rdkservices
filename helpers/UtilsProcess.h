#pragma once

#include <iostream>
#include <cstring>
#include <string>
#include <cstdlib>
#include <proc/readproc.h>
#include <vector>
#include <UtilsLogging.h>

using namespace std;

namespace Utils
{

/**
 * @brief Find all the process IDs with the given process name
 * @param[in] input_pname - The given process name
 * @param[out] processIds - The list of process IDs with the given process name
 * @return true if any process IDs found, otherwise false is returned
 */
bool userDefinedPgrep(string& input_pname, vector<string>& processIds)
{
    PROCTAB* proc = openproc(PROC_FILLMEM | PROC_FILLSTAT | PROC_FILLSTATUS);
    proc_t proc_info = {0};
    bool ret_value = false;

    if (proc != NULL)
    {
        memset(&proc_info, 0, sizeof(proc_info));
        while (readproc(proc, &proc_info) != NULL)
        {
            if (proc_info.cmd == input_pname)
            {
                processIds.push_back(std::to_string(proc_info.tid));
                ret_value = true;
            }
        }
        closeproc(proc);
    }
    return ret_value;
}

/**
* @brief Kill all the processes with the given process name
* @param[in] input_pname - The given process name
* @return true if any process with the given name was killed, otherwise false is returned
*/
bool userDefinedPkill(string& input_pname)
{
    PROCTAB* proc = openproc(PROC_FILLMEM | PROC_FILLSTAT | PROC_FILLSTATUS);
    proc_t proc_info = {0};
    bool ret_value = false;

    if (proc != NULL)
    {
        memset(&proc_info, 0, sizeof(proc_info));
        while (readproc(proc, &proc_info) != NULL)
        {
            if (proc_info.cmd == input_pname)
            {
                if (0 == kill(proc_info.tid, SIGTERM))
                {
                    ret_value = true;
                    LOGINFO("Killed the process [%d] process name [%s]", proc_info.tid, proc_info.cmd);
                }
            }
        }
        closeproc(proc);
    }
    return ret_value;
}

/**
* @brief Get list of child processes with the given parent process ID, equivalent to "pgrep  -P <PPID>"
* @param[in] input_ppid - The given parent process ID
* @param[out] processIds - The list of child process IDs
* @return true if there are any child processes of the given parent process ID, otherwise false is returned
*/
bool childProcessesWithPPID(int input_ppid, vector<int>& processIds)
{
    PROCTAB* proc = openproc(PROC_FILLMEM | PROC_FILLSTAT | PROC_FILLSTATUS);
    proc_t proc_info = {0};
    bool ret_value = false;

    if (proc != NULL)
    {
        memset(&proc_info, 0, sizeof(proc_info));
        while (readproc(proc, &proc_info) != NULL)
        {
            if (proc_info.ppid == input_ppid)
            {
                processIds.push_back(proc_info.tid);
                ret_value = true;
            }
        }
        closeproc(proc);
    }
    return ret_value;
}

}
