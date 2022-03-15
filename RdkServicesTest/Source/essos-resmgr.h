#pragma once

using EssRMgr = std::string;

EssRMgr* EssRMgrCreate() { 
    static EssRMgr msg {"EssRMgr"};
    return &msg;  
}

enum Status {
    EssRMgrRes_idle
};

void EssRMgrDestroy(EssRMgr*) {}

void EssRMgrGetAVState(EssRMgr*, int* status) {
    *status = 1;
}