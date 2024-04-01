/**
* If not stated otherwise in this file or this component's LICENSE
* file the following copyright and licenses apply:
*
* Copyright 2023 Synamedia
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
**/

#include "WpaCtrl.h"
#include <gmock/gmock.h>

WpaCtrlApiImpl* WpaCtrlApi::impl = nullptr;

WpaCtrlApi::WpaCtrlApi() {}

void WpaCtrlApi::setImpl(WpaCtrlApiImpl* newImpl)
{
    // Handles both resetting 'impl' to nullptr and assigning a new value to 'impl'
    EXPECT_TRUE ((nullptr == impl) || (nullptr == newImpl));
    impl = newImpl;
}

struct wpa_ctrl* WpaCtrlApi::wpa_ctrl_open(const char *ctrl_path)
{
    EXPECT_NE(impl, nullptr);
    return impl->wpa_ctrl_open(ctrl_path);
}

void WpaCtrlApi::wpa_ctrl_close(struct wpa_ctrl* ctrl_handle)
{
    EXPECT_NE(impl, nullptr);
    return impl->wpa_ctrl_close(ctrl_handle);
}

int WpaCtrlApi::wpa_ctrl_request(struct wpa_ctrl *ctrl_handle, const char *cmd, size_t cmd_len, char *reply, size_t *reply_len, void(*msg_cb)(char *msg, size_t len))
{
    EXPECT_NE(impl, nullptr);
    return impl->wpa_ctrl_request(ctrl_handle, cmd, cmd_len, reply, reply_len, nullptr);
}

int WpaCtrlApi::wpa_ctrl_pending(struct wpa_ctrl* ctrl_handle)
{
    EXPECT_NE(impl, nullptr);
    return impl->wpa_ctrl_pending(ctrl_handle);
}

int WpaCtrlApi::wpa_ctrl_recv(struct wpa_ctrl *ctrl_handle, char *reply, size_t *reply_len)
{
    EXPECT_NE(impl, nullptr);
    return impl->wpa_ctrl_recv(ctrl_handle, reply, reply_len);
}

int WpaCtrlApi::wpa_ctrl_attach(struct wpa_ctrl *ctrl_handle)
{
    EXPECT_NE(impl, nullptr);
    return impl->wpa_ctrl_attach(ctrl_handle);
}

struct wpa_ctrl* (*wpa_ctrl_open)(const char *ctrl_path) = &WpaCtrlApi::wpa_ctrl_open;
void (*wpa_ctrl_close)(struct wpa_ctrl* ctrl_handle) = &WpaCtrlApi::wpa_ctrl_close;
int (*wpa_ctrl_request)(struct wpa_ctrl *ctrl_handle, const char *cmd, size_t cmd_len, char *reply, size_t *reply_len, void(*msg_cb)(char *msg, size_t len)) = &WpaCtrlApi::wpa_ctrl_request;
int (*wpa_ctrl_pending)(struct wpa_ctrl* ctrl_handle) = &WpaCtrlApi::wpa_ctrl_pending;
int (*wpa_ctrl_recv)(struct wpa_ctrl *ctrl_handle, char *reply, size_t *reply_len) = &WpaCtrlApi::wpa_ctrl_recv;
int (*wpa_ctrl_attach)(struct wpa_ctrl* ctrl_handle) = &WpaCtrlApi::wpa_ctrl_attach;
