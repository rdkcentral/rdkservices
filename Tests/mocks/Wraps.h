#pragma once

#include <stdio.h>
#include "wpa_ctrl_mock.h"

class WrapsImpl {
public:
    virtual ~WrapsImpl() = default;

    virtual int system(const char* command) = 0;
    virtual FILE* popen(const char* command, const char* type) = 0;
    virtual void syslog(int pri, const char* fmt, va_list args) = 0;
    virtual struct wpa_ctrl * wpa_ctrl_open(const char *ctrl_path) = 0;
    virtual int wpa_ctrl_request(struct wpa_ctrl *ctrl, const char *cmd, size_t cmd_len,
                                                   char *reply, size_t *reply_len,
                                                   void *msg_cb) = 0;
    virtual void wpa_ctrl_close(struct wpa_ctrl *ctrl) = 0;
};

class Wraps {
public:
    static Wraps& getInstance()
    {
        static Wraps instance;
        return instance;
    }

    WrapsImpl* impl;

    static int system(const char* command)
    {
        return getInstance().impl->system(command);
    }

    static FILE* popen(const char* command, const char* type)
    {
        return getInstance().impl->popen(command, type);
    }

    static void syslog(int pri, const char* fmt, va_list args)
    {
        getInstance().impl->syslog(pri, fmt, args);
    }

    static struct wpa_ctrl * wpa_ctrl_open(const char *ctrl_path)
    {
        return getInstance().impl->wpa_ctrl_open(ctrl_path);
    }

    static int wpa_ctrl_request(struct wpa_ctrl *ctrl, const char *cmd, size_t cmd_len,
                                                   char *reply, size_t *reply_len,
                                                   void *msg_cb)
    {
        return getInstance().impl->wpa_ctrl_request(ctrl, cmd, cmd_len, reply, reply_len, msg_cb);
    }

    static void wpa_ctrl_close(struct wpa_ctrl *ctrl)
    {
        getInstance().impl->wpa_ctrl_close(ctrl);
    }
};
