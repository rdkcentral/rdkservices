#include "Wraps.h"

extern "C" int __wrap_system(const char* command)
{
    return Wraps::getInstance().system(command);
}
