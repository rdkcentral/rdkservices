#pragma once

#include <gmock/gmock.h>
#include "Operands.h"

class LibCCECImplMock : public LibCCECImpl {
public:
    virtual ~LibCCECImplMock() = default;

    MOCK_METHOD(void, init, (const char *name), (const, override));
    MOCK_METHOD(void, init, (), (const, override));
    MOCK_METHOD(void, term, (), (const, override));
    MOCK_METHOD(void, getPhysicalAddress, (unsigned int *physicalAddress), (const, override));
    MOCK_METHOD(int, addLogicalAddress, (const LogicalAddress &source), (const, override));
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

class PhysicalAddressImplMock : public PhysicalAddressImpl {
public:
    virtual ~PhysicalAddressImplMock() = default;

    MOCK_METHOD(uint8_t, getByteValue, (int), (const, override));
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