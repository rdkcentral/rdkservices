#pragma once

#include <gmock/gmock.h>

#include "WpaCtrl.h"

class WpaCtrlApiImplMock : public WpaCtrlApiImpl {
public:
    virtual ~WpaCtrlApiImplMock() = default;

    MOCK_METHOD(struct wpa_ctrl *, wpa_ctrl_open, (const char *ctrl_path), (override));
    MOCK_METHOD(void, wpa_ctrl_close, (struct wpa_ctrl *), (override));
    MOCK_METHOD(int, wpa_ctrl_request , (struct wpa_ctrl *ctrl, const char *cmd, size_t cmd_len, char *reply, size_t *reply_len, void(*msg_cb)(char *msg, size_t len)), (override));
    MOCK_METHOD(int, wpa_ctrl_pending, (struct wpa_ctrl *), (override));
    MOCK_METHOD(int, wpa_ctrl_recv , (struct wpa_ctrl *ctrl, char *reply, size_t *reply_len), (override));
    MOCK_METHOD(int, wpa_ctrl_attach, (struct wpa_ctrl *), (override));
};
