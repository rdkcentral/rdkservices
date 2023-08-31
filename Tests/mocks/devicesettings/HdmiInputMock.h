#pragma once

#include <gmock/gmock.h>

#include "hdmiIn.hpp"

class HdmiInputImplMock : public device::HdmiInputImpl {
public:
    virtual ~HdmiInputImplMock() = default;

    MOCK_METHOD(uint8_t, getNumberOfInputs, (), (const, override));
    MOCK_METHOD(bool, isPortConnected, (int8_t Port), (const, override));
    MOCK_METHOD(std::string, getCurrentVideoMode, (), (const, override));
    MOCK_METHOD(void, selectPort, (int8_t Port,bool audioMix, int videoPlane), (const, override));
    MOCK_METHOD(void, scaleVideo, (int32_t x, int32_t y, int32_t width, int32_t height), (const, override));
    MOCK_METHOD(void, getEDIDBytesInfo, (int iHdmiPort, std::vector<uint8_t> &edid), (const, override));
    MOCK_METHOD(void, getHDMISPDInfo, (int iHdmiPort, std::vector<uint8_t> &data), (const, override));
    MOCK_METHOD(void, setEdidVersion, (int iHdmiPort, int iEdidVersion), (const, override));
    MOCK_METHOD(void, getEdidVersion, (int iHdmiPort, int *iEdidVersion), (const, override));
    MOCK_METHOD(void, getHdmiALLMStatus, (int iHdmiPort, bool *allmStatus), (const, override));
    MOCK_METHOD(void, getSupportedGameFeatures, (std::vector<std::string> &featureList), (const, override));
    MOCK_METHOD(void, getAVLatency, (int *audio_output_delay,int *video_latency), (const, override));
};
