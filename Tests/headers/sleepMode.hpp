#pragma once
#include <vector>

namespace device {

template <class T>
using List = std::vector<T>;

class SleepMode {

public:
        
        static SleepMode & getInstance(int id);
        static SleepMode & getInstance(const std::string &name);
        List<SleepMode> getSleepModes();
        SleepMode(int id);
	    string toString();
        virtual ~SleepMode();

};
}
