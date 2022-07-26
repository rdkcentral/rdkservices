#pragma once
#include <vector>

namespace device {

template <class T>
using List = std::vector<T>;


class DSConstant  {
	private:

	bool enabled;
	protected:
        int _id;
	std::string _name;
	static bool isValid(int min, int max, int val) {
                return (val >= min && val < max);
        }

public:
	DSConstant() : enabled(false), _id(0), _name("_UNASSIGNED NAME_"){};

};


class SleepMode {

public:
        static const int kLightSleep;  //!< Indicates light sleep mode.
        static const int kDeepSleep;   //!< Indicates deep sleep mode.
        static const int kMax;         

        static SleepMode & getInstance(int id);
        static SleepMode & getInstance(const std::string &name);
        List<SleepMode> getSleepModes();
        SleepMode(int id);
	string toString();
        virtual ~SleepMode();

};

/*
class VideoDevice {
public:
    virtual ~VideoDevice() = default;

    virtual int getFRFMode(int* frfmode) const = 0;
    virtual int setFRFMode(int frfmode) const = 0;
    virtual int getCurrentDisframerate(char* framerate) const = 0;
    virtual int setDisplayframerate(const char* framerate) const = 0;
};


class HostImpl {
public:
    virtual ~HostImpl() = default;

    virtual List<std::reference_wrapper<VideoDevice>> getVideoDevices() = 0;
};


class Host {
public:
    static Host& getInstance()
    {
        static Host instance;
        return instance;
    }

    HostImpl* impl;

    SleepMode getPreferredSleepMode();
    int setPreferredSleepMode(const SleepMode);
    List <SleepMode>  getAvailableSleepModes();


    List<std::reference_wrapper<VideoDevice>> getVideoDevices()
    {
        return impl->getVideoDevices();
    }
};
*/
}
