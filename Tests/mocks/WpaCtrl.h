#pragma once

struct wpa_ctrl {
    int s;
    char *ctrl_path;
};

class WpaCtrlApiImpl {
public:
    virtual ~WpaCtrlApiImpl() = default;

    virtual struct wpa_ctrl* wpa_ctrl_open(const char *ctrl_path) = 0;
    virtual void wpa_ctrl_close(struct wpa_ctrl* ctrl_handle) = 0;
    virtual int wpa_ctrl_request(struct wpa_ctrl *ctrl_handle, const char *cmd, size_t cmd_len, char *reply, size_t *reply_len, void(*msg_cb)(char *msg, size_t len)) = 0;
    virtual int wpa_ctrl_pending(struct wpa_ctrl* ctrl_handle) = 0;
    virtual int wpa_ctrl_recv(struct wpa_ctrl *ctrl_handle, char *reply, size_t *reply_len) = 0;
    virtual int wpa_ctrl_attach(struct wpa_ctrl* ctrl_handle) = 0;
};

class WpaCtrlApi {
protected:
    static WpaCtrlApiImpl* impl;

public:
    WpaCtrlApi();
    WpaCtrlApi(const WpaCtrlApi &obj) = delete;
    static void setImpl(WpaCtrlApiImpl* newImpl);
    static struct wpa_ctrl* wpa_ctrl_open(const char *ctrl_path);
    static void wpa_ctrl_close(struct wpa_ctrl* ctrl_handle);
    static int wpa_ctrl_request(struct wpa_ctrl *ctrl_handle, const char *cmd, size_t cmd_len, char *reply, size_t *reply_len, void(*msg_cb)(char *msg, size_t len));
    static int wpa_ctrl_pending(struct wpa_ctrl* ctrl_handle);
    static int wpa_ctrl_recv(struct wpa_ctrl *ctrl_handle, char *reply, size_t *reply_len);
    static int wpa_ctrl_attach(struct wpa_ctrl* ctrl_handle);
};

extern struct wpa_ctrl* (*wpa_ctrl_open)(const char *ctrl_path);
extern void (*wpa_ctrl_close)(struct wpa_ctrl* ctrl_handle);
extern int (*wpa_ctrl_request)(struct wpa_ctrl *ctrl_handle, const char *cmd, size_t cmd_len, char *reply, size_t *reply_len, void(*msg_cb)(char *msg, size_t len));
extern int (*wpa_ctrl_pending)(struct wpa_ctrl* ctrl_handle);
extern int (*wpa_ctrl_recv)(struct wpa_ctrl *ctrl_handle, char *reply, size_t *reply_len);
extern int (*wpa_ctrl_attach)(struct wpa_ctrl* ctrl_handle);
