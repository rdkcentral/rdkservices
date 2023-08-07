#pragma once

/* This file is copied from wpa_supplicant(lib32-wpa-supplicant/2.10-r0/wpa_supplicant-2.10/src/common/wpa_ctrl.h)
 * to resolve the unit test compliation issue */
 
#ifdef  __cplusplus
extern "C" {
#endif

struct wpa_ctrl * wpa_ctrl_open(const char *ctrl_path);
void wpa_ctrl_close(struct wpa_ctrl *ctrl);
int wpa_ctrl_request(struct wpa_ctrl *ctrl, const char *cmd, size_t cmd_len,
		     char *reply, size_t *reply_len,
		     void (*msg_cb)(char *msg, size_t len));

#ifdef  __cplusplus
}
#endif
