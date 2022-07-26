#ifndef _DS_AUDIOOUTPUTPORT_HPP_
#define _DS_AUDIOOUTPUTPORT_HPP_

#include "enumerable.hpp"


namespace device {

	class AudioOutputPort  : public Enumerable {

		public:

		bool isAudioMSDecode() const;


	};
}

#endif
