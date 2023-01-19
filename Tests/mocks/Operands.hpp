/*
 * If not stated otherwise in this file or this component's Licenses.txt file the
 * following copyright and licenses apply:
 *
 * Copyright 2016 RDK Management
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
*/



/**
* @defgroup hdmicec
* @{
* @defgroup ccec
* @{
**/

#ifndef HDMI_CCEC_OPERANDS_HPP_
#define HDMI_CCEC_OPERANDS_HPP_
#include <stdint.h>

#include <cstring>
#include <cstdio>
#include <vector>

#include <sstream>

#include "CCEC.hpp"
#include "Assert.hpp"
#include "Operand.hpp"
#include "Util.hpp"
#include "CECFrame.hpp"
#include "Exception.hpp"

CCEC_BEGIN_NAMESPACE
typedef uint32_t Op_t;
enum
{
	ACTIVE_SOURCE 					= 0x82,
	IMAGE_VIEW_ON 					= 0x04,
	TEXT_VIEW_ON 					= 0x0D,
	INACTIVE_SOURCE 				= 0x9D,
	REQUEST_ACTIVE_SOURCE 			= 0x85,
	ROUTING_CHANGE                  = 0x80,
	ROUTING_INFORMATION             = 0x81,
	SET_STREAM_PATH                 = 0x86,
	STANDBY 						= 0x36,
	RECORD_OFF                      = 0X0B,
	RECORD_ON                       = 0X09,
	RECORD_STATUS                   = 0X0A,
	RECORD_TV_SCREEN                = 0X0F,
	CLEAR_ANALOGUE_TIMER            = 0X33,
	CLEAR_DIGITAL_TIMER             = 0X99,
	CLEAR_EXTERNAL_TIMER            = 0XA1,
	SET_ANALOG_TIMER                = 0X34,
	SET_DIGITAL_TIMER               = 0X97,
	SET_EXTERNAL_TIMER              = 0XA2,
	SET_TIMER_PROGRAM_TITLE         = 0X67,
	TIMER_CLEARED_STATUS            = 0X43,
	TIMER_STATUS                    = 0X35,
	GET_CEC_VERSION 				= 0x9F,
	CEC_VERSION 					= 0x9E,
	GIVE_PHYSICAL_ADDRESS 			= 0x83,
	GET_MENU_LANGUAGE               = 0X91,
	REPORT_PHYSICAL_ADDRESS 		= 0x84,
	SET_MENU_LANGUAGE               = 0X32,
	DECK_CONTROL                    = 0X42,
	DECK_STATUS                     = 0X1B,
	GIVE_DECK_STATUS                = 0X1A,
	PLAY                            = 0X41,
	GIVE_TUNER_DEVICE_STATUS        = 0X08,
	SELECT_ANALOGUE_SERVICE         = 0X92,
	SELECT_DIGITAL_SERVICE          = 0X93,
	TUNER_DEVICE_STATUS             = 0X07,
	TUNER_STEP_DECREMENT            = 0X06,
	TUNER_STEP_INCREMENT            = 0X05,
	DEVICE_VENDOR_ID 				= 0x87,
	GIVE_DEVICE_VENDOR_ID 			= 0x8C,
	VENDOR_COMMAND                  = 0X89,
	VENDOR_COMMAND_WITH_ID          = 0XA0,
	VENDOR_REMOTE_BUTTON_DOWN       = 0X8A,
	VENDOR_REMOTE_BUTTON_UP         = 0X8B,
	SET_OSD_STRING 					= 0x64,
	GIVE_OSD_NAME 					= 0x46,
	SET_OSD_NAME 					= 0x47,
	MENU_REQUEST                    = 0X8D,
	MENU_STATUS                     = 0X8E,
	USER_CONTROL_PRESSED            = 0X44,
	USER_CONTROL_RELEASED           = 0X45,
	GIVE_DEVICE_POWER_STATUS 		= 0x8F,
	REPORT_POWER_STATUS 			= 0x90,
	FEATURE_ABORT 					= 0x00,
	ABORT 							= 0xFF,
	GIVE_AUDIO_STATUS               = 0X71,
	GIVE_SYSTEM_AUDIO_MODE_STATUS   = 0X7D,
	REPORT_AUDIO_STATUS             = 0X7A,
	REPORT_SHORT_AUDIO_DESCRIPTOR   = 0XA3,
	REQUEST_SHORT_AUDIO_DESCRIPTOR  = 0XA4,
	SET_SYSTEM_AUDIO_MODE           = 0X72,
	SYSTEM_AUDIO_MODE_REQUEST       = 0X70,
	SYSTEM_AUDIO_MODE_STATUS        = 0X7E,
	SET_AUDIO_RATE                  = 0X9A,
	INITIATE_ARC                    = 0XC0,
	REPORT_ARC_INITIATED            = 0XC1,
	REPORT_ARC_TERMINATED           = 0XC2,
	REQUEST_ARC_INITIATION          = 0XC3,
	REQUEST_ARC_TERMINATION         = 0XC4,
	TERMINATE_ARC                   = 0XC5,
	CDC_MESSAGE                     = 0XF8,
	POLLING 						= 0x200, // Special Code for Polling Msg.

	UNKNOWN                         = 0xFFFF
};

class CECBytes : public Operand
{
protected:
    CECBytes(const uint8_t val) : str1(1, val) {}
	CECBytes(const uint8_t *buf, size_t len) {
        if (buf && len) {
            for (size_t i = 0; i < len; i++) {
                str1.push_back(buf[i]);
            }
        }
	}

	bool validate(void) const {
		return (str1.size() && (str1.size() <= getMaxLen()));
	}

	CECBytes(const CECFrame &frame, size_t startPos, size_t len) {
    	/*
    	 * For HDMI CEC definition, the [OSD Name] and [OSD STring] are always the one
    	 * and only one operands in the message. It is not clear if these strings are
    	 * null terminated.  Therefore, consume all remaining bytes in the frame as
    	 * CECBytes
    	 */
        const uint8_t *buf = 0;
        size_t frameLen = 0;
        frame.getBuffer(&buf, &frameLen);
        str1.clear();
        len = ((startPos + len) > frameLen) ? frameLen - startPos : len;
        str1.insert(str1.begin(), buf + startPos, buf + startPos + len);
        if (!validate())
        {
            //throw InvalidParamException();
        }
	}

public:
	CECFrame &serialize(CECFrame &frame) const {
		for (size_t i = 0; i < str1.size(); i++) {
			frame.append(str1[i]);
		}

		return frame;
	}
    using Operand::serialize;

	~CECBytes(void) {}

	virtual const std::string toString(void) const {
		std::stringstream stream;
		for (size_t i = 0; i < str1.size(); i++) {
			stream << std::hex << (int)str1[i];
		}
		return stream.str();
    };

	bool operator == (const CECBytes &in) const {
		return this->str1 == in.str1;
	}

protected:
    std::vector<uint8_t> str1;
    virtual size_t getMaxLen(void) const {
        return CECFrame::MAX_LENGTH;
    }
};

class OSDString : public CECBytes 
{
public:
	enum {
		MAX_LEN = 13,
	};

    OSDString(const char *str) : CECBytes((const uint8_t *)str, strlen(str)) {
        validate();
    }

    OSDString(const CECFrame &frame, size_t startPos) : CECBytes(frame, startPos, MAX_LEN) {
    }

	const std::string toString(void) const {
		return std::string(str1.begin(), str1.end());
	}
protected:
	size_t getMaxLen() const {return MAX_LEN;}
};

class OSDName : public CECBytes 
{
public:
	enum {
		MAX_LEN = 14,
	};

    OSDName(const char *str) : CECBytes((const uint8_t *)str, strlen(str)) {
        validate();
    }

    OSDName(const CECFrame &frame, size_t startPos) : CECBytes(frame, startPos, MAX_LEN) {
    }

	const std::string toString(void) const {
		return std::string(str1.begin(), str1.end());
	}
protected:
	size_t getMaxLen() const {return MAX_LEN;}
};

class AbortReason : public CECBytes 
{
public :
    enum {
        MAX_LEN = 1,
    };

	enum  {
		UNRECOGNIZED_OPCODE,
		NOT_IN_CORRECT_MODE_TO_RESPOND,
		CANNOT_OVERIDE_SOURCE,
		INVALID_OPERAND,
		REFUSED,
		UNABLE_TO_DETERMINE,
	};

	AbortReason(int reason) : CECBytes((uint8_t)reason) { }

	const std::string toString(void) const {
		static const char *names_[] = {
			"Unrecognized opcode",
			"Not in correct mode to respond",
			"Cannot provide source",
			"Invalid operand",
			"Refused",
			"Unable to determine",
		};

		if (validate())
		{
			return names_[str1[0]];
		}
		else
		{
			CCEC_LOG(LOG_WARN,"Unknown abort reason:%x\n", str1[0]);
			return "Unknown";
		}
	}

	bool validate(void) const {
		return (/*(str[0]>= UNRECOGNIZED_OPCODE) && */(str1[0]<= UNABLE_TO_DETERMINE));
	}

	int toInt(void) const {
        return str1[0];
    }

	AbortReason(const CECFrame &frame, size_t startPos) : CECBytes(frame, startPos, MAX_LEN) { } 


protected:
	size_t getMaxLen() const {return MAX_LEN;}

};

class DeviceType : public CECBytes 
{
public :
    enum {
        MAX_LEN = 1,
    };

	enum  {
		TV = 0x0,
		RECORDING_DEVICE,
		RESERVED,
		TUNER,
		PLAYBACK_DEVICE,
		AUDIO_SYSTEM,
		PURE_CEC_SWITCH,
		VIDEO_PROCESSOR,
	};

	DeviceType(int type) : CECBytes((uint8_t)type) {}

	const std::string toString(void) const {
		static const char *names_[] = {
				"TV",
				"Recording Device",
				"Reserved",
				"Tuner",
				"Playback Device",
				"Audio System",
				"Pure CEC Switch",
				"Video Processor",
		};

		if (validate())
		{
			return names_[str1[0]];
		}
		else
		{
			CCEC_LOG(LOG_WARN,"Unknown device type:%x\n", str1[0]);
			return "Unknown";
		}
	}

	bool validate(void) const {
		return (/*(str[0] >= TV) && */(str1[0] <= VIDEO_PROCESSOR));
	}

	DeviceType(const CECFrame &frame, size_t startPos) : CECBytes(frame, startPos, MAX_LEN) {}

	~DeviceType(void) {}

protected:
	size_t getMaxLen() const {return MAX_LEN;}
};

class Language : public CECBytes 
{
public:
	enum {
		MAX_LEN = 3,
	};

    Language(const char *str) : CECBytes((const uint8_t*)str, MAX_LEN) {
        validate();
    }

    Language(const CECFrame &frame, size_t startPos) : CECBytes(frame, startPos, MAX_LEN) {
    }

	const std::string toString(void) const {
		return std::string(str1.begin(), str1.end());
	}
protected:
	size_t getMaxLen() const {return MAX_LEN;}
};

class VendorID
{

public:
    enum {
        MAX_LEN = 3,
    };

    VendorID(uint8_t byte0, uint8_t byte1, uint8_t byte2) {
        /*uint8_t bytes[MAX_LEN];
        bytes[0] = byte0;
        bytes[1] = byte1;
        bytes[2] = byte2;
        str1.insert(str1.begin(), bytes, bytes + MAX_LEN);*/
    }

    VendorID(const uint8_t *buf, size_t len){
    }

    const std::string toString(void) const {
                        //return impl->toString();
			return "1";
    }
    //VendorID(const uint8_t *buf, size_t len) : CECBytes (buf, (len > MAX_LEN) ? MAX_LEN : len) {
    //}

    /*VendorID(const CECFrame &frame, size_t startPos) : CECBytes (frame, startPos, MAX_LEN) {
    };*/

protected:
    size_t getMaxLen() const {return MAX_LEN;}

};

/*
class VendorID : public CECBytes
{
public:
	enum {
		MAX_LEN = 3,
	};

	VendorID(uint8_t byte0, uint8_t byte1, uint8_t byte2) : CECBytes (NULL, 0) {
        uint8_t bytes[MAX_LEN];
        bytes[0] = byte0;
        bytes[1] = byte1;
        bytes[2] = byte2;
        str1.insert(str1.begin(), bytes, bytes + MAX_LEN);
	}
    
    //VendorID(const uint8_t *buf, size_t len){
    //}
    VendorID(const uint8_t *buf, size_t len) : CECBytes (buf, (len > MAX_LEN) ? MAX_LEN : len) {
    }

	VendorID(const CECFrame &frame, size_t startPos) : CECBytes (frame, startPos, MAX_LEN) {
    };

protected:
	size_t getMaxLen() const {return MAX_LEN;}

};*/

class PhysicalAddress : public CECBytes
{
    public:
	enum {
		MAX_LEN = 2,
	};
    
    PhysicalAddress();    

	PhysicalAddress(uint8_t byte0, uint8_t byte1, uint8_t byte2, uint8_t byte3) : CECBytes (NULL, 0) {
        uint8_t bytes[MAX_LEN];
        bytes[0] = (byte0 & 0x0F)<< 4 | (byte1 & 0x0F);
        bytes[1] = (byte2 & 0x0F)<< 4 | (byte3 & 0x0F);
        str1.insert(str1.begin(), bytes, bytes + MAX_LEN);
    }

    PhysicalAddress(uint8_t *buf, size_t len = MAX_LEN) : CECBytes(buf, MAX_LEN) {
        //Assert(len >= MAX_LEN);
    }

	PhysicalAddress(const CECFrame &frame, size_t startPos) : CECBytes (frame, startPos, MAX_LEN) {
    };

	PhysicalAddress(std::string &addr)         : CECBytes (NULL, 0) {
		uint8_t byte[4];
		uint8_t bytes[MAX_LEN];
		size_t dotposition = 0;
		int i = 0;

		//Assert((addr.length() != 0 && addr.length() == 7));
		
		while (addr.length()    && i < 4)
		{
		  byte[i++] = stoi(addr,&dotposition,16);
		  if (addr.length() > 1)
		  {
		   	 addr = addr.substr(dotposition+1);
		  }
		  else
		  {
		 	 break; 	
		  }
		}
		
        bytes[0] = (byte[0] & 0x0F)<< 4 | (byte[1] & 0x0F);
        bytes[1] = (byte[2] & 0x0F)<< 4 | (byte[3] & 0x0F);
        str1.insert(str1.begin(), bytes, bytes + MAX_LEN);
    }

	 uint8_t getByteValue( int index) const {
		uint8_t val;

		//Assert(index < 4);

		switch(index)
		{
			case 0: 
			{
				val = (int) ((str1[0] & 0xF0) >> 4);
			}
				break;
			case 1: 
			{
				val = (int) (str1[0] & 0x0F);
			}
				break;
			
			case 2: 
			{
				val = (int) ((str1[1] & 0xF0) >> 4);
			}
				break;

			case 3: 
			{
				val = (int) (str1[1] & 0x0F);
			}
				break;
		}

		return val;
    }

    const std::string toString(void) const {
		std::stringstream stream;
        stream << (int)((str1[0] & 0xF0) >> 4)<< "." << (int)(str1[0] & 0x0F) << "." << (int)((str1[1] & 0xF0) >> 4) << "." << (int)(str1[1] & 0x0F);
		return stream.str();
    }

    const std::string name(void) const {
        return "PhysicalAddress";
    }

protected:
	size_t getMaxLen() const {return MAX_LEN;}
};

class  LogicalAddress : public CECBytes 
{
public:
    enum {
        MAX_LEN = 1,
    };

    static const LogicalAddress kTv;

    enum {
    	TV 						= 0,
    	RECORDING_DEVICE_1 		= 1,
    	RECORDING_DEVICE_2		= 2,
        TUNER_1 				= 3,
        PLAYBACK_DEVICE_1 		= 4,
        AUDIO_SYSTEM 			= 5,
        TUNER_2 				= 6,
        TUNER_3 				= 7,
        PLAYBACK_DEVICE_2 		= 8,
        RECORDING_DEVICE_3 		= 9,
        TUNER_4 				= 10,
        PLAYBACK_DEVICE_3 		= 11,
        RESERVED_12 			= 12,
        RESERVED_13 			= 13,
        SPECIFIC_USE 			= 14,
        UNREGISTERED 			= 15,
        BROADCAST				= UNREGISTERED,
    };

    LogicalAddress(int addr = UNREGISTERED) : CECBytes((uint8_t)addr) { };

    const std::string toString(void) const
    {
    	static const char *names_[] = {
    	"TV",
    	"Recording Device 1",
    	"Recording Device 2",
    	"Tuner 1",
    	"Playback Device 1",
    	"Audio System",
    	"Tuner 2",
    	"Tuner 3",
    	"Playback Device 2",
    	"Recording Device 3",
    	"Tuner 4",
    	"Playback Device 3",
    	"Reserved 12",
    	"Reserved 13",
    	"Specific Use",
    	"Broadcast/Unregistered",
    	};

	if (validate())
	{
		return names_[str1[0]];
	}
	else
	{
		CCEC_LOG(LOG_WARN,"Unknown logical address:%x\n", str1[0]);
		return "Unknown";
	}
    }

	int toInt(void) const {
		return str1[0];
	}

	bool validate(void) const {
		return ((str1[0] <= BROADCAST) && (str1[0] >= TV));
	}

    int getType(void) const {

        if (!validate()) {
            //throw InvalidParamException();
        }

        static int _type[] = {
            DeviceType::TV,
            DeviceType::RECORDING_DEVICE,
            DeviceType::RECORDING_DEVICE,
            DeviceType::TUNER,
            DeviceType::PLAYBACK_DEVICE,
            DeviceType::AUDIO_SYSTEM,
            DeviceType::TUNER,
            DeviceType::TUNER,
            DeviceType::PLAYBACK_DEVICE,
            DeviceType::RECORDING_DEVICE,
            DeviceType::TUNER,
            DeviceType::PLAYBACK_DEVICE,
            DeviceType::RESERVED,
            DeviceType::RESERVED,
            DeviceType::RESERVED,
            DeviceType::RESERVED,
        };

        return _type[str1[0]];
    }

	LogicalAddress(const CECFrame &frame, size_t startPos) : CECBytes (frame, startPos, MAX_LEN) {
    };
protected:
	size_t getMaxLen() const {return MAX_LEN;}
};

class Version : public CECBytes 
{
public:
    enum {
        MAX_LEN = 1,
    };

    enum {
    	V_RESERVED_0,
    	V_RESERVED_1,
    	V_RESERVED_2,
    	V_RESERVED_3,
    	V_1_3a,
    	V_1_4,
    };

	Version(int version) : CECBytes((uint8_t)version) { };

	bool validate(void) const {
		return ((str1[0] <= V_1_4) && (str1[0] >= V_1_3a));
	}

	const std::string toString(void) const
	{
		static const char *names_[] = {
				"Reserved",
				"Reserved",
				"Reserved",
				"Reserved",
				"Version 1.3a",
				"Version 1.4",
		};

		if (validate())
		{
			return names_[str1[0]];
		}
		else
		{
			CCEC_LOG(LOG_WARN,"Unknown version:%x\n", str1[0]);
			return "Unknown";
		}
	}

	Version (const CECFrame &frame, size_t startPos) : CECBytes (frame, startPos, MAX_LEN) {
    };
protected:
	size_t getMaxLen() const {return MAX_LEN;}
};

class PowerStatus : public CECBytes 
{
public:
    enum {
        MAX_LEN = 1,
    };

    enum {
        ON = 0,
        STANDBY = 0x01,
        IN_TRANSITION_STANDBY_TO_ON = 0x02,
        IN_TRANSITION_ON_TO_STANDBY = 0x03,
        POWER_STATUS_NOT_KNOWN = 0x4, 
        POWER_STATUS_FEATURE_ABORT = 0x05,
   };

	PowerStatus(int status) : CECBytes((uint8_t)status) { };

	bool validate(void) const {
		return ((str1[0] <= IN_TRANSITION_ON_TO_STANDBY)/* && (str[0] >= ON)*/);
	}

	const std::string toString(void) const
	{
		static const char *names_[] = {
			"On",
			"Standby",
			"In transition Standby to On",
			"In transition On to Standby",
			"Not Known",
			"Feature Abort"
		};

		if (validate())
		{
			return names_[str1[0]];
		}
		else
		{
			CCEC_LOG(LOG_WARN,"Unknown powerstatus:%x\n", str1[0]);
			return "Unknown";
		}
	}

	int toInt(void) const {
		return str1[0];
	}

	PowerStatus (const CECFrame &frame, size_t startPos) : CECBytes (frame, startPos, MAX_LEN) {
	};

protected:
	size_t getMaxLen() const {return MAX_LEN;}
};

class RequestAudioFormat : public CECBytes
{
  public :
    enum {
        MAX_LEN = 1,
    };

	enum {

          SAD_FMT_CODE_LPCM =1 ,		       // 1
 	  SAD_FMT_CODE_AC3,	      // 2
 	  SAD_FMT_CODE_MPEG1,		    //   3
          SAD_FMT_CODE_MP3,	      // 4
          SAD_FMT_CODE_MPEG2,	  //     5
	  SAD_FMT_CODE_AAC_LC,	      // 6
          SAD_FMT_CODE_DTS,	      // 7
          SAD_FMT_CODE_ATRAC,	      // 8
          SAD_FMT_CODE_ONE_BIT_AUDIO,  // 9
          SAD_FMT_CODE_ENHANCED_AC3,	 // 10
          SAD_FMT_CODE_DTS_HD,	 // 11
          SAD_FMT_CODE_MAT,	    //  12
          SAD_FMT_CODE_DST,	    //  13
          SAD_FMT_CODE_WMA_PRO, 	 // 14
          SAD_FMT_CODE_EXTENDED,		//  15
	};
	RequestAudioFormat(uint8_t AudioFormatIdCode) : CECBytes((uint8_t)AudioFormatIdCode) { };
        RequestAudioFormat(const CECFrame &frame, size_t startPos) : CECBytes (frame, startPos,MAX_LEN) { };
	const std::string toString(void) const
        {
		static const char *AudioFormtCode[] = {
				"Reserved",
				"LPCM",
				"AC3",
				"MPEG1",
				"MP3",
				"MPEG2",
				"AAC",
				"DTS",
				"ATRAC",
				"One Bit Audio",
				"E-AC3",
				"DTS-HD",
				"MAT",
				"DST",
				"WMA PRO",
				"Reserved for Audio format 15",
    	};
		/* audio formt code uses 6 bits but codes are defined only for 15 codes */
    	return AudioFormtCode[str1[0] & 0xF];
        }
	int getAudioformatId(void) const {

            return (str1[0]>>6);
		};

	int getAudioformatCode(void) const {

		return (str1[0] & 0x3F);

	};
protected:
	size_t getMaxLen() const {return MAX_LEN;}
};

class ShortAudioDescriptor : public CECBytes
{
  public :
    enum {
        MAX_LEN = 3,
    };

	enum {

          SAD_FMT_CODE_LPCM =1 ,		       // 1
 		  SAD_FMT_CODE_AC3,	      // 2
 		  SAD_FMT_CODE_MPEG1,		    //   3
          SAD_FMT_CODE_MP3,	      // 4
          SAD_FMT_CODE_MPEG2,	  //     5
		  SAD_FMT_CODE_AAC_LC,	      // 6
          SAD_FMT_CODE_DTS,	      // 7
          SAD_FMT_CODE_ATRAC,	      // 8
          SAD_FMT_CODE_ONE_BIT_AUDIO,  // 9
          SAD_FMT_CODE_ENHANCED_AC3,	 // 10
          SAD_FMT_CODE_DTS_HD,	 // 11
          SAD_FMT_CODE_MAT,	    //  12
          SAD_FMT_CODE_DST,	    //  13
          SAD_FMT_CODE_WMA_PRO, 	 // 14
          SAD_FMT_CODE_EXTENDED,		//  15

	};

	ShortAudioDescriptor(uint8_t *buf, size_t len = MAX_LEN) : CECBytes(buf, MAX_LEN) {
               //Assert(len >= MAX_LEN);
        };
        ShortAudioDescriptor(const CECFrame &frame, size_t startPos) : CECBytes (frame, startPos,MAX_LEN) {
		   };
	const std::string toString(void) const
        {

		static const char *AudioFormtCode[] = {
				"Reserved",
				"LPCM",
				"AC3",
				"MPEG1",
				"MP3",
				"MPEG2",
				"AAC",
				"DTS",
				"ATRAC",
				"One Bit Audio",
				"E-AC3",
				"DTS-HD",
				"MAT",
				"DST",
				"WMA PRO",
				"Reserved for Audio format 15",
    	        };
   	/* audio formt code uses 6 bits but codes are defined only for 15 codes */
        return AudioFormtCode[(str1[0] >> 3) & 0xF];
    }

	uint32_t getAudiodescriptor(void) const	{
         uint32_t audiodescriptor;

		audiodescriptor =  (str1[0] | str1[1] << 8 | str1[2] << 16);
		return audiodescriptor;

	};

	int getAudioformatCode(void) const	{
         uint8_t audioformatCode;
         audioformatCode = ((str1[0] >> 3) & 0xF);
		 return audioformatCode;

	};
	uint8_t getAtmosbit(void) const {

        bool atmosSupport = false;
		if ((((str1[0] >> 3) & 0xF) >= 9) && (((str1[0] >> 3) & 0xF) <= 15))
		{
                      if((str1[2] & 0x3) != 0)
                      {
                         atmosSupport = true;
                      }

		}
            return atmosSupport;
	};

protected:
	size_t getMaxLen() const {return MAX_LEN;}

};
class SystemAudioStatus : public CECBytes
{
public:
    enum {
        MAX_LEN = 1,
    };

    enum {
            OFF = 0x00,
	    ON = 0x01,
         };

	SystemAudioStatus(int status) : CECBytes((uint8_t)status) { };

	bool validate(void) const {
		return ((str1[0] <= ON) );
	}

	const std::string toString(void) const
	{
		static const char *names_[] = {
			"Off",
			"On",
		};

		if (validate())
		{
			return names_[str1[0]];
		}
		else
		{
			CCEC_LOG(LOG_WARN,"Unknown SystemAudioStatus:%x\n", str1[0]);
			return "Unknown";
		}
	}

	int toInt(void) const {
		return str1[0];
	}

	SystemAudioStatus (const CECFrame &frame, size_t startPos) : CECBytes (frame, startPos, MAX_LEN) {
	};

protected:
	size_t getMaxLen() const {return MAX_LEN;}
};
class AudioStatus : public CECBytes
{
   public:
   enum {
        MAX_LEN = 1,
   };

   enum {
          AUDIO_MUTE_OFF = 0x00,
          AUDIO_MUTE_ON  = 0x01,
        };
	AudioStatus(uint8_t status) : CECBytes((uint8_t)status) { };

	bool validate(void) const {
		return (((str1[0] & 0x80) >> 7) <= AUDIO_MUTE_ON) ;
	}
	const std::string toString(void) const
	{
		static const char *names_[] = {
			"Audio Mute Off",
			"Audio Mute On",
		};
		if (validate())
		{
			return names_[((str1[0] & 0x80) >> 7)];
		}
		else
		{
			CCEC_LOG(LOG_WARN,"Unknown Audio Mute Status:%x\n", str1[0]);
			return "Unknown";
		}
	}
	int getAudioMuteStatus(void) const {
            return ((str1[0] & 0x80) >> 7);
           };
	int getAudioVolume(void) const {
		return (str1[0] & 0x7F);
        }
	AudioStatus ( const CECFrame &frame, size_t startPos) : CECBytes (frame, startPos, MAX_LEN) {
	};
protected:
	size_t getMaxLen() const {return MAX_LEN;}
};
class UICommand : public CECBytes
{
public:
    enum {
        MAX_LEN = 1,
    };

    enum {
           UI_COMMAND_VOLUME_UP          = 0x41,
           UI_COMMAND_VOLUME_DOWN        = 0x42,
           UI_COMMAND_MUTE               = 0x43,
           UI_COMMAND_MUTE_FUNCTION      = 0x65,
           UI_COMMAND_RESTORE_FUNCTION   = 0x66,
           UI_COMMAND_POWER_OFF_FUNCTION = 0x6C,
           UI_COMMAND_POWER_ON_FUNCTION  = 0x6D,
	   UI_COMMAND_UP                 = 0x01,
	   UI_COMMAND_DOWN               = 0x02,
	   UI_COMMAND_LEFT               = 0x03,
	   UI_COMMAND_RIGHT              = 0x04,
	   UI_COMMAND_SELECT             = 0x00,
	   UI_COMMAND_HOME               = 0x09,
	   UI_COMMAND_BACK               = 0x0D,
	   UI_COMMAND_NUM_0              = 0x20,
	   UI_COMMAND_NUM_1              = 0x21,
	   UI_COMMAND_NUM_2              = 0x22,
	   UI_COMMAND_NUM_3              = 0x23,
	   UI_COMMAND_NUM_4              = 0x24,
	   UI_COMMAND_NUM_5              = 0x25,
	   UI_COMMAND_NUM_6              = 0x26,
	   UI_COMMAND_NUM_7              = 0x27,
	   UI_COMMAND_NUM_8              = 0x28,
	   UI_COMMAND_NUM_9              = 0x29,
        };

    UICommand(int command) : CECBytes((uint8_t)command) { };

    int toInt(void) const {
        return str1[0];
    }

protected:
	size_t getMaxLen() const {return MAX_LEN;}
};


//CCEC_END_NAMESPACE

//#endif
inline const char *GetOpName(Op_t op)
{

    return "name";
}


class MessageProcessor {
    public:
        MessageProcessor(void){}
        virtual ~MessageProcessor(void){}

};

class ActiveSourceImpl{
    public:
        virtual Op_t opCode() const = 0;
};
class ActiveSource {
    public:
    Op_t opCode(void) const {return ACTIVE_SOURCE;}

	ActiveSource(PhysicalAddress &phyAddress) : physicalAddress(phyAddress) {
    }

	ActiveSource(const CECFrame &frame, int startPos = 0) 
    : physicalAddress(frame, startPos)
    {
    }
	CECFrame &serialize(CECFrame &frame) const {
        CCEC_LOG( LOG_DEBUG, "%s \n",physicalAddress.toString().c_str());
        return physicalAddress.serialize(frame);
	}

    void print(void) const {
        CCEC_LOG( LOG_DEBUG, "%s : %s  : %s \n",GetOpName(opCode()),physicalAddress.name().c_str(),physicalAddress.toString().c_str());
    }
public:
    PhysicalAddress physicalAddress;
};

class InActiveSource{
    public:
    Op_t opCode(void) const{
              //return impl-> opCode();
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

class RequestActiveSource{


};

class Standby{


};

class CECVersion {
    public:
    Op_t opCode(void) const {return CEC_VERSION;}

    CECVersion(const Version &ver) : version(ver) {}

    CECVersion(const CECFrame &frame, int startPos = 0)
    : version(frame, startPos)
    {
    }

	CECFrame &serialize(CECFrame &frame) const {
        return version.serialize(frame);
	}

    void print(void) const {
        CCEC_LOG( LOG_DEBUG, "Version : %s \n",version.toString().c_str());
    }

    Version version;

};

class GetCECVersion{


};

class GetMenuLanguage{
    public:
        Language language;
};

class SetMenuLanguage{
    public:
        SetMenuLanguage(const Language &lan) : language(lan) {};
        //SetMenuLanguage(const CECFrame &frame, int startPos = 0) : language(frame, startPos) {}
        const Language language;

};

class GiveOSDName{


};

class GivePhysicalAddress{


};

class GiveDeviceVendorID{


};

class SetOSDString{
    public:
    OSDString osdString;


};

class RoutingChange{
    public:
     Op_t opCode(void) const {return ROUTING_CHANGE;}

    RoutingChange(PhysicalAddress &from1, const PhysicalAddress &to1) : from(from1), to(to1) {}

    RoutingChange(const CECFrame &frame, int startPos = 0)
    : from(frame, startPos), to(frame, startPos + PhysicalAddress::MAX_LEN)
    {
    }

	CECFrame &serialize(CECFrame &frame) const {
		return to.serialize(from.serialize(frame));
	}

    void print(void) const {
        CCEC_LOG( LOG_DEBUG,"Routing Change From : %s\n",from.toString().c_str());
        CCEC_LOG( LOG_DEBUG,"Routing Change to : %s\n",to.toString().c_str());
    }

    PhysicalAddress from;
    PhysicalAddress to;

};

class RoutingInformation {
    public:
    PhysicalAddress toSink;

};

class SetStreamPath{
    public:
     Op_t opCode(void) const {return SET_STREAM_PATH;}

    SetStreamPath(const PhysicalAddress &toSink1) : toSink(toSink1) {}

    SetStreamPath(const CECFrame &frame, int startPos = 0)
    : toSink(frame, startPos)
    {
    }

	CECFrame &serialize(CECFrame &frame) const {
		return toSink.serialize(frame);
	}

    void print(void) const {
        CCEC_LOG( LOG_DEBUG,"Set Stream Path to Sink : %s\n",toSink.toString().c_str());
    }

    PhysicalAddress from;
    PhysicalAddress to;
    PhysicalAddress toSink;

};


class GiveDevicePowerStatus{


};
class ReportPhysicalAddress {
    public:
    Op_t opCode(void) const {return REPORT_PHYSICAL_ADDRESS;}

    ReportPhysicalAddress(PhysicalAddress &physAddress, const DeviceType &devType)
    : physicalAddress(physAddress), deviceType(devType) {
    }

    ReportPhysicalAddress(const CECFrame &frame, int startPos = 0)
    : physicalAddress(frame, startPos), deviceType(frame, startPos + PhysicalAddress::MAX_LEN)
    {
    }

	CECFrame &serialize(CECFrame &frame) const {
		return deviceType.serialize(physicalAddress.serialize(frame));
	}

    void print(void) const {
        CCEC_LOG( LOG_DEBUG,"Physical Address : %s\n",physicalAddress.toString().c_str());
        CCEC_LOG( LOG_DEBUG,"Device Type : %s\n",deviceType.toString().c_str());
    }

	PhysicalAddress physicalAddress;
	DeviceType deviceType;
};


class DeviceVendorID {
    public:
        DeviceVendorID(const VendorID &vendor) : vendorId(vendor){}
        VendorID vendorId;
};

class ReportPowerStatus{
    public:
        ReportPowerStatus(PowerStatus stat) : status(stat) {}
        PowerStatus status;

};

class OpCode{
    public:
    enum {
        MAX_LEN = 1,
    };

    OpCode(Op_t opCode) : opCode_(opCode) {};
	OpCode(const CECFrame &frame, int startPos) : opCode_(frame.at(startPos)) {
    }
	CECFrame &serialize(CECFrame &frame) const {
        if (opCode_ != POLLING) {
            frame.append(opCode_);
        }
        return frame;
    }
    std::string toString(void) const {return GetOpName(opCode_);}
    Op_t opCode(void) const {return FEATURE_ABORT;}
    private:
        Op_t opCode_;
};

class FeatureAbort{
public:
    Op_t opCode(void) const {return FEATURE_ABORT;}

    FeatureAbort(const OpCode &abfeature, const AbortReason &abreason) : feature(abfeature), reason(abreason) {}

    FeatureAbort(const CECFrame &frame, int startPos = 0)
    : feature(frame, startPos), reason(frame, startPos + OpCode::MAX_LEN)
    {
    }

	CECFrame &serialize(CECFrame &frame) const {
		return reason.serialize(feature.serialize(frame));
	}

    void print(void) const {
        CCEC_LOG( LOG_DEBUG,"Abort For Feature : %s\n",feature.toString().c_str());
        CCEC_LOG( LOG_DEBUG,"Abort Reason : %s\n",reason.toString().c_str());
    }

    OpCode feature;
    AbortReason reason;
};

class Abort{
    public:
    Op_t opCode(void) const{
            //return impl-> opCode();
            return 1;
        }


};

class UserControlReleased {


};


class Polling{


};

class RequestShortAudioDescriptor {
    public:
    Op_t opCode(void) const {return REQUEST_SHORT_AUDIO_DESCRIPTOR;}

      RequestShortAudioDescriptor(const std::vector<uint8_t> formatid, const std::vector<uint8_t> audioFormatCode, uint8_t number_of_descriptor = 1)
      {
	    uint8_t audioFormatIdCode;
	    numberofdescriptor = number_of_descriptor > 4 ? 4 : number_of_descriptor;
	    for (uint8_t i=0 ; i < numberofdescriptor ;i++)
	    {
		   audioFormatIdCode = (formatid[i] << 6) | ( (audioFormatCode[i])& 0x3f) ;
		   requestAudioFormat.push_back(RequestAudioFormat(audioFormatIdCode));
	    }
       }
	 /* called by the messaged_decoder */
     RequestShortAudioDescriptor(const CECFrame &frame, int startPos = 0)
     {
	uint8_t len = frame.length();
        numberofdescriptor = len > 4 ? 4:len;
        for (uint8_t i=0; i< numberofdescriptor ; i++)
        {
	   requestAudioFormat.push_back(RequestAudioFormat(frame,startPos + i ));
        }
     }
     /* called by the message encoder */
     CECFrame &serialize(CECFrame &frame) const {

	  for (uint8_t i=0; i < numberofdescriptor ; i++)
	  {
	  requestAudioFormat[i].serialize(frame);

	  }
	  return frame;
	}

     /*void print(void) const {
	    uint8_t i=0;
		for(i=0;i < numberofdescriptor;i++)
		{

                  CCEC_LOG( LOG_DEBUG,"audio format id %d audioFormatCode : %s\n",requestAudioFormat[i].getAudioformatId(),requestAudioFormat[i].toString());

		}
      }*/
     std::vector<RequestAudioFormat> requestAudioFormat ;
     uint8_t  numberofdescriptor;
};

class UserControlPressed {
public:
    Op_t opCode(void) const {return USER_CONTROL_PRESSED;}

	UserControlPressed( const UICommand &command ) : uiCommand(command) { }

	CECFrame &serialize(CECFrame &frame) const {
		return uiCommand.serialize(frame);
	}

	UICommand uiCommand;

};

class SetOSDName;

/*class MessageEncoderImpl{
        public:
                virtual CECFrame encode(const ActiveSource &source) const = 0;
                virtual CECFrame encode(const CECVersion &version) const = 0;
                virtual CECFrame encode(const SetOSDName &osdName) const = 0;
                virtual CECFrame encode(const ReportPhysicalAddress &physAddress) const = 0;
                virtual CECFrame encode(const DeviceVendorID &vendorId) const = 0;
                virtual CECFrame encode(const Standby &standby) const = 0;
                virtual CECFrame encode(const GiveDevicePowerStatus &status) const = 0;
                virtual CECFrame encode(const ImageViewOn &view) const = 0;
                virtual CECFrame encode(const RequestActiveSource &source) const = 0;
                virtual CECFrame encode(const ReportPowerStatus &status) const = 0;
                virtual CECFrame encode(const FeatureAbort &feature) const = 0;
                virtual CECFrame encode(const UserControlReleased &control) const = 0;
                virtual CECFrame encode(const GiveOSDName &msg) const = 0;
                virtual CECFrame encode(const GivePhysicalAddress &physicalAddres) const = 0;
                virtual CECFrame encode(const UserControlPressed &controlPressed) const = 0; 
                virtual CECFrame encode(const RequestShortAudioDescriptor &shortAudioDes) const = 0;
                virtual CECFrame encode(const SetMenuLanguage &setMenuLang) const = 0;
                virtual CECFrame encode(const GiveAudioStatus &giveaudiosts) const = 0;
                virtual CECFrame encode(const SystemAudioModeRequest &audioModeReq) const = 0;

};
*/
#if 0
class MessageEncoder {
        private:
                //ActiveSource activeSource;


        public:
                MessageEncoderImpl* impl;
                MessageEncoder(){};
                /*MessageDecoder& getInstance(){
                        static MessageDecoder instance;
                        return instance;
                }*/
                //MessageDecoder(MessageProcessor & processor) : processor(processor){};
                CECFrame encode(const ActiveSource &source){
                        return impl->encode(source);
                }
                CECFrame encode(const CECVersion &version){
                        return impl->encode(version);
                }
                CECFrame encode(const SetOSDName &osdName){
                        return impl->encode(osdName);
                }
                CECFrame encode(const ReportPhysicalAddress &physAddress){
                        return impl->encode(physAddress);
                }
                CECFrame encode(const DeviceVendorID &vendorId){
                        return impl->encode(vendorId);
                }
                CECFrame encode(const Standby &standby){
                        return impl->encode(standby);
                }
                CECFrame encode(const GiveDevicePowerStatus &status){
                        return impl->encode(status);
                }

                CECFrame encode(const ImageViewOn &view){
                        return impl->encode(view);
                }
                CECFrame encode(const RequestActiveSource &source){
                        return impl->encode(source);
                }
                CECFrame encode(const ReportPowerStatus &status){
                        return impl->encode(status);
                }
                CECFrame encode(const FeatureAbort &feature){
                        return impl->encode(feature);
                }
                CECFrame encode(const UserControlReleased &control){
                        return impl->encode(control);
                }
                CECFrame encode(const GivePhysicalAddress &physicalAddres){
                        return impl->encode(physicalAddres);
                }
                CECFrame encode(const UserControlPressed &controlPressed){
                    return impl->encode(controlPressed);
                }

                CECFrame encode(const RequestShortAudioDescriptor &shortAudioDes){
                    return impl->encode(shortAudioDes);
                }
                CECFrame encode(const SetMenuLanguage &setMenuLang){
                    return impl->encode(setMenuLang);
                }
                CECFrame encode(const GiveAudioStatus &giveaudiosts){
                    return impl->encode(giveaudiosts);
                }				
                CECFrame encode(const SystemAudioModeRequest &audioModeReq){
                    return impl->encode(audioModeReq);
                }		

};
#endif
class ReportAudioStatus
{

public:
    Op_t opCode(void) const {return REPORT_AUDIO_STATUS;}

    ReportAudioStatus(AudioStatus &audiostatus ) : status(audiostatus) { }
    ReportAudioStatus(const CECFrame &frame, int startPos = 0):status(frame, startPos)
    {
    }
    CECFrame &serialize(CECFrame &frame) const {
    return status.serialize(frame);
    }
    AudioStatus status;

};

class SetSystemAudioMode
{

public:
    Op_t opCode(void) const {return SET_SYSTEM_AUDIO_MODE;}

    SetSystemAudioMode(SystemAudioStatus &systemstatus ) : status(systemstatus) { }

    SetSystemAudioMode(const CECFrame &frame, int startPos = 0) : status(frame, startPos)
       {
       }
    CECFrame &serialize(CECFrame &frame) const {
        return status.serialize(frame);
    }

    SystemAudioStatus status;
};

class ReportShortAudioDescriptor 
{
public:
    Op_t opCode(void) const {return REPORT_SHORT_AUDIO_DESCRIPTOR;}

	      ReportShortAudioDescriptor( std::vector <uint32_t> shortaudiodescriptor, uint8_t numberofdes=1)
	      {

	       	uint8_t bytes[3];
	        numberofdescriptor = numberofdes > 4 ? 4 : numberofdes;
	        for (uint8_t i=0; i < numberofdescriptor ;i++)
	        {
                  bytes[0] = (shortaudiodescriptor[i] & 0xF);
	          bytes[1] = ((shortaudiodescriptor[i] >> 8) & 0xF);
	          bytes[2] = ((shortaudiodescriptor[i] >> 16) & 0xF);
	          shortAudioDescriptor.push_back(ShortAudioDescriptor(bytes));

	        }
	      }
	       // called by the messaged_decoder 
             ReportShortAudioDescriptor(const CECFrame &frame, int startPos = 0)
            {
               numberofdescriptor = (frame.length())/3;
               for (uint8_t i=0; i< numberofdescriptor ;i++)
	       {
	          shortAudioDescriptor.push_back(ShortAudioDescriptor(frame,startPos + i*3 ));
	       }
             }

	     // called by the message encoder
	   CECFrame &serialize(CECFrame &frame) const {
		 for (uint8_t i=0; i < numberofdescriptor ; i++)
		 {
		 //just do the append of the stored cec bytes
		 shortAudioDescriptor[i].serialize(frame);

		 }
		 return frame;
	   }
           void print(void) const {
                for(uint8_t i=0;i < numberofdescriptor;i++)
                {
			CCEC_LOG( LOG_DEBUG," audioFormatCode : %s audioFormatCode %d Atmos = %d\n",shortAudioDescriptor[i].toString(),shortAudioDescriptor[i].getAudioformatCode(),shortAudioDescriptor[i].getAtmosbit());
		}
	   }
    std::vector <ShortAudioDescriptor> shortAudioDescriptor ;
    uint8_t numberofdescriptor;
};

class InitiateArc
{
//  public:
  //  Op_t opCode(void) const {return INITIATE_ARC;}
};

class TerminateArc
{
  //public:
  //  Op_t opCode(void) const {return TERMINATE_ARC;}

};


class MessageDecoderImpl{
    public:
        virtual void decode(const CECFrame &in) const = 0;

};
class MessageDecoder {
    private:
                MessageProcessor &processor;


    public:
        MessageDecoderImpl* impl;
        MessageDecoder(MessageProcessor & proc) : processor(proc){};

        void decode(const CECFrame &in){
            return impl->decode(in);
        }

};

class SetOSDName{
    public:
    Op_t opCode(void) const {return SET_OSD_NAME;}

    SetOSDName(OSDName &OsdName) : osdName(OsdName) {};

    SetOSDName(const CECFrame &frame, int startPos = 0)
    : osdName(frame, startPos)
    {
    }

	CECFrame &serialize(CECFrame &frame) const {
	    return osdName.serialize(frame);
	}

    void print(void) const {
        CCEC_LOG( LOG_DEBUG,"OSDName : %s\n",osdName.toString().c_str());
    }

    OSDName osdName;
};

class FrameListener
{
    public:
        virtual void notify(const CECFrame &) const = 0;
        virtual ~FrameListener(void){}

};


class ConnectionImpl{
    public:
        virtual ~ConnectionImpl() = default;
        virtual void open() const = 0;
        virtual void close() const = 0;
        virtual void addFrameListener(FrameListener *listener) const = 0;
        virtual void removeFrameListener(FrameListener *listener) const = 0;
        virtual void sendAsync(const CECFrame &frame) const = 0;
        virtual void ping(const LogicalAddress &from, const LogicalAddress &to, const Throw_e &doThrow) const = 0;
        virtual void sendToAsync(const LogicalAddress &to, const CECFrame &frame) const =0;
        virtual void sendTo(const LogicalAddress &to, const CECFrame &frame) const = 0;
        virtual void sendTo(const LogicalAddress &to, const CECFrame &frame, int timeout) const = 0;
        virtual void poll(const LogicalAddress &from, const Throw_e &doThrow) const =0;
};

class Connection{
    public:
        ConnectionImpl* impl;
        void updateImpl(ConnectionImpl* imp){
            //impl* = imp;
        }
        //Connection(const LogicalAddress &source = LogicalAddress::UNREGISTERED){}
        Connection(const LogicalAddress &source = LogicalAddress::UNREGISTERED, bool opened = true, const std::string &name="")
        {

        }

    void open(void){
        return impl->open();
    }
    void close(void){
        return impl->close();
    }
    void addFrameListener(FrameListener *listener){
        return impl->addFrameListener(listener);
    }
    void removeFrameListener(FrameListener *listener){
        return impl->removeFrameListener(listener);
    }
    void sendAsync(const CECFrame &frame){
        return impl->sendAsync(frame);
    }
    void ping(const LogicalAddress &from, const LogicalAddress &to, const Throw_e &doThrow){
        return impl->ping(from, to, doThrow);
    }

    void sendToAsync(const LogicalAddress &to, const CECFrame &frame)
    {
        return impl->sendToAsync(to, frame);
    }

     void sendTo(const LogicalAddress &to, const CECFrame &frame){
        return impl->sendTo(to, frame);
    }
    void sendTo(const LogicalAddress &to, const CECFrame &frame, int timeout){
        return impl->sendTo(to, frame, timeout);
    }

    void poll(const LogicalAddress &from, const Throw_e &doThrow){
        return impl->poll(from, doThrow);
    }

    void setSource(LogicalAddress &from) {
		//return impl->setSource(from);
        //source = from;
	}
};

class SystemAudioModeRequest 
{

public:
    Op_t opCode(void) const {return SYSTEM_AUDIO_MODE_REQUEST;}
	SystemAudioModeRequest(const PhysicalAddress &physicaladdress = {0xf,0xf,0xf,0xf} ): _physicaladdress(physicaladdress) {}
	 /* called by the messaged_decoder */
	SystemAudioModeRequest(const CECFrame &frame, int startPos = 0):_physicaladdress(frame, startPos)
        {
           if (frame.length() == 0 )
	   {
              _physicaladdress= PhysicalAddress((uint8_t) 0xf,(uint8_t) 0xf,(uint8_t)0xf,(uint8_t)0xf);
	   }
        }
      CECFrame &serialize(CECFrame &frame) const {
        if ( (_physicaladdress.getByteValue(3) == 0xF) && (_physicaladdress.getByteValue(2) == 0xF) && (_physicaladdress.getByteValue(1) == 0xF) &&  (_physicaladdress.getByteValue(0) == 0xF))
	{
	    return frame;
	 } else{
                return _physicaladdress.serialize(frame);
	 }
	 }
     void print(void) const {
        CCEC_LOG( LOG_DEBUG,"Set SystemAudioModeRequest : %s\n",_physicaladdress.toString().c_str());
    }
  PhysicalAddress _physicaladdress;
};

class GiveAudioStatus 
{
public:
    Op_t opCode(void) const {return GIVE_AUDIO_STATUS;}
};

class LibCCECImpl{
    public:
        virtual void init(const char *name) const = 0;
        virtual void init() const = 0;
        virtual void term() const = 0;
        virtual void getPhysicalAddress(uint32_t *physicalAddress) const  = 0;
        virtual int getLogicalAddress(int devType) const = 0;
        virtual int addLogicalAddress(const LogicalAddress &source) const = 0;
};
class LibCCEC{
    public:
        LibCCEC(void){};

        static LibCCEC& getInstance()
        {
            static LibCCEC instance;
            return instance;
        };
        LibCCECImpl* impl;
        void init(const char *name){
            return impl->init(name);
        }
        void init(){
                        return impl->init();
                }
        void term(){
            return;
        }
        void getPhysicalAddress(uint32_t *physicalAddress){
            return impl->getPhysicalAddress(physicalAddress);
        }
        int getLogicalAddress(int devType){
                        return impl->getLogicalAddress(devType);
                }

         int addLogicalAddress(const LogicalAddress &source){
                        return impl->addLogicalAddress(source);
                }

};

class RequestArcInitiation
{
  public:
    Op_t opCode(void) const {return REQUEST_ARC_INITIATION;}

};

class ReportArcInitiation
{
  public:
    Op_t opCode(void) const {return REPORT_ARC_INITIATED;}

};

class RequestArcTermination
{
  public:
    Op_t opCode(void) const {return REQUEST_ARC_TERMINATION;}

};

class ReportArcTermination
{
  public:
    Op_t opCode(void) const {return REPORT_ARC_TERMINATED;}

};


class MessageEncoderImpl{
        public:
                virtual CECFrame encode(const ActiveSource &source) const = 0;
                virtual CECFrame encode(const CECVersion &version) const = 0;
                virtual CECFrame encode(const SetOSDName &osdName) const = 0;
                virtual CECFrame encode(const ReportPhysicalAddress &physAddress) const = 0;
                virtual CECFrame encode(const DeviceVendorID &vendorId) const = 0;
                virtual CECFrame encode(const Standby &standby) const = 0;
                virtual CECFrame encode(const GiveDevicePowerStatus &status) const = 0;
                virtual CECFrame encode(const ImageViewOn &view) const = 0;
                virtual CECFrame encode(const RequestActiveSource &source) const = 0;
                virtual CECFrame encode(const ReportPowerStatus &status) const = 0;
                virtual CECFrame encode(const FeatureAbort &feature) const = 0;
                virtual CECFrame encode(const UserControlReleased &control) const = 0;
                virtual CECFrame encode(const GiveOSDName &msg) const = 0;
                virtual CECFrame encode(const GivePhysicalAddress &physicalAddres) const = 0;
                virtual CECFrame encode(const UserControlPressed &controlPressed) const = 0;
                virtual CECFrame encode(const RequestShortAudioDescriptor &shortAudioDes) const = 0;
                virtual CECFrame encode(const SetMenuLanguage &setMenuLang) const = 0;
                virtual CECFrame encode(const GiveAudioStatus &giveaudiosts) const = 0;
                virtual CECFrame encode(const SystemAudioModeRequest &audioModeReq) const = 0;
                virtual CECFrame encode(const GiveDeviceVendorID &vendorId) const = 0;
                virtual CECFrame encode(const GetCECVersion &getCecVersion) const = 0;
                virtual CECFrame encode(const RequestArcInitiation &arcInit) const = 0;
                virtual CECFrame encode(const ReportArcInitiation &arcInit) const = 0;
                virtual CECFrame encode(const RequestArcTermination &arcTermReq) const = 0;
                virtual CECFrame encode(const ReportArcTermination &arcTermRep) const = 0;
                virtual CECFrame encode(const SetStreamPath &streamPath) const = 0;
                virtual CECFrame encode(const RoutingChange &routeChange) const = 0;
};

class MessageEncoder {
        private:
                //ActiveSource activeSource;


        public:
                MessageEncoderImpl* impl;
                MessageEncoder(){};
                /*MessageDecoder& getInstance(){
                        static MessageDecoder instance;
                        return instance;
                }*/
                //MessageDecoder(MessageProcessor & processor) : processor(processor){};
                CECFrame encode(const ActiveSource &source){
                        return impl->encode(source);
                }
                CECFrame encode(const CECVersion &version){
                        return impl->encode(version);
                }
                CECFrame encode(const SetOSDName &osdName){
                        return impl->encode(osdName);
                }
                CECFrame encode(const ReportPhysicalAddress &physAddress){
                        return impl->encode(physAddress);
                }
                CECFrame encode(const DeviceVendorID &vendorId){
                        return impl->encode(vendorId);
                }
                CECFrame encode(const Standby &standby){
                        return impl->encode(standby);
                }
                        
                CECFrame encode(const GiveDevicePowerStatus &status){
                        return impl->encode(status);
                }

                CECFrame encode(const ImageViewOn &view){
                        return impl->encode(view);
                }
                CECFrame encode(const RequestActiveSource &source){
                        return impl->encode(source);
                }
                CECFrame encode(const GiveOSDName &msg){
                        return impl->encode(msg);
                }
                CECFrame encode(const ReportPowerStatus &status){
                        return impl->encode(status);
                }
                CECFrame encode(const FeatureAbort &feature){
                        return impl->encode(feature);
                }
                CECFrame encode(const UserControlReleased &control){
                        return impl->encode(control);
                }
                CECFrame encode(const GivePhysicalAddress &physicalAddres){
                        return impl->encode(physicalAddres);
                }
                CECFrame encode(const UserControlPressed &controlPressed){
                    return impl->encode(controlPressed);
                }

                CECFrame encode(const RequestShortAudioDescriptor &shortAudioDes){
                    return impl->encode(shortAudioDes);
                }
                CECFrame encode(const SetMenuLanguage &setMenuLang){
                    return impl->encode(setMenuLang);
                }
                CECFrame encode(const GiveAudioStatus &giveaudiosts){
                    return impl->encode(giveaudiosts);
                }
                CECFrame encode(const SystemAudioModeRequest &audioModeReq){
                    return impl->encode(audioModeReq);
                }
                CECFrame encode(const GiveDeviceVendorID &vendorId){
                    return impl->encode(vendorId);
                }

                CECFrame encode(const GetCECVersion &getCecVersion){
                    return impl->encode(getCecVersion);
                }
                CECFrame encode(const RequestArcInitiation &arcInit){
                    return impl->encode(arcInit);
                }

                CECFrame encode(const ReportArcInitiation &arcInit){
                    return impl->encode(arcInit);
                }
                CECFrame encode(const ReportArcTermination &arcTermRep){
                    return impl->encode(arcTermRep);
                }
                CECFrame encode(const RequestArcTermination &arcTermReq){
                    return impl->encode(arcTermReq);
                }

                CECFrame encode(const SetStreamPath &streamPath){
                    return impl->encode(streamPath);
                }
                CECFrame encode(const RoutingChange &routeChange){
                    return impl->encode(routeChange);
                }

};


typedef struct _dsHdmiInGetNumberOfInputsParam_t
{
    dsError_t   result;
    uint8_t     numHdmiInputs;
} dsHdmiInGetNumberOfInputsParam_t;

/*typedef enum _dsHdmiInPort_t
{
    dsHDMI_IN_PORT_NONE = -1,
    dsHDMI_IN_PORT_0,
    dsHDMI_IN_PORT_1,
    dsHDMI_IN_PORT_2,
    dsHDMI_IN_PORT_MAX
} dsHdmiInPort_t;
*/
typedef struct _dsHdmiInStatus_t
{
    bool             isPresented;                          /**< Boolean flag indicating Hdmi Input is enabled for presentation by client */
    bool             isPortConnected[dsHDMI_IN_PORT_MAX];  /**< Boolean flag indicating Hdmi source connected to this Hdmi Input port */
    dsHdmiInPort_t   activePort;                           /**< Hdmi Input Port selected as the currently active port (to the set-top)
                                                              - note that only one HDMI Input port can be active at a time */
} dsHdmiInStatus_t;

typedef struct _dsHdmiInGetStatusParam_t
{
    dsError_t           result;
    dsHdmiInStatus_t    status;
} dsHdmiInGetStatusParam_t;

typedef struct _dsGetHDMIARCPortIdParam_t {
        dsError_t result;
        int portId;
} dsGetHDMIARCPortIdParam_t;
/** @} */
/** @} */
CCEC_END_NAMESPACE

#endif
