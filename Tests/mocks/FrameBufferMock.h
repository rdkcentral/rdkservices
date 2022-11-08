#pragma once

#include <gmock/gmock.h>

class FrameBufferApiMock {
public:
    static FrameBufferApiMock* instance{ nullptr };
    FrameBufferApiMock()
    {
        instance = this;
    }
    virtual ~FrameBufferApiMock()
    {
        instance = nullptr;
    }

    MOCK_METHOD(ErrCode, fbCreate, (FBContext * *fbctx));
    MOCK_METHOD(ErrCode, fbInit, (FBContext * fbctx, VncServerFramebufferAPI* server, void* serverctx));
    MOCK_METHOD(ErrCode, fbDestroy, (FBContext * fbctx));
    MOCK_METHOD(vnc_uint8_t*, fbGetFramebuffer, (FBContext * fbctx));
    MOCK_METHOD(PixelFormat*, fbGetPixelFormat, (FBContext * fbctx));
    MOCK_METHOD(vnc_uint16_t, fbGetWidth, (FBContext * fbctx));
    MOCK_METHOD(vnc_uint16_t, fbGetHeight, (FBContext * fbctx));
    MOCK_METHOD(vnc_uint16_t, fbGetStride, (FBContext * fbctx));

    static ErrCode _fbCreate(FBContext** fbctx)
    {
        return instance->fbCreate(fbctx);
    }

    static ErrCode _fbInit(FBContext* fbctx, VncServerFramebufferAPI* server, void* serverctx)
    {
        return instance->fbInit(fbctx, server, serverctx);
    }

    static ErrCode _fbDestroy(FBContext* fbctx)
    {
        return instance->fbDestroy(fbctx);
    }

    static vnc_uint8_t* _fbGetFramebuffer(FBContext* fbctx)
    {
        return instance->fbGetFramebuffer(fbctx);
    }

    static PixelFormat* _fbGetPixelFormat(FBContext* fbctx)
    {
        return instance->fbGetPixelFormat(fbctx);
    }

    static vnc_uint16_t _fbGetWidth(FBContext* fbctx)
    {
        return instance->fbGetWidth(fbctx);
    }

    static vnc_uint16_t _fbGetHeight(FBContext* fbctx)
    {
        return instance->fbGetHeight(fbctx);
    }

    static vnc_uint16_t _fbGetStride(FBContext* fbctx)
    {
        return instance->fbGetStride(fbctx);
    }
};

constexpr auto fbCreate = &FrameBufferApiMock::_fbCreate;
constexpr auto fbInit = &FrameBufferApiMock::_fbInit;
constexpr auto fbDestroy = &FrameBufferApiMock::_fbDestroy;
constexpr auto fbGetFramebuffer = &FrameBufferApiMock::_fbGetFramebuffer;
constexpr auto fbGetPixelFormat = &FrameBufferApiMock::_fbGetPixelFormat;
constexpr auto fbGetWidth = &FrameBufferApiMock::_fbGetWidth;
constexpr auto fbGetHeight = &FrameBufferApiMock::_fbGetHeight;
constexpr auto fbGetStride = &FrameBufferApiMock::_fbGetStride;
