#pragma once

#include <gmock/gmock.h>
#include "DRMScreenCapture.h"

class DRMScreenCaptureApiImplMock : public DRMScreenCaptureApiImpl {
public:
    virtual ~DRMScreenCaptureApiImplMock() = default;

    MOCK_METHOD(DRMScreenCapture*, Init, (), (override));
    MOCK_METHOD(bool, GetScreenInfo, (DRMScreenCapture* handle), (override));
    MOCK_METHOD(bool, ScreenCapture, (DRMScreenCapture* handle, uint8_t* output, uint32_t size), (override));
    MOCK_METHOD(bool, Destroy, (DRMScreenCapture* handle), (override));
};
