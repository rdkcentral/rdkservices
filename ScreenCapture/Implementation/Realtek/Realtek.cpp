/**
* If not stated otherwise in this file or this component's LICENSE
* file the following copyright and licenses apply:
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
**/
#include <iostream>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include "kms.h"
#include "Realtek.h"

#define DEFAULT_DEVICE "/dev/dri/card0"

using namespace std;

typedef struct DRMContext_s {
	int fd;
	kms_ctx *kms;
	uint64_t offset;
} DRMContext;


DRMScreenCapture* DRMScreenCapture_Init() {
	DRMScreenCapture *handle = nullptr;
	DRMContext *context = nullptr;

	do {
		// init variables
		handle = (DRMScreenCapture*)calloc(1, sizeof(DRMScreenCapture));
		if(!handle) {
			cout << "[SCREENCAP] fail to calloc DRMScreenCapture" << endl;
			break;
		}
		context = (DRMContext*)calloc(1, sizeof(DRMContext));
		if(!context) {
			cout << "[SCREENCAP] fail to calloc DRMContext" << endl;
			break;
		}
		handle->context = (void*) context;

		return handle;
	} while(0);

	// fail to calloc
	free(handle);
	free(context);
	return nullptr;
}

bool DRMScreenCapture_GetScreenInfo(DRMScreenCapture* handle) {
	DRMContext *context;
	bool ret = true;
	drmModeFB *fb = nullptr;

	do {
		if(!handle || !handle->context) {
			ret = false;
			cout << "[SCREENCAP] null input parameter" << endl;
			break;
		}
		context = (DRMContext*) handle->context;

		// open drm device to get screen information
		int retryCount = 0;
		drmModePlane *plane = nullptr;
		struct drm_mode_map_dumb map = {};

		context->fd = open(DEFAULT_DEVICE, O_RDWR);
		if(!context->fd) {
			cout << "[SCREENCAP] fail to open " <<  DEFAULT_DEVICE << endl;
			ret = false;
			break;
		}

		context->kms = kms_setup(context->fd);
		if(!context->kms || !context->kms->crtc) {
			cout << "[SCREENCAP] kms_setup fail " <<  endl;
			ret = false;
			break;
		}
		cout << "Choose mode: " << context->kms->current_info.hdisplay << "x" <<  context->kms->current_info.vdisplay
			<< "@" << context->kms->current_info.vrefresh << endl;

		/* Get primary plane */
		kms_get_plane(context->fd, context->kms);
		plane = drmModeGetPlane(context->fd, context->kms->primary_plane_id );
		if(!plane) {
			cout << "[SCREENCAP] fail to drmModeGetPlane" <<  endl;
			ret = false;
			break;
		}

		// get primary buffer
		fb = drmModeGetFB(context->fd, plane->fb_id);
		while(!fb) {
			// try again
			cout << "[SCREENCAP] try get primary buffer again" << endl;
			kms_get_plane(context->fd, context->kms);
			plane = drmModeGetPlane(context->fd, context->kms->primary_plane_id );
			if(!plane) {
				cout << "[SCREENCAP] fail to drmModeGetPlane" <<  endl;
				ret = false;
				break;
			}

			fb = drmModeGetFB(context->fd, plane->fb_id);
			if(retryCount > 2) {
				break;
			}
			retryCount++;
		}
		if(retryCount > 2) {
			cout << "[SCREENCAP] fail to drmModeGetFB" << endl;
			ret = false;
			break;
		}
		handle->width = fb->width;
		handle->height = fb->height;
		handle->bpp = fb->bpp;
		handle->pitch = fb->pitch;

		cout << "[SCREENCAP] width : " << fb->width << endl;
		cout << "[SCREENCAP] height : " << fb->height << endl;
		cout << "[SCREENCAP] bpp : " << fb->bpp << endl;
		cout << "[SCREENCAP] pitch : " << fb->pitch << endl;

		if (32 != handle->bpp) {
			cout << "[SCREENCAP] Unsupported bits per pixel: " << handle->bpp << endl;
			ret = false;
			break;
		}
		map.handle = fb->handle;
		int drmRet = drmIoctl(context->fd, DRM_IOCTL_MODE_MAP_DUMB, &map);
		if(drmRet) {
			cout << "[SCREENCAP] drmIoctl fail, ret=" << drmRet << endl;
			ret = false;
			break;
		}
		context->offset = map.offset;
		cout << "[SCREENCAP] offset : " << map.offset << endl;
	} while(false);

	if(fb)
		drmModeFreeFB(fb);

	return ret;

}

bool DRMScreenCapture_ScreenCapture(DRMScreenCapture* handle, uint8_t* output, uint32_t bufSize) {
	DRMContext *context;
	bool ret = true;

	do {
		if(!handle || !handle->context || !output) {
			cout << "[SCREENCAP] null input parameter" << endl;
			ret = false;
			break;
		}

		context = (DRMContext*) handle->context;
		uint32_t size = handle->pitch * handle->height;
		if(bufSize < size) {
			// buffer size not match
			ret = false;
			break;
		}

		// copy frame
		void *vaddr = NULL;
		vaddr =(void*) mmap(NULL, size, PROT_READ , MAP_SHARED, context->fd, context->offset) ;
		memcpy(output,(unsigned char*)vaddr, size);
		munmap(vaddr, size);

	} while(false);

	return ret;
}

bool DRMScreenCapture_Destroy(DRMScreenCapture* handle) {
	DRMContext *context;
	if(!handle) {
		cout << "[SCREENCAP] null input parameter" << endl;
		return false;
	}

	context = (DRMContext*) handle->context;
	if(context) {
		if(context->kms) {
			kms_cleanup_context(context->kms);
			free(context->kms);
			context->kms = nullptr;
		}

		if(context->fd) {
			close(context->fd);
			context->fd = 0;
		}
		free(context);
		handle->context = nullptr;
	}
	free(handle);
	return true;
}
