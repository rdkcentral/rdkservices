/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2020 RDK Management
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
 */

#ifndef __DRM_KMS_H__
#define __DRM_KMS_H__

#include <xf86drm.h>
#include <xf86drmMode.h>
#include <drm_mode.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _kms_ctx {

    drmModeRes *res;
    drmModeConnector *connector;
    drmModeEncoder *encoder;
    drmModeCrtc *crtc;

    drmModeCrtc *previous_crtc;
    drmModeModeInfo current_info;

    uint32_t connector_id;      /**< Connector id which will be use in program  */
    uint32_t crtc_id;
    uint32_t encoder_id;
    uint32_t primary_plane_id;
    uint32_t overlay_plane_id;

    /* atomic properties */
    uint32_t active_property;
    uint32_t mode_id_property;
    uint32_t crtc_id_property;
    uint32_t blob_id;

    uint32_t fb_id_property;
    uint32_t crtc_x_property;
    uint32_t crtc_y_property;
    uint32_t crtc_h_property;
    uint32_t crtc_w_property;
    uint32_t src_x_property;
    uint32_t src_y_property;
    uint32_t src_w_property;
    uint32_t src_h_property;

    drmModeAtomicReq *req;

} kms_ctx;

/**
 * @brief Create kms context
 *
 * @param[in] fd    drm file descriptor
 *
 *
 * @retval kms context
 */
kms_ctx* kms_setup(int fd);



/**
 * @brief Cleanup kms context 
 *
 * @param[in] kms    kms context
 *
 */
void kms_cleanup_context(kms_ctx *kms);

/**
 * @brief Set and get possible encoder id
 *
 * @param[in] fd    drm file descriptor
 * @param[in] kms   kms context
 *
 */
void kms_setup_encoder(int fd, kms_ctx *kms);


/**
 * @brief Set and get possible connector id
 *
 * @param[in] fd    drm file descriptor
 * @para,[in] kms   kms context
 *
 */
void kms_setup_connector(int fd, kms_ctx *kms);

/**
 * @brief Set and get possible crtc id
 *
 * @param[in] fd    drm file descriptor
 * @param[in] kms   kms context
 *
 */
void kms_setup_crtc(int fd, kms_ctx *kms);

/**
 * @brief Get the specific object properties
 *
 * @param[in] fd     drm file descriptor
 * @param[in] props  all object properties
 * @param[in] name   the property name which want to be queried
 *
 */
uint32_t kms_get_properties(int fd, drmModeObjectProperties *props, const char *name);


/**
 * @brief Get primary and overlay plane.
 *        The plane id will set to -1 if cannot get.
 *
 * @param[in] fd    drm file descriptor
 * @param[in] kms   kms context
 *
 */
void kms_get_plane( int fd, kms_ctx *kms);

#ifdef __cplusplus
}
#endif
#endif
