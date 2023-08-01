#pragma once

#include <core/core.h>
#include <iostream>
#include <cstring>
#include <string>
#include <cstdlib>
#include <proc/readproc.h>
#include <vector>

using namespace std;

namespace Utils
{

/* Getting the Process Ids of the processname - pgrep <processname> */
bool UserDefinedPgrep(string& input_pname, vector<string>& processIds)
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

/* Killing the Processes - pkill <processname> */
bool UserDefinedPkill(string& input_pname, vector<string>& processIds)
{
    PROCTAB* proc = openproc(PROC_FILLMEM | PROC_FILLSTAT | PROC_FILLSTATUS);
    proc_t proc_info = {0};
    vector<string> processIds;
    bool ret_value = false;

    if(proc != NULL)
    {
        memset(&proc_info, 0, sizeof(proc_info));
        while (readproc(proc, &proc_info) != NULL)
        {
            if (proc_info.cmd == input_pname)
            {
                ret_value = true;
                processIds.push_back(std::to_string(proc_info.tid));
                if (-1 == kill(std::to_string(proc_info.tid), SIGTERM))
                {
                    ret_value = false;
                }
            }
        }
        closeproc(proc);
    }
    return ret_value;
}

/* Getting child process from the given input PPID - pgrep  -P <PPID> */
bool ChildProcessesWithPPID(int input_pname, vector<int>& processIds)
{
    PROCTAB* proc = openproc(PROC_FILLMEM | PROC_FILLSTAT | PROC_FILLSTATUS);
    proc_t proc_info = {0};
    bool ret_value = false;

    if (proc != NULL)
    {
        memset(&proc_info, 0, sizeof(proc_info));
        while (readproc(proc, &proc_info) != NULL)
        {
            if (proc_info.ppid == input_pname)
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
