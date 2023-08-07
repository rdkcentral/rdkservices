#pragma once

#include <gmock/gmock.h>

#include "Wraps.h"

class WrapsImplMock : public WrapsImpl {
public:
    virtual ~WrapsImplMock() = default;

    MOCK_METHOD(int, system, (const char* command), (override));
    MOCK_METHOD(FILE*, popen, (const char* command, const char* type), (override));
    MOCK_METHOD(void, syslog, (int pri, const char* fmt, va_list args), (override));
    MOCK_METHOD(struct wpa_ctrl*, wpa_ctrl_open, (const char *ctrl_path), (override));
    MOCK_METHOD(int, wpa_ctrl_request, (struct wpa_ctrl *ctrl, const char *cmd, size_t cmd_len,
                                                   char *reply, size_t *reply_len,
                                                   void *msg_cb), (override));
    MOCK_METHOD(void, wpa_ctrl_close, (struct wpa_ctrl *ctrl), (override));
};
