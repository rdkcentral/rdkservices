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

#include "HdmiCec.h"
#include <gmock/gmock.h>

MessageDecoderImpl* MessageDecoder::impl = nullptr;

MessageDecoder::MessageDecoder(MessageProcessor& proc)
    : processor(proc) {}

void MessageDecoder::setImpl(MessageDecoderImpl* newImpl) {
    // Handles both resetting 'impl' to nullptr and assigning a new value to 'impl'
    EXPECT_TRUE ((nullptr == impl) || (nullptr == newImpl));
    impl = newImpl;
}

void MessageDecoder::decode(const CECFrame& in) {
    EXPECT_NE(impl, nullptr);
    return impl->decode(in);
}


ConnectionImpl* Connection::impl = nullptr;

Connection::Connection(const LogicalAddress& source, bool opened, const std::string& name) {}

Connection& Connection::getInstance() {
    static Connection instance;
    return instance;
}

void Connection::setImpl(ConnectionImpl* newImpl) {
    // Handles both resetting 'impl' to nullptr and assigning a new value to 'impl'
    EXPECT_TRUE ((nullptr == impl) || (nullptr == newImpl));
    impl = newImpl;
}

void Connection::open() {
    EXPECT_NE(impl, nullptr);
    return impl->open();
}

void Connection::close() {
    EXPECT_NE(impl, nullptr);
    return impl->close();
}
void Connection::addFrameListener(FrameListener* listener) {
    EXPECT_NE(impl, nullptr);
    return impl->addFrameListener(listener);
}

void Connection::ping(const LogicalAddress& from, const LogicalAddress& to, const Throw_e& doThrow) {
    EXPECT_NE(impl, nullptr);
    return impl->ping(from, to, doThrow);
}

void Connection::sendToAsync(const LogicalAddress& to, const CECFrame& frame) {
    EXPECT_NE(impl, nullptr);
    return impl->sendToAsync(to, frame);
}

void Connection::sendTo(const LogicalAddress& to, const CECFrame& frame) {
    EXPECT_NE(impl, nullptr);
    return impl->sendTo(to, frame);
}

void Connection::sendTo(const LogicalAddress& to, const CECFrame& frame, int timeout) {
    EXPECT_NE(impl, nullptr);
    return impl->sendTo(to, frame, timeout);
}

void Connection::poll(const LogicalAddress& from, const Throw_e& doThrow) {
    EXPECT_NE(impl, nullptr);
    return impl->poll(from, doThrow);
}

void Connection::sendAsync(const CECFrame &frame){
     EXPECT_NE(impl, nullptr);
     return impl->sendAsync(frame);
}

void Connection::setSource(LogicalAddress& from) {
}


LibCCECImpl* LibCCEC::impl = nullptr;

LibCCEC& LibCCEC::getInstance() {
    static LibCCEC instance;
    return instance;
}

void LibCCEC::setImpl(LibCCECImpl* newImpl) {
    // Handles both resetting 'impl' to nullptr and assigning a new value to 'impl'
    EXPECT_TRUE ((nullptr == impl) || (nullptr == newImpl));
    impl = newImpl;
}
void LibCCEC::init(const char* name) {
    EXPECT_NE(impl, nullptr);
    impl->init(name);
}

void LibCCEC::init() {
    EXPECT_NE(impl, nullptr);
    impl->init();
}

void LibCCEC::term() {
}

void LibCCEC::getPhysicalAddress(uint32_t* physicalAddress) {
    EXPECT_NE(impl, nullptr);
    impl->getPhysicalAddress(physicalAddress);
}

int LibCCEC::addLogicalAddress(const LogicalAddress& source) {
    EXPECT_NE(impl, nullptr);
    return impl->addLogicalAddress(source);
}

int LibCCEC::getLogicalAddress(int devType) {
    EXPECT_NE(impl, nullptr);
    return impl->getLogicalAddress(devType);
}

MessageEncoderImpl* MessageEncoder::impl = nullptr;

void MessageEncoder::setImpl(MessageEncoderImpl* newImpl) {
        // Handles both resetting 'impl' to nullptr and assigning a new value to 'impl'
    EXPECT_TRUE ((nullptr == impl) || (nullptr == newImpl));
    impl = newImpl;
}

CECFrame& MessageEncoder::encode(const UserControlPressed m) {
    EXPECT_NE(impl, nullptr);
    return impl->encode(m);
}

CECFrame& MessageEncoder::encode(const DataBlock m) {
    EXPECT_NE(impl, nullptr);
    return impl->encode(m);
}
