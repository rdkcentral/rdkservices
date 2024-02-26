#pragma once

struct wpa_ctrl {
    int s;
    char *ctrl_path;
};

class WpaCtrlImpl {
public:
    virtual ~WpaCtrlImpl() = default;

    virtual struct wpa_ctrl* wpa_ctrl_open(const char *ctrl_path) = 0;
    virtual void wpa_ctrl_close(struct wpa_ctrl* ctrl_handle) = 0;
    virtual int wpa_ctrl_request(struct wpa_ctrl *ctrl_handle, const char *cmd, size_t cmd_len, char *reply, size_t *reply_len, void(*msg_cb)(char *msg, size_t len)) = 0;
    virtual int wpa_ctrl_pending(struct wpa_ctrl* ctrl_handle) = 0;
    virtual int wpa_ctrl_recv(struct wpa_ctrl *ctrl_handle, char *reply, size_t *reply_len) = 0;
    virtual int wpa_ctrl_attach(struct wpa_ctrl* ctrl_handle) = 0;
};

class WpaCtrl {
public:
    static WpaCtrl& getInstance()
    {
        static WpaCtrl instance;
        return instance;
    }

    WpaCtrlImpl* impl;

    static struct wpa_ctrl* wpa_ctrl_open(const char *ctrl_path)
    {
        return getInstance().impl->wpa_ctrl_open(ctrl_path);
    }

    static void wpa_ctrl_close(struct wpa_ctrl* ctrl_handle)
    {
        return getInstance().impl->wpa_ctrl_close(ctrl_handle);
    }

    static int wpa_ctrl_request(struct wpa_ctrl *ctrl_handle, const char *cmd, size_t cmd_len, char *reply, size_t *reply_len, void(*msg_cb)(char *msg, size_t len))
    {
        return getInstance().impl->wpa_ctrl_request(ctrl_handle, cmd, cmd_len, reply, reply_len, nullptr);
    }

    static int wpa_ctrl_pending(struct wpa_ctrl* ctrl_handle)
    {
        return getInstance().impl->wpa_ctrl_pending(ctrl_handle);
    }

    static int wpa_ctrl_recv(struct wpa_ctrl *ctrl_handle, char *reply, size_t *reply_len)
    {
        return getInstance().impl->wpa_ctrl_recv(ctrl_handle, reply, reply_len);
    }

    static int wpa_ctrl_attach(struct wpa_ctrl *ctrl_handle)
    {
        return getInstance().impl->wpa_ctrl_attach(ctrl_handle);
    }
};

constexpr auto wpa_ctrl_open = &WpaCtrl::wpa_ctrl_open;
constexpr auto wpa_ctrl_close = &WpaCtrl::wpa_ctrl_close;
constexpr auto wpa_ctrl_request = &WpaCtrl::wpa_ctrl_request;
constexpr auto wpa_ctrl_pending = &WpaCtrl::wpa_ctrl_pending;
constexpr auto wpa_ctrl_recv = &WpaCtrl::wpa_ctrl_recv;
constexpr auto wpa_ctrl_attach = &WpaCtrl::wpa_ctrl_attach;
