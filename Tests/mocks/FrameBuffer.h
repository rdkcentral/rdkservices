#pragma once

typedef enum {
    ErrNone = 0,
    ErrNotSupported,
    ErrInvalidArguments,
    ErrResource,
    ErrUnknown
} ErrCode;

// Common API type, shared by both framebuffer and input APIs
typedef void VNCServerLogMessage(void* ctx_, const char* fmt, ...);

typedef int8_t vnc_int8_t;
typedef int16_t vnc_int16_t;
typedef int32_t vnc_int32_t;
typedef int64_t vnc_int64_t;
typedef uint8_t vnc_uint8_t;
typedef uint16_t vnc_uint16_t;
typedef uint32_t vnc_uint32_t;
typedef uint64_t vnc_uint64_t;

typedef vnc_uint8_t vnc_bool_t;

#define vnc_true 1
#define vnc_false 0

typedef struct {
    vnc_uint8_t bitsPerPixel;
    vnc_uint8_t depth;
    vnc_bool_t bigEndian;
    vnc_bool_t trueColour;
    vnc_uint16_t redMax;
    vnc_uint16_t greenMax;
    vnc_uint16_t blueMax;
    vnc_uint8_t redShift;
    vnc_uint8_t greenShift;
    vnc_uint8_t blueShift;
} PixelFormat;

typedef struct {
    vnc_uint8_t reds[256];
    vnc_uint8_t greens[256];
    vnc_uint8_t blues[256];
} Palette;

typedef enum {
    UI,
    Video,
    Composite,
    ScaledComposite,
    Invalid,
} PlaneType;

typedef void* FBContext;

typedef vnc_bool_t VNCServerFramebufferUpdateReady(void* ctx);
typedef void VNCServerFramebufferDetailsChanged(void* ctx, vnc_uint8_t* fb, vnc_uint16_t width, vnc_uint16_t height, vnc_uint16_t stride, PixelFormat* pf);
typedef void VNCServerPaletteChanged(void* ctx, Palette* palette);

typedef struct {
    VNCServerFramebufferUpdateReady* framebufferUpdateReady;
    VNCServerFramebufferDetailsChanged* framebufferDetailsChanged;
    VNCServerPaletteChanged* paletteChanged;
    VNCServerLogMessage* logMsg;
} VncServerFramebufferAPI;

class FrameBufferApiImpl {
public:
    virtual ~FrameBufferApiImpl() = default;

    virtual ErrCode fbCreate(FBContext** fbctx) = 0;
    virtual ErrCode fbInit(FBContext* fbctx, VncServerFramebufferAPI* server, void* serverctx) = 0;
    virtual ErrCode fbDestroy(FBContext* fbctx) = 0;

    virtual vnc_uint8_t* fbGetFramebuffer(FBContext* fbctx) = 0;
    virtual PixelFormat* fbGetPixelFormat(FBContext* fbctx) = 0;

    virtual vnc_uint16_t fbGetWidth(FBContext* fbctx) = 0;
    virtual vnc_uint16_t fbGetHeight(FBContext* fbctx) = 0;
    virtual vnc_uint16_t fbGetStride(FBContext* fbctx) = 0;
};

class FrameBufferApi {
public:
    static FrameBufferApi& getInstance()
    {
        static FrameBufferApi instance;
        return instance;
    }

    FrameBufferApiImpl* impl;

    static ErrCode fbCreate(FBContext** fbctx)
    {
        return getInstance().impl->fbCreate(fbctx);
    }

    static ErrCode fbInit(FBContext* fbctx, VncServerFramebufferAPI* server, void* serverctx)
    {
        return getInstance().impl->fbInit(fbctx, server, serverctx);
    }

    static ErrCode fbDestroy(FBContext* fbctx)
    {
        return getInstance().impl->fbDestroy(fbctx);
    }

    static vnc_uint8_t* fbGetFramebuffer(FBContext* fbctx)
    {
        return getInstance().impl->fbGetFramebuffer(fbctx);
    }

    static PixelFormat* fbGetPixelFormat(FBContext* fbctx)
    {
        return getInstance().impl->fbGetPixelFormat(fbctx);
    }

    static vnc_uint16_t fbGetWidth(FBContext* fbctx)
    {
        return getInstance().impl->fbGetWidth(fbctx);
    }

    static vnc_uint16_t fbGetHeight(FBContext* fbctx)
    {
        return getInstance().impl->fbGetHeight(fbctx);
    }

    static vnc_uint16_t fbGetStride(FBContext* fbctx)
    {
        return getInstance().impl->fbGetStride(fbctx);
    }
};

constexpr auto fbCreate = &FrameBufferApi::fbCreate;
constexpr auto fbInit = &FrameBufferApi::fbInit;
constexpr auto fbDestroy = &FrameBufferApi::fbDestroy;
constexpr auto fbGetFramebuffer = &FrameBufferApi::fbGetFramebuffer;
constexpr auto fbGetPixelFormat = &FrameBufferApi::fbGetPixelFormat;
constexpr auto fbGetWidth = &FrameBufferApi::fbGetWidth;
constexpr auto fbGetHeight = &FrameBufferApi::fbGetHeight;
constexpr auto fbGetStride = &FrameBufferApi::fbGetStride;
