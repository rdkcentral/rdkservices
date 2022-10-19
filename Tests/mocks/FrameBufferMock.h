#pragma once

#include <gmock/gmock.h>

#include "FrameBuffer.h"

class FrameBufferApiImplMock : public FrameBufferApiImpl {
public:
    virtual ~FrameBufferApiImplMock() = default;

    MOCK_METHOD(ErrCode, fbCreate, (FBContext** fbctx), (override));
    MOCK_METHOD(ErrCode, fbInit, (FBContext* fbctx, VncServerFramebufferAPI* server, void* serverctx), (override));
    MOCK_METHOD(ErrCode, fbDestroy, (FBContext* fbctx), (override));

    MOCK_METHOD(vnc_uint8_t*, fbGetFramebuffer, (FBContext* fbctx), (override));
    MOCK_METHOD(PixelFormat*, fbGetPixelFormat, (FBContext* fbctx), (override));

    MOCK_METHOD(vnc_uint16_t, fbGetWidth, (FBContext* fbctx), (override));
    MOCK_METHOD(vnc_uint16_t, fbGetHeight, (FBContext* fbctx), (override));
    MOCK_METHOD(vnc_uint16_t, fbGetStride, (FBContext* fbctx), (override));
};
