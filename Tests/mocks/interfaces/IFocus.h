#pragma once
#include "Module.h"

namespace WPEFramework {
namespace Exchange {

    struct EXTERNAL IFocus : virtual public Core::IUnknown {
        enum { ID = 0x000003F0 };

        // @brief focused state
        virtual uint32_t Focused(const bool focused) = 0;
    };

}
}
