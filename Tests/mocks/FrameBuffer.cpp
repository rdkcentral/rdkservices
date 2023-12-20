/**
* If not stated otherwise in this file or this component's LICENSE
* file the following copyright and licenses apply:
*
* Copyright 2023 Synamedia
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

#include "FrameBuffer.h"
#include <gmock/gmock.h>


FrameBufferApiImpl* FrameBufferApi::impl = nullptr;

FrameBufferApi::FrameBufferApi() {}

void FrameBufferApi::setImpl(FrameBufferApiImpl* newImpl)
{
    // Handles both resetting 'impl' to nullptr and assigning a new value to 'impl'
    EXPECT_TRUE ((nullptr == impl) || (nullptr == newImpl));
    impl = newImpl;
}

ErrCode FrameBufferApi::fbCreate(FBContext** fbctx)
{
    EXPECT_NE(impl, nullptr);
    return impl->fbCreate(fbctx);
}

ErrCode FrameBufferApi::fbInit(FBContext* fbctx, VncServerFramebufferAPI* server, void* serverctx)
{
    EXPECT_NE(impl, nullptr);
    return impl->fbInit(fbctx, server, serverctx);
}

ErrCode FrameBufferApi::fbDestroy(FBContext* fbctx)
{
    EXPECT_NE(impl, nullptr);
    return impl->fbDestroy(fbctx);
}

vnc_uint8_t* FrameBufferApi::fbGetFramebuffer(FBContext* fbctx)
{
    EXPECT_NE(impl, nullptr);
    return impl->fbGetFramebuffer(fbctx);
}

PixelFormat* FrameBufferApi::fbGetPixelFormat(FBContext* fbctx)
{
    EXPECT_NE(impl, nullptr);
    return impl->fbGetPixelFormat(fbctx);
}

vnc_uint16_t FrameBufferApi::fbGetWidth(FBContext* fbctx)
{
    EXPECT_NE(impl, nullptr);
    return impl->fbGetWidth(fbctx);
}

vnc_uint16_t FrameBufferApi::fbGetHeight(FBContext* fbctx)
{
    EXPECT_NE(impl, nullptr);
    return impl->fbGetHeight(fbctx);
}

vnc_uint16_t FrameBufferApi::fbGetStride(FBContext* fbctx)
{
    EXPECT_NE(impl, nullptr);
    return impl->fbGetStride(fbctx);
}

ErrCode (*fbCreate)(FBContext**) = &FrameBufferApi::fbCreate;
ErrCode (*fbInit)(FBContext*,VncServerFramebufferAPI*,void*) = &FrameBufferApi::fbInit;
ErrCode (*fbDestroy)(FBContext*) = &FrameBufferApi::fbDestroy;
vnc_uint8_t* (*fbGetFramebuffer)(FBContext*) = &FrameBufferApi::fbGetFramebuffer;
PixelFormat* (*fbGetPixelFormat)(FBContext*) = &FrameBufferApi::fbGetPixelFormat;
vnc_uint16_t (*fbGetWidth)(FBContext*) = &FrameBufferApi::fbGetWidth;
vnc_uint16_t (*fbGetHeight)(FBContext*) = &FrameBufferApi::fbGetHeight;
vnc_uint16_t (*fbGetStride)(FBContext*) = &FrameBufferApi::fbGetStride;

