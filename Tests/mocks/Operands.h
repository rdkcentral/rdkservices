#ifndef HDMI_CCEC_OPERANDS_HPP_
#define HDMI_CCEC_OPERANDS_HPP_
#include <stdint.h>


#include <cstring>
#include <cstdio>
#include <vector>

#include <sstream>
#include <string>

typedef uint32_t Op_t;

enum {
    ACTIVE_SOURCE = 0x82,
    IMAGE_VIEW_ON = 0x04,
    TEXT_VIEW_ON = 0x0D,
    INACTIVE_SOURCE = 0x9D,
    REQUEST_ACTIVE_SOURCE = 0x85,
    ROUTING_CHANGE = 0x80,
    ROUTING_INFORMATION = 0x81,
    SET_STREAM_PATH = 0x86,
    STANDBY = 0x36,
    RECORD_OFF = 0X0B,
    RECORD_ON = 0X09,
    RECORD_STATUS = 0X0A,
    RECORD_TV_SCREEN = 0X0F,
    CLEAR_ANALOGUE_TIMER = 0X33,
    CLEAR_DIGITAL_TIMER = 0X99,
    CLEAR_EXTERNAL_TIMER = 0XA1,
    SET_ANALOG_TIMER = 0X34,
    SET_DIGITAL_TIMER = 0X97,
    SET_EXTERNAL_TIMER = 0XA2,
    SET_TIMER_PROGRAM_TITLE = 0X67,
    TIMER_CLEARED_STATUS = 0X43,
    TIMER_STATUS = 0X35,
    GET_CEC_VERSION = 0x9F,
    CEC_VERSION = 0x9E,
    GIVE_PHYSICAL_ADDRESS = 0x83,
    GET_MENU_LANGUAGE = 0X91,
    REPORT_PHYSICAL_ADDRESS = 0x84,
    SET_MENU_LANGUAGE = 0X32,
    DECK_CONTROL = 0X42,
    DECK_STATUS = 0X1B,
    GIVE_DECK_STATUS = 0X1A,
    PLAY = 0X41,
    GIVE_TUNER_DEVICE_STATUS = 0X08,
    SELECT_ANALOGUE_SERVICE = 0X92,
    SELECT_DIGITAL_SERVICE = 0X93,
    TUNER_DEVICE_STATUS = 0X07,
    TUNER_STEP_DECREMENT = 0X06,
    TUNER_STEP_INCREMENT = 0X05,
    DEVICE_VENDOR_ID = 0x87,
    GIVE_DEVICE_VENDOR_ID = 0x8C,
    VENDOR_COMMAND = 0X89,
    VENDOR_COMMAND_WITH_ID = 0XA0,
    VENDOR_REMOTE_BUTTON_DOWN = 0X8A,
    VENDOR_REMOTE_BUTTON_UP = 0X8B,
    SET_OSD_STRING = 0x64,
    GIVE_OSD_NAME = 0x46,
    SET_OSD_NAME = 0x47,
    MENU_REQUEST = 0X8D,
    MENU_STATUS = 0X8E,
    USER_CONTROL_PRESSED = 0X44,
    USER_CONTROL_RELEASED = 0X45,
    GIVE_DEVICE_POWER_STATUS = 0x8F,
    REPORT_POWER_STATUS = 0x90,
    FEATURE_ABORT = 0x00,
    ABORT = 0xFF,
    GIVE_AUDIO_STATUS = 0X71,
    GIVE_SYSTEM_AUDIO_MODE_STATUS = 0X7D,
    REPORT_AUDIO_STATUS = 0X7A,
    REPORT_SHORT_AUDIO_DESCRIPTOR = 0XA3,
    REQUEST_SHORT_AUDIO_DESCRIPTOR = 0XA4,
    SET_SYSTEM_AUDIO_MODE = 0X72,
    SYSTEM_AUDIO_MODE_REQUEST = 0X70,
    SYSTEM_AUDIO_MODE_STATUS = 0X7E,
    SET_AUDIO_RATE = 0X9A,
    INITIATE_ARC = 0XC0,
    REPORT_ARC_INITIATED = 0XC1,
    REPORT_ARC_TERMINATED = 0XC2,
    REQUEST_ARC_INITIATION = 0XC3,
    REQUEST_ARC_TERMINATION = 0XC4,
    TERMINATE_ARC = 0XC5,
    CDC_MESSAGE = 0XF8,
    POLLING = 0x200, // Special Code for Polling Msg.
    UNKNOWN = 0xFFFF
};

typedef struct _dsHdmiInGetNumberOfInputsParam_t {
    dsError_t result;
    uint8_t numHdmiInputs;
} dsHdmiInGetNumberOfInputsParam_t;

typedef struct _dsHdmiInStatus_t {
    bool isPresented; /**< Boolean flag indicating Hdmi Input is enabled for presentation by client */
    bool isPortConnected[dsHDMI_IN_PORT_MAX]; /**< Boolean flag indicating Hdmi source connected to this Hdmi Input port */
    dsHdmiInPort_t activePort; /**< Hdmi Input Port selected as the currently active port (to the set-top)
                                 - note that only one HDMI Input port can be active at a time */
} dsHdmiInStatus_t;

typedef struct _dsHdmiInGetStatusParam_t {
    dsError_t result;
    dsHdmiInStatus_t status;
} dsHdmiInGetStatusParam_t;

typedef struct _dsGetHDMIARCPortIdParam_t {
    dsError_t result;
    int portId;
} dsGetHDMIARCPortIdParam_t;

typedef struct _Throw_e {
} Throw_e;

class Exception : public std::exception {
public:
    virtual const char* what() const throw()
    {
        return "Base Exception..";
    }
};

class CECNoAckException : public Exception {
public:
    virtual const char* what() const throw()
    {
        return "Ack not received..";
    }
};

class CECFrameImpl {
public:
    virtual void getBuffer(const uint8_t** buf, size_t* len) const = 0;
};

class CECFrame {
public:
    enum {
        MAX_LENGTH = 128,
    };

    CECFrame(const uint8_t* buf = NULL, uint16_t len = 0) {}

    static CECFrame& getInstance()
    {
        static CECFrame instance;
        return instance;
    }

    CECFrameImpl* impl;
    void getBuffer(const uint8_t** buf, size_t* len) const
    {
        return impl->getBuffer(buf, len);
    }

private:
    uint8_t buf_[MAX_LENGTH];
    size_t len_;
};

class CECBytes {
protected:
    std::vector<uint8_t> str;
    CECBytes(const uint8_t val) {}
    CECBytes(const uint8_t* buf, size_t len) {}
    CECBytes(const CECFrame& frame, size_t startPos, size_t len) {}

public:
    CECFrame& serialize(CECFrame& frame) const
    {
        return frame;
    }

    bool operator==(const CECBytes& in) const
    {
        return this->str == in.str;
    };

    virtual const std::string name(void) const
    {
        return "Operand";
    }

    virtual const std::string toString(void) const
    {
        std::stringstream stream;
        return stream.str();
    };

    ~CECBytes(void) {}
};

class OSDString : public CECBytes {
};

class OSDName : public CECBytes {
public:
    OSDName(const char* str1)
        : CECBytes((const uint8_t*)str1, strlen(str1))
    {
    }
    const std::string toString(void) const {
        return std::string(str.begin(), str.end());
    }
};

class AbortReasonImpl {
public:
    virtual int toInt(void) const = 0;
};

class AbortReason : public CECBytes {
public:
    enum {
        UNRECOGNIZED_OPCODE,
        NOT_IN_CORRECT_MODE_TO_RESPOND,
        CANNOT_OVERIDE_SOURCE,
        INVALID_OPERAND,
        REFUSED,
        UNABLE_TO_DETERMINE,
    };
    AbortReason(int reason)
        : CECBytes((uint8_t)reason)
    {
    }

    AbortReason();

    AbortReason* impl;
    static AbortReason& getInstance()
    {
        static AbortReason instance;
        return instance;
    }

    int toInt(void) const
    {
        return impl->toInt();
    }
};

class DeviceType : public CECBytes {
public:
    enum {
        MAX_LEN = 1,
    };

    enum {
        TV = 0x0,
        RECORDING_DEVICE,
        RESERVED,
        TUNER,
        PLAYBACK_DEVICE,
        AUDIO_SYSTEM,
        PURE_CEC_SWITCH,
        VIDEO_PROCESSOR,
    };

    DeviceType(const CECFrame& frame, size_t startPos)
        : CECBytes(frame, startPos, MAX_LEN)
    {
    }
    DeviceType(int type)
        : CECBytes((uint8_t)type)
    {
    }
};

class Language : public CECBytes {
public:
    enum {
        MAX_LEN = 3,
    };

    Language(const char* str1)
        : CECBytes((const uint8_t*)str1, MAX_LEN){};
};

class VendorIDImpl {
public:
    virtual const std::string toString(void) const = 0;
};

class VendorID : public CECBytes{

public:
    VendorID(uint8_t byte0, uint8_t byte1, uint8_t byte2): CECBytes (NULL,0){};
    VendorID(const uint8_t* buf, size_t len): CECBytes (NULL,0){};
    VendorID(): CECBytes (NULL,0){};

    VendorIDImpl* impl;
    static VendorID& getInstance()
    {
        static VendorID instance;
        return instance;
    }

   const std::string toString(void) const
    {
        return getInstance().impl->toString();
    }
};

class PhysicalAddressImpl {
public:
    virtual uint8_t getByteValue(int index) const = 0;
};

class PhysicalAddress : public CECBytes {
public:
    enum {
        MAX_LEN = 2,
    };
    PhysicalAddress(const CECFrame& frame, size_t startPos)
        : CECBytes(frame, startPos, MAX_LEN){};
    PhysicalAddress(uint8_t byte0, uint8_t byte1, uint8_t byte2, uint8_t byte3)
        : CECBytes(NULL, 0){};
    PhysicalAddress(std::string& addr)
        : CECBytes(NULL, 0)
    {
    }
    PhysicalAddress();

    PhysicalAddress* impl;
    static PhysicalAddress& getInstance()
    {
        static PhysicalAddress instance;
        return instance;
    }

    uint8_t getByteValue(int index) const
    {
        return impl->getByteValue(index);
    }
};

class LogicalAddressImpl {
public:
    virtual int toInt() const = 0;
    virtual int getType() const = 0;
};

class LogicalAddress : public CECBytes {
public:
    enum {
        TV = 0,
        RECORDING_DEVICE_1 = 1,
        RECORDING_DEVICE_2 = 2,
        TUNER_1 = 3,
        PLAYBACK_DEVICE_1 = 4,
        AUDIO_SYSTEM = 5,
        TUNER_2 = 6,
        TUNER_3 = 7,
        PLAYBACK_DEVICE_2 = 8,
        RECORDING_DEVICE_3 = 9,
        TUNER_4 = 10,
        PLAYBACK_DEVICE_3 = 11,
        RESERVED_12 = 12,
        RESERVED_13 = 13,
        SPECIFIC_USE = 14,
        UNREGISTERED = 15,
        BROADCAST = UNREGISTERED,
    };

    LogicalAddress* impl;
    static LogicalAddress& getInstance()
    {
        static LogicalAddress instance;
        return instance;
    }

    LogicalAddress(int addr = UNREGISTERED)
        : CECBytes((uint8_t)addr){};
    
	int toInt() const
    {
        return impl->toInt();
    }

    int getType() const;
};

class Version : public CECBytes {
public:
    enum {
        V_RESERVED_0,
        V_RESERVED_1,
        V_RESERVED_2,
        V_RESERVED_3,
        V_1_3a,
        V_1_4,
    };

    Version(int version)
        : CECBytes((uint8_t)version){};
};

class PowerStatusImpl {
public:
    virtual int toInt(void) const = 0;
};

class PowerStatus : public CECBytes {
public:
    enum {
        ON = 0,
        STANDBY = 0x01,
        IN_TRANSITION_STANDBY_TO_ON = 0x02,
        IN_TRANSITION_ON_TO_STANDBY = 0x03,
        POWER_STATUS_NOT_KNOWN = 0x4,
        POWER_STATUS_FEATURE_ABORT = 0x05,
    };

    PowerStatus(int status)
        : CECBytes((uint8_t)status){};
    PowerStatus();

    PowerStatus* impl;

    static PowerStatus& getInstance()
    {
        static PowerStatus instance;
        return instance;
    }

    int toInt(void) const
    {
        return impl->toInt();
    }
};

class RequestAudioFormat : public CECBytes {
};

class ShortAudioDescriptorImpl {
public:
    virtual uint32_t getAudiodescriptor(void) const = 0;
};

class ShortAudioDescriptor : public CECBytes {
public:
    ShortAudioDescriptor();

    ShortAudioDescriptor* impl;
    static ShortAudioDescriptor& getInstance() {
        static ShortAudioDescriptor instance;
        return instance;
    }

    uint32_t getAudiodescriptor(void) const {
        return impl->getAudiodescriptor();
    }
};

class SystemAudioStatusImpl {
public:
    virtual int toInt(void) const = 0;
};

class SystemAudioStatus : public CECBytes {
public:
    SystemAudioStatus* impl;

    SystemAudioStatus();

    static SystemAudioStatus& getInstance() {
        static SystemAudioStatus instance;
        return instance;
    }

    int toInt(void) const {
        return impl->toInt();
    }
};

class AudioStatusImpl {
public:
    virtual int getAudioMuteStatus(void) const = 0;
    virtual int getAudioVolume(void) const = 0;
};

class AudioStatus : public CECBytes {
public:
    AudioStatus* impl;

    AudioStatus();

    static AudioStatus& getInstance() {
        static AudioStatus instance;
        return instance;
    }

    int getAudioMuteStatus(void) const {
        return impl->getAudioMuteStatus();
    }

    int getAudioVolume(void) const {
        return impl->getAudioVolume();
    }
};

class UICommand : public CECBytes {
public:
    enum {
        UI_COMMAND_VOLUME_UP = 0x41,
        UI_COMMAND_VOLUME_DOWN = 0x42,
        UI_COMMAND_MUTE = 0x43,
        UI_COMMAND_MUTE_FUNCTION = 0x65,
        UI_COMMAND_RESTORE_FUNCTION = 0x66,
        UI_COMMAND_POWER_OFF_FUNCTION = 0x6C,
        UI_COMMAND_POWER_ON_FUNCTION = 0x6D,
        UI_COMMAND_UP = 0x01,
        UI_COMMAND_DOWN = 0x02,
        UI_COMMAND_LEFT = 0x03,
        UI_COMMAND_RIGHT = 0x04,
        UI_COMMAND_SELECT = 0x00,
        UI_COMMAND_HOME = 0x09,
        UI_COMMAND_BACK = 0x0D,
        UI_COMMAND_NUM_0 = 0x20,
        UI_COMMAND_NUM_1 = 0x21,
        UI_COMMAND_NUM_2 = 0x22,
        UI_COMMAND_NUM_3 = 0x23,
        UI_COMMAND_NUM_4 = 0x24,
        UI_COMMAND_NUM_5 = 0x25,
        UI_COMMAND_NUM_6 = 0x26,
        UI_COMMAND_NUM_7 = 0x27,
        UI_COMMAND_NUM_8 = 0x28,
        UI_COMMAND_NUM_9 = 0x29,
    };

    UICommand(int command)
        : CECBytes((uint8_t)command){};
};

class DataBlock {
};

inline const char* GetOpName(Op_t op) {
    return "name";
}

class MessageProcessor {
};

class ActiveSourceImpl {
public:
    virtual Op_t opCode() const = 0;
};

class ActiveSource : public DataBlock {
public:
    Op_t opCode(void) const { return ACTIVE_SOURCE; }

    ActiveSource(PhysicalAddress& phyAddress)
        : physicalAddress(phyAddress){
    }

    PhysicalAddress physicalAddress;
};

class InActiveSource {
public:
    Op_t opCode(void) const{
        return 1;
    }

    PhysicalAddress physicalAddress;
};

class Header {
public:
    LogicalAddress from;
    LogicalAddress to;
};

class ImageViewOn {
};

class TextViewOn {
};

class RequestActiveSource : public DataBlock {
};

class Standby : public DataBlock {
};

class CECVersion : public DataBlock {
public:
    CECVersion(const Version& ver)
        : version(ver){
    }

    Version version;
};

class GetCECVersion : public DataBlock {
};

class GetMenuLanguage {
};

class SetMenuLanguage : public DataBlock {
public:
    SetMenuLanguage(const Language& lan)
        : language(lan){};
    const Language language;
};

class GiveOSDName : public DataBlock {
};

class GivePhysicalAddress : public DataBlock {
};

class GiveDeviceVendorID : public DataBlock {
};

class SetOSDString {
public:
    OSDString osdString;
};

class RoutingChange : public DataBlock {
public:
    RoutingChange(PhysicalAddress& from1, const PhysicalAddress& to1)
        : from(from1)
        , to(to1)
    {
    }
    PhysicalAddress from;
    PhysicalAddress to;
};

class RoutingInformation {
public:
    PhysicalAddress toSink;
};

class SetStreamPath : public DataBlock {
public:
    SetStreamPath(const PhysicalAddress& toSink1)
        : toSink(toSink1){
    }

    SetStreamPath(const CECFrame& frame, int startPos = 0)
        : toSink(frame, startPos){
    }

    PhysicalAddress toSink;
};

class GiveDevicePowerStatus : public DataBlock {
};

class ReportPhysicalAddress : public DataBlock {
public:
    ReportPhysicalAddress(PhysicalAddress& physAddress, const DeviceType& devType)
        : physicalAddress(physAddress)
        , deviceType(devType){
    }

    ReportPhysicalAddress(const CECFrame& frame, int startPos = 0)
        : physicalAddress(frame, startPos)
        , deviceType(frame, startPos + PhysicalAddress::MAX_LEN){
    }

    PhysicalAddress physicalAddress;
    DeviceType deviceType;
};

class DeviceVendorID : public DataBlock {
public:
    DeviceVendorID(const VendorID& vendor)
        : vendorId(vendor) {
    }

    VendorID vendorId;
};

class ReportPowerStatus : public DataBlock {
public:
    ReportPowerStatus(PowerStatus stat)
        : status(stat) {
    }

    PowerStatus status;
};

class OpCode {
public:
    std::string toString(void) const { return GetOpName(opCode_); }
    Op_t opCode(void) const { return FEATURE_ABORT; }
    OpCode(Op_t opCode)
        : opCode_(opCode){};

private:
    Op_t opCode_;
};

class FeatureAbort : public DataBlock {
public:
    FeatureAbort(const OpCode& abfeature, const AbortReason& abreason)
        : feature(abfeature)
        , reason(abreason){
    }

    OpCode feature;
    AbortReason reason;
};

class Abort {
public:
    Op_t opCode(void) const { return ABORT; }
};

class UserControlReleased : public DataBlock {
};

class Polling {
};

class RequestShortAudioDescriptor : public DataBlock {
public:
    RequestShortAudioDescriptor(const std::vector<uint8_t> formatid, const std::vector<uint8_t> audioFormatCode, uint8_t number_of_descriptor = 1) {

    }
};

class UserControlPressed : public DataBlock {
public:
    UserControlPressed(const UICommand& command)
        : uiCommand(command){
    }

    UICommand uiCommand;
};

class SetOSDName;

class ReportAudioStatus {
public:
    Op_t opCode(void) const { return REPORT_AUDIO_STATUS; }
    AudioStatus status;
};

class SetSystemAudioMode {
public:
    Op_t opCode(void) const { return SET_SYSTEM_AUDIO_MODE; }
    SystemAudioStatus status;
};

class ReportShortAudioDescriptor {
public:
    Op_t opCode(void) const { return REPORT_SHORT_AUDIO_DESCRIPTOR; }
    std::vector<ShortAudioDescriptor> shortAudioDescriptor;
    uint8_t numberofdescriptor;
};

class InitiateArc {
};

class TerminateArc {
};

class MessageDecoderImpl {
public:
    virtual void decode(const CECFrame& in) const = 0;
};

class MessageDecoder {
private:
    MessageProcessor& processor;

public:
    MessageDecoderImpl* impl;
    MessageDecoder(MessageProcessor& proc)
        : processor(proc){};

    void decode(const CECFrame& in) {
        return impl->decode(in);
    }
};

class SetOSDName : public DataBlock {
public:
    SetOSDName(OSDName& OsdName)
        : osdName(OsdName){};

    OSDName osdName;
};

class FrameListener {
public:
    virtual void notify(const CECFrame&) const = 0;
    virtual ~FrameListener(void) {}
};

class ConnectionImpl {
public:
    virtual ~ConnectionImpl() = default;
    virtual void open() const = 0;
    virtual void close() const = 0;
    virtual void addFrameListener(FrameListener* listener) const = 0;
    virtual void ping(const LogicalAddress& from, const LogicalAddress& to, const Throw_e& doThrow) const = 0;
    virtual void sendToAsync(const LogicalAddress& to, const CECFrame& frame) const = 0;
    virtual void sendTo(const LogicalAddress& to, const CECFrame& frame) const = 0;
    virtual void sendTo(const LogicalAddress& to, const CECFrame& frame, int timeout) const = 0;
    virtual void poll(const LogicalAddress& from, const Throw_e& doThrow) const = 0;
};

class Connection {
public:
    ConnectionImpl* impl;

    Connection(const LogicalAddress& source = LogicalAddress::UNREGISTERED, bool opened = true, const std::string& name = "") {}

    static Connection& getInstance() {
        static Connection instance;
        return instance;
    };

    void open(void) {
        return getInstance().impl->open();
    }

    void close(void) {
        return getInstance().impl->close();
    }

    void addFrameListener(FrameListener* listener) {
        return getInstance().impl->addFrameListener(listener);
    }

    void ping(const LogicalAddress& from, const LogicalAddress& to, const Throw_e& doThrow) {
        return getInstance().impl->ping(from, to, doThrow);
    }

    void sendToAsync(const LogicalAddress& to, const CECFrame& frame) {
        return getInstance().impl->sendToAsync(to, frame);
    }

    void sendTo(const LogicalAddress& to, const CECFrame& frame) {
        return getInstance().impl->sendTo(to, frame);
    }

    void sendTo(const LogicalAddress& to, const CECFrame& frame, int timeout) {
        return getInstance().impl->sendTo(to, frame, timeout);
    }

    void poll(const LogicalAddress& from, const Throw_e& doThrow) {
        return getInstance().impl->poll(from, doThrow);
    }

    void setSource(LogicalAddress& from) {
    }
};

class SystemAudioModeRequest : public DataBlock {
public:
    SystemAudioModeRequest(const PhysicalAddress& physicaladdress = { 0xf, 0xf, 0xf, 0xf })
        : _physicaladdress(physicaladdress){
    }

    PhysicalAddress _physicaladdress;
};

class GiveAudioStatus : public DataBlock {
public:
    Op_t opCode(void) const { return GIVE_AUDIO_STATUS; }
};

class LibCCECImpl {
public:
    virtual void init(const char* name) const = 0;
    virtual void init() const = 0;
    virtual void term() const = 0;
    virtual void getPhysicalAddress(uint32_t* physicalAddress) const = 0;
    virtual int addLogicalAddress(const LogicalAddress& source) const = 0;
};

class LibCCEC {
public:
    LibCCEC(void){};

    static LibCCEC& getInstance(){
        static LibCCEC instance;
        return instance;
    };

    LibCCECImpl* impl;
    void init(const char* name){
        return impl->init(name);
    }

    void init(){
        return impl->init();
    }

    void term(){
        return;
    }

    void getPhysicalAddress(uint32_t* physicalAddress){
        return impl->getPhysicalAddress(physicalAddress);
    }

    int addLogicalAddress(const LogicalAddress& source){
        return impl->addLogicalAddress(source);
    }
};

class RequestArcInitiation : public DataBlock {
public:
    Op_t opCode(void) const { return REQUEST_ARC_INITIATION; }
};

class ReportArcInitiation : public DataBlock {
public:
    Op_t opCode(void) const { return REPORT_ARC_INITIATED; }
};

class RequestArcTermination : public DataBlock {
public:
    Op_t opCode(void) const { return REQUEST_ARC_TERMINATION; }
};

class ReportArcTermination : public DataBlock {
public:
    Op_t opCode(void) const { return REPORT_ARC_TERMINATED; }
};

class MessageEncoderImpl {
public:
    virtual CECFrame& encode(const DataBlock& m) const = 0;
};

class MessageEncoder {
public:
    MessageEncoderImpl* impl;
    MessageEncoder(){};
    static MessageEncoder& getInstance()
    {
        static MessageEncoder instance;
        return instance;
    }
    CECFrame& encode(const DataBlock m)
    {
        return getInstance().impl->encode(m);
    }
};

#endif
