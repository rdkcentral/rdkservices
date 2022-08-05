
#pragma once
#include <vector>
namespace device {

    template <class T>
    using List = std::vector<T>;


    class SleepMode /*: public DSConstant*/ {
        public:
            static const int kLightSleep;
            static const int kDeepSleep;
            static const int kMax;
            SleepMode() = default;
            static SleepMode & getInstance(int id);
            static SleepMode & getInstance(const std::string &name);
            List<SleepMode> getSleepModes();
            SleepMode(int id);
    	    std::string toString(){return "";};
            virtual ~SleepMode() = default;
    };
}



