#include <stdarg.h>
#include <syslog.h>
#include "Wraps.h"

extern "C" int __wrap_system(const char* command)
{
    return Wraps::getInstance().system(command);
}

extern "C" FILE* __wrap_popen(const char* command, const char* type)
{
    return Wraps::getInstance().popen(command, type);
}

//This function will be called for syslog() in the code (added -Wl,-wrap,syslog)
extern  "C" void __wrap_syslog(int pri, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    Wraps::getInstance().syslog(pri, fmt, args);
    vsyslog(pri, fmt, args);
    va_end(args);
}

extern "C" struct wpa_ctrl * __wrap_wpa_ctrl_open(const char *ctrl_path)
{
    return Wraps::getInstance().wpa_ctrl_open(ctrl_path);
}

extern "C" int __wrap_wpa_ctrl_request(struct wpa_ctrl *ctrl, const char *cmd, size_t cmd_len,
                                                   char *reply, size_t *reply_len,
                                                   void *msg_cb)
{
    return Wraps::getInstance().wpa_ctrl_request(ctrl, cmd, cmd_len, reply, reply_len, msg_cb);
}

extern "C" void __wrap_wpa_ctrl_close(struct wpa_ctrl *ctrl)
{
    Wraps::getInstance().wpa_ctrl_close(ctrl);
}

extern "C" FILE * __wrap_v_secure_popen(const char *direction, const char *command, ...)
{
    va_list args;
    FILE *retFp = nullptr;
    va_start(args, command);
    retFp = Wraps::getInstance().v_secure_popen(direction, command, args);
    va_end(args);
    return retFp;
}

extern "C" int __wrap_v_secure_pclose(FILE *file)
{
    return Wraps::getInstance().v_secure_pclose(file);
}

extern "C" int __wrap_unlink(const char* filePath)
{
    return Wraps::getInstance().unlink(filePath);
}

extern "C" int __wrap_v_secure_system(const char *command, ...)
{
    va_list args;
    int ret;
    va_start(args, command);
    ret = Wraps::getInstance().v_secure_system(command, args);
    va_end(args);
    return ret;
}
