#pragma once
#include <gmock/gmock.h>
#include "HdmiCec.h"

class LibCCECImplMock : public LibCCECImpl {
public:
    virtual ~LibCCECImplMock() = default;

    MOCK_METHOD(void, init, (const char *name), (const, override));
    MOCK_METHOD(void, init, (), (const, override));
    MOCK_METHOD(void, term, (), (const, override));
    MOCK_METHOD(void, getPhysicalAddress, (unsigned int *physicalAddress), (const, override));
    MOCK_METHOD(int, addLogicalAddress, (const LogicalAddress &source), (const, override));
    MOCK_METHOD(int, getLogicalAddress, (int devType), (const, override));
};

class SystemAudioStatusImplMock : public SystemAudioStatusImpl {
public:
    virtual ~SystemAudioStatusImplMock() = default;

    MOCK_METHOD(int, toInt, (), (const, override));
};

class AudioStatusImplMock : public AudioStatusImpl {
public:
    virtual ~AudioStatusImplMock() = default;

    MOCK_METHOD(int, getAudioMuteStatus, (), (const, override));
    MOCK_METHOD(int, getAudioVolume,     (), (const, override));
};

class PowerStatusImplMock : public PowerStatusImpl {
public:
    virtual ~PowerStatusImplMock() = default;

    MOCK_METHOD(int, toInt, (), (const, override));
};

class AbortReasonImplMock : public AbortReasonImpl {
public:
    virtual ~AbortReasonImplMock() = default;

    MOCK_METHOD(int, toInt, (), (const, override));
};

class ShortAudioDescriptorImplMock : public ShortAudioDescriptorImpl {
public:
    virtual ~ShortAudioDescriptorImplMock() = default;

    MOCK_METHOD(uint32_t, getAudiodescriptor, (), (const, override));
};

class ConnectionImplMock : public ConnectionImpl {
public:
    virtual ~ConnectionImplMock() = default;

    MOCK_METHOD(void, open, (), (const, override));
    MOCK_METHOD(void, close, (), (const, override));
    MOCK_METHOD(void, addFrameListener, (FrameListener *listener), (const, override));
    MOCK_METHOD(void, ping, (const LogicalAddress &from, const LogicalAddress &to, const Throw_e &doThrow), (const, override));
    MOCK_METHOD(void, sendToAsync, (const LogicalAddress &to, const CECFrame &frame), (const, override));
    MOCK_METHOD(void, sendTo, (const LogicalAddress &to, const CECFrame &frame), (const, override));
    MOCK_METHOD(void, sendTo, (const LogicalAddress &to, const CECFrame &frame, int timeout), (const, override));
    MOCK_METHOD(void, poll, (const LogicalAddress &from, const Throw_e &doThrow), (const, override));
    MOCK_METHOD(void, sendAsync, (const CECFrame &frame), (const, override));
};

class LogicalAddressImplMock : public LogicalAddressImpl {
public:
    virtual ~LogicalAddressImplMock() = default;

    MOCK_METHOD(int, getType, (), (const, override));
};

class MessageEncoderMock : public MessageEncoderImpl{
public:
    virtual ~MessageEncoderMock() = default;

    MOCK_METHOD(CECFrame&, encode, (const DataBlock &m), (const, override));
    MOCK_METHOD(CECFrame&, encode, (const UserControlPressed &m), (const, override));
};

