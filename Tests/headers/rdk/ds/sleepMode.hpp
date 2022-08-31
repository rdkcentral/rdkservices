
#pragma once
#include <vector>
#include "list.hpp"

#ifndef UNUSED
#define UNUSED(x) (void)(x)
#endif

namespace device {
    class SleepMode;
    class SleepModeImpl {
        public:
            virtual ~SleepModeImpl() = default;
            virtual SleepMode& getInstance(int id) = 0;
            virtual SleepMode& getInstance(const std::string& name) = 0;
            virtual List<SleepMode> getSleepModes() = 0;
            virtual const std::string& toString() const;
    };

    class SleepMode /*: public DSConstant*/ {
        public:
            SleepModeImpl* impl;

            static SleepMode& getInstance()
            {
                static SleepMode instance;
                return instance;
            }

            static SleepMode& getInstance(int id)
            {
                UNUSED(id);
                return getInstance();
            }

            static SleepMode& getInstance(const std::string& name)
            {
                UNUSED(name);
                return getInstance();
            }

            List<SleepMode> getSleepModes()
            {
                return impl->getSleepModes();
            }

            const std::string& toString() const
            {
                return impl->toString();
            }
    };
}
