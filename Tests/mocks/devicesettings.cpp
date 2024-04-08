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

#include "devicesettings.h"
#include <gmock/gmock.h>

namespace device
{
AudioOutputPortImpl* AudioOutputPort::impl = nullptr;

AudioOutputPort::AudioOutputPort() {}

void AudioOutputPort::setImpl(AudioOutputPortImpl* newImpl) {
    // Handles both resetting 'impl' to nullptr and assigning a new value to 'impl'
    EXPECT_TRUE ((nullptr == impl) || (nullptr == newImpl));
    impl = newImpl;
}

AudioOutputPort& AudioOutputPort::getInstance() {
    static AudioOutputPort instance;
    return instance;
}

const std::string& AudioOutputPort::getName() const {
    EXPECT_NE(impl, nullptr);
    return impl->getName();
}

std::vector<std::string> AudioOutputPort::getMS12AudioProfileList() const {
    EXPECT_NE(impl, nullptr);
    return impl->getMS12AudioProfileList();
}

void AudioOutputPort::getAudioCapabilities(int* capabilities) {
    EXPECT_NE(impl, nullptr);
    impl->getAudioCapabilities(capabilities);
}

void AudioOutputPort::getMS12Capabilities(int* capabilities) {
    EXPECT_NE(impl, nullptr);
    impl->getMS12Capabilities(capabilities);
}

bool AudioOutputPort::isAudioMSDecode() {
    EXPECT_NE(impl, nullptr);
    return impl->isAudioMSDecode();
}

CompositeInputImpl* CompositeInput::impl = nullptr;

CompositeInput::CompositeInput() {}

void CompositeInput::setImpl(CompositeInputImpl* newImpl) {
        // Handles both resetting 'impl' to nullptr and assigning a new value to 'impl'
    EXPECT_TRUE ((nullptr == impl) || (nullptr == newImpl));
    impl = newImpl;
}

CompositeInput& CompositeInput::getInstance() {
    static CompositeInput instance;
    return instance;
}

uint8_t CompositeInput::getNumberOfInputs() const {
    EXPECT_NE(impl, nullptr);
    return impl->getNumberOfInputs();
}

bool CompositeInput::isPortConnected(int8_t port) const {
    EXPECT_NE(impl, nullptr);
    return impl->isPortConnected(port);
}

void CompositeInput::selectPort(int8_t port) const {
    EXPECT_NE(impl, nullptr);
    impl->selectPort(port);
}

void CompositeInput::scaleVideo(int32_t x, int32_t y, int32_t width, int32_t height) const {
    EXPECT_NE(impl, nullptr);
    impl->scaleVideo(x, y, width, height);
}


HdmiInputImpl* HdmiInput::impl = nullptr;

HdmiInput::HdmiInput() {}

void HdmiInput::setImpl(HdmiInputImpl* newImpl) {
        // Handles both resetting 'impl' to nullptr and assigning a new value to 'impl'
    EXPECT_TRUE ((nullptr == impl) || (nullptr == newImpl));
    impl = newImpl;
}

HdmiInput& HdmiInput::getInstance() {
    static HdmiInput instance;
    return instance;
}

uint8_t HdmiInput::getNumberOfInputs() const {
    EXPECT_NE(impl, nullptr);
    return impl->getNumberOfInputs();
}

bool HdmiInput::isPortConnected(int8_t port) const {
    EXPECT_NE(impl, nullptr);
    return impl->isPortConnected(port);
}

std::string HdmiInput::getCurrentVideoMode() const {
    EXPECT_NE(impl, nullptr);
    return impl->getCurrentVideoMode();
}

/*void HdmiInput::selectPort(int8_t port) const {
    EXPECT_NE(impl, nullptr);
    impl->selectPort(port);
}*/

void HdmiInput::selectPort(int8_t Port,bool audioMix , int videoPlane ,bool topMost ) const {
    EXPECT_NE(impl, nullptr);
    impl->selectPort(Port,audioMix,videoPlane,topMost);
}

void HdmiInput::scaleVideo(int32_t x, int32_t y, int32_t width, int32_t height) const {
    EXPECT_NE(impl, nullptr);
    impl->scaleVideo(x, y, width, height);
}

void HdmiInput::getEDIDBytesInfo(int iHdmiPort, std::vector<uint8_t>& edid) const {
    EXPECT_NE(impl, nullptr);
    impl->getEDIDBytesInfo(iHdmiPort, edid);
}

void HdmiInput::getHDMISPDInfo(int iHdmiPort, std::vector<uint8_t>& data) const {
    EXPECT_NE(impl, nullptr);
    impl->getHDMISPDInfo(iHdmiPort, data);
}

void HdmiInput::setEdidVersion(int iHdmiPort, int iEdidVersion) const {
    EXPECT_NE(impl, nullptr);
    impl->setEdidVersion(iHdmiPort, iEdidVersion);
}

void HdmiInput::getEdidVersion(int iHdmiPort, int* iEdidVersion) const {
    EXPECT_NE(impl, nullptr);
    impl->getEdidVersion(iHdmiPort, iEdidVersion);
}

void HdmiInput::getHdmiALLMStatus(int iHdmiPort, bool* allmStatus) const {
    EXPECT_NE(impl, nullptr);
    impl->getHdmiALLMStatus(iHdmiPort, allmStatus);
}

void HdmiInput::getSupportedGameFeatures(std::vector<std::string>& featureList) const {
    EXPECT_NE(impl, nullptr);
    impl->getSupportedGameFeatures(featureList);
}

    void HdmiInput::setEdid2AllmSupport(int iport, bool allmSupport) const {
    EXPECT_NE(impl, nullptr);
    impl->setEdid2AllmSupport(iport,allmSupport);
}

    void HdmiInput::getAVLatency(int *audio_output_delay, int *video_latency) const {
    EXPECT_NE(impl, nullptr);
    impl->getAVLatency(audio_output_delay,video_latency);
}


    void HdmiInput::getEdid2AllmSupport(int iport, bool *allmSupport) const {
    EXPECT_NE(impl, nullptr);
    impl->getEdid2AllmSupport(iport,allmSupport);
}

SleepModeImpl* SleepMode::impl = nullptr;

SleepMode::SleepMode() {}

void SleepMode::setImpl(SleepModeImpl* newImpl) {
    // Handles both resetting 'impl' to nullptr and assigning a new value to 'impl'
    EXPECT_TRUE ((nullptr == impl) || (nullptr == newImpl));
    impl = newImpl;
}

SleepMode& SleepMode::getInstance() {
    static SleepMode instance;
    return instance;
}

SleepMode& SleepMode::getInstance(int id) {
    EXPECT_NE(impl, nullptr);
    return impl->getInstanceById(id);
}

SleepMode& SleepMode::getInstance(const std::string &name) {
    EXPECT_NE(impl, nullptr);
    return impl->getInstanceByName(name);
}

List<SleepMode> SleepMode::getSleepModes() {
    EXPECT_NE(impl, nullptr);
    return impl->getSleepModes();
}

const std::string& SleepMode::toString() const {
    EXPECT_NE(impl, nullptr);
    return impl->toString();
}

VideoDeviceImpl* VideoDevice::impl = nullptr;

VideoDevice::VideoDevice() {}

void VideoDevice::setImpl(VideoDeviceImpl* newImpl) {
    // Handles both resetting 'impl' to nullptr and assigning a new value to 'impl'
    EXPECT_TRUE ((nullptr == impl) || (nullptr == newImpl));
    impl = newImpl;
}

VideoDevice& VideoDevice::getInstance() {
    static VideoDevice instance;
    return instance;
}

int VideoDevice::getFRFMode(int* frfmode) const {
    EXPECT_NE(impl, nullptr);
    return impl->getFRFMode(frfmode);
}

int VideoDevice::setFRFMode(int frfmode) const {
    EXPECT_NE(impl, nullptr);
    return impl->setFRFMode(frfmode);
}

int VideoDevice::getCurrentDisframerate(char* framerate) const {
    EXPECT_NE(impl, nullptr);
    return impl->getCurrentDisframerate(framerate);
}

int VideoDevice::setDisplayframerate(const char* framerate) const {
    EXPECT_NE(impl, nullptr);
    return impl->setDisplayframerate(framerate);
}


VideoResolutionImpl* VideoResolution::impl = nullptr;

void VideoResolution::setImpl(VideoResolutionImpl* newImpl) {
    // Handles both resetting 'impl' to nullptr and assigning a new value to 'impl'
    EXPECT_TRUE ((nullptr == impl) || (nullptr == newImpl));
    impl = newImpl;
}
const std::string& VideoResolution::getName() const {
    return impl->getName();
}

VideoOutputPortTypeImpl* VideoOutputPortType::impl = nullptr;
VideoOutputPortType::VideoOutputPortType(){}

void VideoOutputPortType::setImpl(VideoOutputPortTypeImpl* newImpl) {
    // Handles both resetting 'impl' to nullptr and assigning a new value to 'impl'
    EXPECT_TRUE ((nullptr == impl) || (nullptr == newImpl));
    impl = newImpl;
}

int VideoOutputPortType::getId() const {
    EXPECT_NE(impl, nullptr);
    return impl->getId();
}

const List<VideoResolution> VideoOutputPortType::getSupportedResolutions() const {
    EXPECT_NE(impl, nullptr);
    return impl->getSupportedResolutions();
}


DisplayImpl* Display::impl = nullptr;


void Display::setImpl(DisplayImpl* newImpl) {
        // Handles both resetting 'impl' to nullptr and assigning a new value to 'impl'
    EXPECT_TRUE ((nullptr == impl) || (nullptr == newImpl));
    impl = newImpl;
}

Display& Display::getInstance() {
    static Display instance;
    return instance;
}

void Display::getEDIDBytes(std::vector<uint8_t>& edid) {
    EXPECT_NE(impl, nullptr);
    impl->getEDIDBytes(edid);
}

VideoOutputPortImpl* VideoOutputPort::impl = nullptr;

VideoOutputPort::VideoOutputPort() {}

void VideoOutputPort::setImpl(VideoOutputPortImpl* newImpl) {
    // Handles both resetting 'impl' to nullptr and assigning a new value to 'impl'
    EXPECT_TRUE ((nullptr == impl) || (nullptr == newImpl));
    impl = newImpl;
}

VideoOutputPort& VideoOutputPort::getInstance() {
    static VideoOutputPort instance;
    return instance;
}

const VideoOutputPortType& VideoOutputPort::getType() const {
    EXPECT_NE(impl, nullptr);
    return impl->getType();
}

const std::string& VideoOutputPort::getName() const {
    EXPECT_NE(impl, nullptr);
    return impl->getName();
}

const VideoResolution& VideoOutputPort::getDefaultResolution() const {
    EXPECT_NE(impl, nullptr);
    return impl->getDefaultResolution();
}

int VideoOutputPort::getHDCPProtocol() {
    EXPECT_NE(impl, nullptr);
    return impl->getHDCPProtocol();
}

int VideoOutputPort::getHDCPReceiverProtocol() {
    EXPECT_NE(impl, nullptr);
    return impl->getHDCPReceiverProtocol();
}

int VideoOutputPort::getHDCPCurrentProtocol() {
    EXPECT_NE(impl, nullptr);
    return impl->getHDCPCurrentProtocol();
}

int VideoOutputPort::getHDCPStatus() {
    EXPECT_NE(impl, nullptr);
    return impl->getHDCPStatus();
}

AudioOutputPort& VideoOutputPort::getAudioOutputPort() {
    EXPECT_NE(impl, nullptr);
    return impl->getAudioOutputPort();
}

bool VideoOutputPort::isDisplayConnected() {
    EXPECT_NE(impl, nullptr);
    return impl->isDisplayConnected();
}

bool VideoOutputPort::isContentProtected() {
    EXPECT_NE(impl, nullptr);
    return impl->isContentProtected();
}

Display& VideoOutputPort::getDisplay() {
    EXPECT_NE(impl, nullptr);
    return impl->getDisplay();
}

VideoOutputPortConfigImpl* VideoOutputPortConfig::impl = nullptr;

VideoOutputPortConfig::VideoOutputPortConfig() {}

void VideoOutputPortConfig::setImpl(VideoOutputPortConfigImpl* newImpl) {
    // Handles both resetting 'impl' to nullptr and assigning a new value to 'impl'
    EXPECT_TRUE ((nullptr == impl) || (nullptr == newImpl));
    impl = newImpl;
}

VideoOutputPortConfig& VideoOutputPortConfig::getInstance() {
    static VideoOutputPortConfig instance;
    return instance;
}

VideoOutputPortType& VideoOutputPortConfig::getPortType(int id) {
    EXPECT_NE(impl, nullptr);
    return impl->getPortType(id);
}

VideoOutputPort& VideoOutputPortConfig::getPort(const std::string& name) {
    EXPECT_NE(impl, nullptr);
    return impl->getPort(name);
}

ManagerImpl* Manager::impl = nullptr;

Manager::Manager() {}

void Manager::setImpl(ManagerImpl* newImpl) {
        // Handles both resetting 'impl' to nullptr and assigning a new value to 'impl'
    EXPECT_TRUE ((nullptr == impl) || (nullptr == newImpl));
    impl = newImpl;
}

Manager& Manager::getInstance() {
    static Manager instance;
    return instance;
}

void Manager::Initialize() {
    EXPECT_NE(impl, nullptr);
    impl->Initialize();
}

void Manager::DeInitialize() {
    EXPECT_NE(impl, nullptr);
    impl->DeInitialize();
}

HostImpl* Host::impl = nullptr;


void Host::setImpl(HostImpl* newImpl) {
    // Handles both resetting 'impl' to nullptr and assigning a new value to 'impl'
    EXPECT_TRUE ((nullptr == impl) || (nullptr == newImpl));
    impl = newImpl;
}

Host& Host::getInstance() {
    static Host instance;
    return instance;
}

SleepMode Host::getPreferredSleepMode() {
    EXPECT_NE(impl, nullptr);
    return impl->getPreferredSleepMode();
}

int Host::setPreferredSleepMode(const SleepMode mode) {
    EXPECT_NE(impl, nullptr);
    return impl->setPreferredSleepMode(mode);
}

List<SleepMode> Host::getAvailableSleepModes() {
    EXPECT_NE(impl, nullptr);
    return impl->getAvailableSleepModes();
}

List<VideoOutputPort> Host::getVideoOutputPorts() {
    EXPECT_NE(impl, nullptr);
    return impl->getVideoOutputPorts();
}

List<AudioOutputPort> Host::getAudioOutputPorts() {
    EXPECT_NE(impl, nullptr);
    return impl->getAudioOutputPorts();
}

List<VideoDevice> Host::getVideoDevices() {
    EXPECT_NE(impl, nullptr);
    return impl->getVideoDevices();
}

VideoOutputPort& Host::getVideoOutputPort(const std::string& name) {
    EXPECT_NE(impl, nullptr);
    return impl->getVideoOutputPort(name);
}

AudioOutputPort& Host::getAudioOutputPort(const std::string& name) {
    EXPECT_NE(impl, nullptr);
    return impl->getAudioOutputPort(name);
}

void Host::getHostEDID(std::vector<uint8_t>& edid) const {
    EXPECT_NE(impl, nullptr);
    return impl->getHostEDID(edid);
}

std::string Host::getDefaultVideoPortName() {
    EXPECT_NE(impl, nullptr);
    return impl->getDefaultVideoPortName();
}

std::string Host::getDefaultAudioPortName() {
    EXPECT_NE(impl, nullptr);
    return impl->getDefaultAudioPortName();
}

FrontPanelIndicator::ColorImpl* FrontPanelIndicator::Color::impl = nullptr;

FrontPanelIndicator::Color::Color() {}

void FrontPanelIndicator::Color::setImpl(ColorImpl* newImpl) {
    // Handles both resetting 'impl' to nullptr and assigning a new value to 'impl'
    EXPECT_TRUE ((nullptr == impl) || (nullptr == newImpl));
    impl = newImpl;
}

FrontPanelIndicator::Color& FrontPanelIndicator::Color::getInstance() {
    static Color instance;
    return instance;
}

const FrontPanelIndicator::Color& FrontPanelIndicator::Color::getInstance(int id) {
    EXPECT_NE(impl, nullptr);
    return impl->getInstanceById(id);
}

const FrontPanelIndicator::Color& FrontPanelIndicator::Color::getInstance(const std::string& name) {
    EXPECT_NE(impl, nullptr);
    return impl->getInstanceByName(name);
}

const int FrontPanelIndicator::Color::kWhite = dsFPD_COLOR_WHITE;

std::string FrontPanelIndicator::Color::getName() const {
    EXPECT_NE(impl, nullptr);
    return impl->getName();
}

FrontPanelIndicator::Color::~Color() {}


FrontPanelTextDisplayImpl* FrontPanelTextDisplay::impl = nullptr;

FrontPanelTextDisplay::FrontPanelTextDisplay() {}

void FrontPanelTextDisplay::setImpl(FrontPanelTextDisplayImpl* newImpl) {
    // Handles both resetting 'impl' to nullptr and assigning a new value to 'impl'
    EXPECT_TRUE ((nullptr == impl) || (nullptr == newImpl));
    impl = newImpl;
}

FrontPanelTextDisplay& FrontPanelTextDisplay::getInstance(const std::string& name) {
    EXPECT_NE(impl, nullptr);
    return impl->getInstanceByName(name);
}

FrontPanelTextDisplay& FrontPanelTextDisplay::getInstance(int id) {
    EXPECT_NE(impl, nullptr);
    return impl->getInstanceById(id);
}

FrontPanelTextDisplay& FrontPanelTextDisplay::getInstance() {
    static FrontPanelTextDisplay instance;
    return instance;
}

const int FrontPanelTextDisplay::kModeClock12Hr = dsFPD_TIME_12_HOUR;
const int FrontPanelTextDisplay::kModeClock24Hr = dsFPD_TIME_24_HOUR;

int FrontPanelTextDisplay::getCurrentTimeFormat() const {
    EXPECT_NE(impl, nullptr);
    return impl->getCurrentTimeFormat();
}

void FrontPanelTextDisplay::setTimeFormat(const int iTimeFormat) {
    EXPECT_NE(impl, nullptr);
    impl->setTimeFormat(iTimeFormat);
}

void FrontPanelTextDisplay::setText(const std::string text) {
    EXPECT_NE(impl, nullptr);
    impl->setText(text);
}

void FrontPanelTextDisplay::setMode(int mode) {
    EXPECT_NE(impl, nullptr);
    impl->setMode(mode);
}

int FrontPanelTextDisplay::getTextBrightness() const {
    EXPECT_NE(impl, nullptr);
    return impl->getTextBrightness();
}

void FrontPanelTextDisplay::setTextBrightness(const int brightness) const {
    EXPECT_NE(impl, nullptr);
    impl->setTextBrightness(brightness);
}

FrontPanelConfigImpl* FrontPanelConfig::impl = nullptr;

FrontPanelConfig::FrontPanelConfig() {}

void FrontPanelConfig::setImpl(FrontPanelConfigImpl* newImpl) {
    // Handles both resetting 'impl' to nullptr and assigning a new value to 'impl'
    EXPECT_TRUE ((nullptr == impl) || (nullptr == newImpl));
    impl = newImpl;
}

FrontPanelConfig& FrontPanelConfig::getInstance() {
    static FrontPanelConfig instance;
    return instance;
}

List<FrontPanelIndicator> FrontPanelConfig::getIndicators() {
    EXPECT_NE(impl, nullptr);
    return impl->getIndicators();
}

FrontPanelTextDisplay& FrontPanelConfig::getTextDisplay(const std::string& name) {
    EXPECT_NE(impl, nullptr);
    return impl->getTextDisplay(name);
}

List<FrontPanelTextDisplay> FrontPanelConfig::getTextDisplays() {
    EXPECT_NE(impl, nullptr);
    return impl->getTextDisplays();
}

FrontPanelTextDisplay& FrontPanelConfig::getTextDisplay(int id) {
    EXPECT_NE(impl, nullptr);
    return impl->getTextDisplay(id);
}

FrontPanelTextDisplay& FrontPanelConfig::getTextDisplay() const {
    EXPECT_NE(impl, nullptr);
    return impl->getTextDisplay();
}
}

rtObjectBaseImpl* rtObjectBase::impl = nullptr;
rtObjectBase& rtObjectBase::getInstance() {
    static rtObjectBase instance;
    return instance;
}

void rtObjectBase::setImpl(rtObjectBaseImpl* newImpl) {
    // Handles both resetting 'impl' to nullptr and assigning a new value to 'impl'
    EXPECT_TRUE ((nullptr == impl) || (nullptr == newImpl));
    impl = newImpl;
}

rtError rtObjectBase::set(const char* name, const char* value) {
    EXPECT_NE(impl, nullptr);
    return impl->set(name, value);
}

rtError rtObjectBase::set(const char* name, bool value) {
    EXPECT_NE(impl, nullptr);
    return impl->set(name, value);
}

rtError rtObjectBase::set(const char* name, const rtValue& value) {
    EXPECT_NE(impl, nullptr);
    return impl->set(name, value);
}


rtObjectRefImpl* rtObjectRef::impl = nullptr;

rtObjectRef& rtObjectRef::getInstance() {
    static rtObjectRef instance;
    return instance;
}

void rtObjectRef::setImpl(rtObjectRefImpl* newImpl) {
    // Handles both resetting 'impl' to nullptr and assigning a new value to 'impl'
    EXPECT_TRUE ((nullptr == impl) || (nullptr == newImpl));
    impl = newImpl;
}
rtObjectRef::rtObjectRef() {}

rtObjectRef::rtObjectRef(const rtObjectRef&) = default;

rtObjectRef::rtObjectRef(const rtMapObject* o) {
    delete o;
    o = nullptr;
}

rtObjectRef& rtObjectRef::operator=(rtMapObject* o) {
    delete o;
    o = nullptr;
    return *this;
}

rtObjectRef& rtObjectRef::operator=(const rtObjectRef&) {
    return *this;
}

rtObjectRef& rtObjectRef::operator=(rtIObject* o) {
    asn(o);
    return *this;
}

rtObjectRef& rtObjectRef::operator=(rtObjectRef&&) = default;

rtError rtObjectRef::send(const char* messageName) {
    EXPECT_NE(impl, nullptr);
    return impl->send(messageName);
}

rtError rtObjectRef::send(const char* messageName, const char* method, rtFunctionCallback* callback) {
    EXPECT_NE(impl, nullptr);
    return impl->send(messageName, method, callback);
}

rtError rtObjectRef::send(const char* messageName, const rtValue& arg1) {
    EXPECT_NE(impl, nullptr);
    return impl->send(messageName, arg1);
}

rtError rtObjectRef::send(const char* messageName, rtObjectRef& base) {
    EXPECT_NE(impl, nullptr);
    return impl->send(messageName, base);
}

rtObjectRef::~rtObjectRef() {}

rtValueImpl* rtValue::impl = nullptr;

rtValue& rtValue::getInstance() {
    static rtValue instance;
    return instance;
}

rtValue::rtValue() = default;

rtValue::rtValue(bool v) : mValue({ .boolValue = v }) {}

rtValue::rtValue(const char* v) : mValue({ .stringValue = v }) {}

rtValue::rtValue(const rtString& v) : mValue({ .stringValue = v.cString() }) {}

rtValue::rtValue(rtIObject* v) {
    if (v) {
        delete v;
        v = nullptr;
    }
}

rtValue::rtValue(const rtObjectRef& v) {}

rtValue::rtValue(const rtValue& other) {
    mValue = other.mValue;
}

rtValue::~rtValue() {}

rtValue& rtValue::operator=(bool v) {
    mValue.boolValue = v;
    return *this;
}

rtValue& rtValue::operator=(const char* v) {
    mValue.stringValue = v;
    return *this;
}

rtValue& rtValue::operator=(const rtString& v) {
    mValue.stringValue = v.cString();
    return *this;
}

rtValue& rtValue::operator=(const rtIObject* v) {
    delete v;
    v = nullptr;
    return *this;
}

rtValue& rtValue::operator=(const rtObjectRef& v) {
    return *this;
}

rtValue& rtValue::operator=(const rtValue& other) {
    if (this != &other) {
        mValue = other.mValue;
    }
    return *this;
}

void rtValue::setImpl(rtValueImpl* newImpl) {
    // Handles both resetting 'impl' to nullptr and assigning a new value to 'impl'
    EXPECT_TRUE ((nullptr == impl) || (nullptr == newImpl));
    impl = newImpl;
}

rtObjectRef rtValue::toObject() const {
    rtObjectRef v;
    return v;
}

void rtValue::setString(const char* v) {
    mValue.stringValue = v;
}

void rtValue::setString(const rtString& v) {
    mValue.stringValue = v.cString();
}


rtArrayObjectImpl* rtArrayObject::impl = nullptr;

rtArrayObject& rtArrayObject::getInstance() {
    static rtArrayObject instance;
    return instance;
}

void rtArrayObject::pushBack(const char* v) {
    EXPECT_NE(impl, nullptr);
    impl->pushBack(v);
}

void rtArrayObject::pushBack(rtValue v) {
    EXPECT_NE(impl, nullptr);
    impl->pushBack(v);
}

void rtArrayObject::setImpl(rtArrayObjectImpl* newImpl) {
    // Handles both resetting 'impl' to nullptr and assigning a new value to 'impl'
    EXPECT_TRUE ((nullptr == impl) || (nullptr == newImpl));
    impl = newImpl;
}

floatingRtFunctionsImpl* floatingRtFunctions::impl = nullptr;
floatingRtFunctions& floatingRtFunctions::getInstance()
{
    static floatingRtFunctions instance;
    return instance;
}
void floatingRtFunctions::setImpl(floatingRtFunctionsImpl* newImpl) {
    // Handles both resetting 'impl' to nullptr and assigning a new value to 'impl'
    EXPECT_TRUE ((nullptr == impl) || (nullptr == newImpl));
    impl = newImpl;
}

rtError rtRemoteProcessSingleItem() {
    return floatingRtFunctions::getInstance().impl->rtRemoteProcessSingleItem();
}

rtError rtRemoteLocateObject(rtRemoteEnvironment* env, const char* str, rtObjectRef& obj, int x, remoteDisconnectCallback back, void* cbdata) {
    return floatingRtFunctions::getInstance().impl->rtRemoteLocateObject(env, str, obj, x, back, cbdata);
}

rtRemoteEnvironment* rtEnvironmentGetGlobal() {
    return floatingRtFunctions::getInstance().impl->rtEnvironmentGetGlobal();
}

rtError rtRemoteInit(rtRemoteEnvironment* env) {
    return floatingRtFunctions::getInstance().impl->rtRemoteInit(env);
}

rtError rtRemoteShutdown(rtRemoteEnvironment* env) {
    return floatingRtFunctions::getInstance().impl->rtRemoteShutdown(env);
}

char* rtStrError(rtError err) {
    return floatingRtFunctions::getInstance().impl->rtStrError(err);
}
