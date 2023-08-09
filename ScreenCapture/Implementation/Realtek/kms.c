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

#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "kms.h"

void kms_setup_encoder( int fd, kms_ctx *kms )
{
    int crtcId = 0;

    for( int i = 0; i < kms->res->count_encoders; i++ ) {

        kms->encoder = drmModeGetEncoder(fd,kms->res->encoders[i]);

        if ( kms->encoder && ( kms->encoder->encoder_id == kms->connector->encoder_id ) ) {

            kms->encoder_id = kms->encoder->encoder_id;
            return;
        }


        for( int j = 0; j < kms->res->count_crtcs; j++ ) {

            if( kms->encoder->possible_crtcs & ( 1 << j ) ) {

                drmModeFreeEncoder( kms->encoder );
                kms->encoder = drmModeGetEncoder(fd, kms->res->encoders[j]);

                crtcId = kms->res->crtcs[j];
                kms->encoder->crtc_id = kms->crtc_id = j;
                goto exit;
            }
        }
    }

exit:
    return;
}




void kms_setup_connector( int fd, kms_ctx *kms )
{
    int i = 0, j = 0;
    drmModeConnector *connector;

    for( i = 0; i < kms->res->count_connectors; i++ ) {

        connector = drmModeGetConnector(fd, kms->res->connectors[i]);
        if( connector ) {

            if( connector->count_modes && ( connector->connection == DRM_MODE_CONNECTED ) ) {
                break;
            }
        }
    }

    if ( connector ) {

        kms->connector = connector;
        kms->connector_id = connector->connector_id;
    }

    return;
}


void kms_setup_crtc( int fd, kms_ctx *kms )
{
    if( kms->encoder ) {

        kms->crtc = drmModeGetCrtc(fd, kms->encoder->crtc_id);

        if( kms->crtc && kms->crtc->mode_valid ) {

            kms->current_info = kms->crtc->mode;
            kms->crtc_id = kms->encoder->crtc_id;
        }
    }

    return;
}


kms_ctx* kms_setup( int fd )
{
    kms_ctx *kms = NULL;
    kms = (kms_ctx*)calloc(1,sizeof(*kms));
    if( !kms )
        assert(0);

    kms->res = drmModeGetResources(fd);

    kms_setup_connector(fd, kms);
    kms_setup_encoder(fd, kms);
    kms_setup_crtc(fd, kms);
    return kms;
}


void kms_cleanup_context( kms_ctx *kms )
{
    if( kms->connector )
        drmModeFreeConnector(kms->connector);

    if( kms->encoder )
        drmModeFreeEncoder(kms->encoder);

    if( kms->crtc )
        drmModeFreeCrtc(kms->crtc);

    if( kms->res )
        drmModeFreeResources(kms->res);
}


uint32_t kms_get_properties(int fd, drmModeObjectProperties *props, const char *name)
{
    drmModePropertyPtr property;
    uint32_t i, id = 0;

    for (i = 0; i < props->count_props; i++) {

        property = drmModeGetProperty(fd, props->props[i]);
        if (!strcmp(property->name, name))
            id = property->prop_id;

        drmModeFreeProperty(property);

        if ( id )
            return id;
    }
}



void kms_get_plane( int fd, kms_ctx *kms )
{
    int len = 0, n = 0, j = 0, plane_index = -1;

    drmModePlane *plane = NULL;
    drmModePlaneRes *planeRes = NULL;
    drmModePropertyRes *prop = NULL;
    drmModeObjectProperties *props = NULL;

    drmSetClientCap(fd, DRM_CLIENT_CAP_UNIVERSAL_PLANES, 1);

    kms->primary_plane_id = kms->overlay_plane_id = -1;

    planeRes = drmModeGetPlaneResources( fd );
    if ( planeRes ) {

        for( n= 0; n < planeRes->count_planes; ++n ) {

            plane = drmModeGetPlane( fd, planeRes->planes[n] );

            if ( plane ) {

                props = drmModeObjectGetProperties( fd, planeRes->planes[n], DRM_MODE_OBJECT_PLANE );
                if ( props ) {

                    for( j= 0; j < props->count_props; ++j ) {

                        prop = drmModeGetProperty( fd, props->props[j] );
                        if ( prop ) {

                            len = strlen(prop->name);
                            if ( !strcmp( prop->name, "type") ) {

                                if ( ( props->prop_values[j] == DRM_PLANE_TYPE_PRIMARY ) && ( kms->primary_plane_id == -1 ) )
                                    kms->primary_plane_id = planeRes->planes[n];

                                else if ( ( props->prop_values[j] == DRM_PLANE_TYPE_OVERLAY ) && ( kms->overlay_plane_id == -1 ) )
                                    kms->overlay_plane_id = planeRes->planes[n];
                            }
                        }

                        drmModeFreeProperty( prop );
                    }
                }

                drmModeFreeObjectProperties( props );
            }

            drmModeFreePlane( plane );
        }

    }

    drmModeFreePlaneResources( planeRes );
}
