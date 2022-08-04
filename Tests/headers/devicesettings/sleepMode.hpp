
#pragma once
#include <vector>
#include "list.hpp"

namespace device {
    class SleepMode /*: public DSConstant*/ {
        public:
            SleepMode() = default;
            static SleepMode & getInstance(int id);
            static SleepMode & getInstance(const std::string &name);
            List<SleepMode> getSleepModes();
            SleepMode(int id);
            const std::string & toString() const;
            virtual ~SleepMode() = default;
    };
}
