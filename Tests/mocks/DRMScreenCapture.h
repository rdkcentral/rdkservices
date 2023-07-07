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
#pragma once

typedef struct DRMScreenCapture_s {
	void *context;
	uint32_t width;
	uint32_t height;
	uint32_t pitch;
	uint8_t bpp;
}DRMScreenCapture;


class DRMScreenCaptureApiImpl {
public:
    virtual ~DRMScreenCaptureApiImpl() = default;

    virtual DRMScreenCapture* Init() = 0;
    virtual bool GetScreenInfo(DRMScreenCapture* handle) = 0;
    virtual bool ScreenCapture(DRMScreenCapture* handle, uint8_t* output, uint32_t size) = 0;
    virtual bool Destroy(DRMScreenCapture* handle) = 0;
};

class DRMScreenCaptureApi {
public:
    static DRMScreenCaptureApi& getInstance()
    {
        static DRMScreenCaptureApi instance;
        return instance;
    }

    DRMScreenCaptureApiImpl* impl;

    static DRMScreenCapture* Init()
    {
        return getInstance().impl->Init();
    }

    static bool GetScreenInfo(DRMScreenCapture* handle)
    {
        return getInstance().impl->GetScreenInfo(handle);
    }

    static bool ScreenCapture(DRMScreenCapture* handle, uint8_t* output, uint32_t size)
    {
        return getInstance().impl->ScreenCapture(handle, output, size);
    }

    static bool Destroy(DRMScreenCapture* handle)
    {
        return getInstance().impl->Destroy(handle);
    }

};

DRMScreenCapture* DRMScreenCapture_Init() {
    return DRMScreenCaptureApi::Init();
}

bool DRMScreenCapture_GetScreenInfo(DRMScreenCapture* handle) {
    return DRMScreenCaptureApi::GetScreenInfo(handle);
}

bool DRMScreenCapture_ScreenCapture(DRMScreenCapture* handle, uint8_t* output, uint32_t size) {
    return DRMScreenCaptureApi::ScreenCapture(handle, output, size);
}

bool DRMScreenCapture_Destroy(DRMScreenCapture* handle) {
    return DRMScreenCaptureApi::Destroy(handle);
}
