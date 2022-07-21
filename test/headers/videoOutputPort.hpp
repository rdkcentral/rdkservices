#pragma once

#include <string>
#include <vector>
#include <stdint.h>
#include "enumerable.hpp"

using namespace std;

namespace device {
class VideoOutputPortType;
class AudioOutputPort;


class  VideoOutputPort : public Enumerable {


	public:

    class Display {
	    AudioOutputPort &getAudioOutputPort();

    };
};
}
