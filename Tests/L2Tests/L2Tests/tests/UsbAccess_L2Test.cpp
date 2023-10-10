#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "L2Tests.h"
#include "L2TestsMock.h"

using ::testing::NiceMock;

class UsbAccess_L2test : public L2TestMocks {
};
TEST_F(UsbAccess_L2test, test1)
{
} 
