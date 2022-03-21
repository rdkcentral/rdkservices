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
* @defgroup devicesettings
* @{
* @defgroup ds
* @{
**/


#ifndef _DS_HOST_HPP_
#define _DS_HOST_HPP_

#include <iostream>
//#include "powerModeChangeListener.hpp"
//#include "displayConnectionChangeListener.hpp"
//#include "audioOutputPort.hpp"
//#include "videoOutputPort.hpp"
//#include "videoDevice.hpp"
//#include "sleepMode.hpp"
#include  "list.hpp"
#include  <list>
#include <string>

/**
 * @file host.hpp
 * @brief It contains class,structures referenced by host.cpp file.
 */
using namespace std;

namespace device {

class VideoDevice
{
	public:
      VideoDevice(int id){}
static VideoDevice & getInstance(int id)
{
	VideoDevice obj(0);
	return obj;
}
int getFRFMode(int *frfmode) const
{
	return 0;
}
int setFRFMode(int frfmode) const
{
	return 0;
}
int getCurrentDisframerate(char *framerate) const
{
	return 0;
}
int setDisplayframerate(const char *framerate) const
{
	return 0;
}
int getId() const
{
	return 0;
}
};
/**
 * @class Host
 * @brief Class to implement the Host interface.
 * @ingroup devicesettingsclass
 */
class Host {
public:
    static const int kPowerOn;
    static const int kPowerOff;
    static const int kPowerStandby;

static Host& getInstance(void)
{
	static Host instance;
	static bool isFirstTime = true;
	try {
		 if (isFirstTime) {
		 isFirstTime = false;
	 }
		 else {
	 	}
	}
	catch (...) {
		 cout << "Host Exception Thrown ...!\n";
	 }
	 return instance;
}

List<VideoDevice> getVideoDevices()
{
    	List<VideoDevice> ret;
	VideoDevice obj(1);
	for(int i=0;i<3;i++)
		ret.push_back(obj);
	return ret;
}
private:
        Host(){}
        virtual ~Host(){}
    Host (const Host&);
    Host& operator=(const Host&);
};

}

#endif /* _DS_HOST_HPP_ */


/** @} */
/** @} */
               
