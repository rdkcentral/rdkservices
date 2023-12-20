#pragma once

#include <stdio.h>
#include "wpa_ctrl_mock.h"
#include "secure_wrappermock.h"

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
    virtual FILE *v_secure_popen(const char *direction, const char *command, va_list args) = 0;
    virtual int v_secure_pclose(FILE *) = 0;
    virtual int v_secure_system(const char *command, va_list args) = 0;
    virtual int unlink(const char* filePath) = 0;
};

class Wraps {
protected:
   static WrapsImpl* impl;
public:
    Wraps();
    Wraps(const Wraps &obj) = delete;
    static void setImpl(WrapsImpl* newImpl);
    static Wraps& getInstance();
    static int system(const char* command);
    static FILE* popen(const char* command, const char* type);
    static void syslog(int pri, const char* fmt, va_list args);
    static struct wpa_ctrl * wpa_ctrl_open(const char *ctrl_path);
    static int wpa_ctrl_request(struct wpa_ctrl *ctrl, const char *cmd, size_t cmd_len,
                                                   char *reply, size_t *reply_len,
                                                   void *msg_cb);
    static void wpa_ctrl_close(struct wpa_ctrl *ctrl);
    static FILE *v_secure_popen(const char *direction, const char *command, va_list args);
    static int v_secure_pclose(FILE *file);
    static int v_secure_system(const char *command, va_list args);
    static int unlink(const char* filePath);
};
