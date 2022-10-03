#pragma once

#include <gmock/gmock.h>

#include "framebuffer-api.h"

class FrameBufferApiImplMock : public FrameBufferApiImpl {
public:
    virtual ~FrameBufferApiImplMock() = default;

    MOCK_METHOD(ErrCode, fbCreate, (FBContext** fbctx));
    MOCK_METHOD(ErrCode, fbInit, (FBContext* fbctx, VncServerFramebufferAPI* server, void* serverctx));
    MOCK_METHOD(ErrCode, fbDestroy, (FBContext* fbctx));

    MOCK_METHOD(vnc_uint8_t*, fbGetFramebuffer, (FBContext* fbctx));
    MOCK_METHOD(PixelFormat*, fbGetPixelFormat, (FBContext* fbctx));

    MOCK_METHOD(vnc_uint16_t, fbGetWidth, (FBContext* fbctx));
    MOCK_METHOD(vnc_uint16_t, fbGetHeight, (FBContext* fbctx));
    MOCK_METHOD(vnc_uint16_t, fbGetStride, (FBContext* fbctx));
};
