
#include <cstdio>
#include "Module.h"
#include "thunder_test_base.h"

class ThunderTest : public ThunderTestBase
{
};

/**
 * @brief Start and stop the Thunder process
 */
TEST_F(ThunderTest, SetupThunder)
{
}

/**
 * @brief Test duplicate activation of the plugin
 *
 * Note that the plugin is first activated in ThunderTestBase::SetUp.
 */
TEST_F(ThunderTest, DuplicateActivate)
{
    uint32_t status;

    status = Activate("org.rdk.System");
    EXPECT_EQ(0U, status);
}

/**
 * @brief Test duplicate deactivation of plugin
 *
 * Note that the plugin is also deactivated in ThunderTestBase::TearDown.
 */
TEST_F(ThunderTest, DuplicateDeactivate)
{
    uint32_t status;

    status = Deactivate("org.rdk.System");
    EXPECT_EQ(0U, status);
}

/**
 * @brief Test SystemService hasRebootBeenRequested methods
 *
*/
TEST_F(ThunderTest, SystemServicehasRebootBeenRequested)
{
    JsonObject params;
    JsonObject result;
    uint32_t status;

    status = InvokeMethod("org.rdk.System.1", "hasRebootBeenRequested", params, result);
    EXPECT_EQ(0U, status);
}
/**
 * @brief Test SystemService set and get temperature methods
 *
*/
TEST_F(ThunderTest, SystemServiceSetGetTemperature)
{
    JsonObject params;
    JsonObject result;
    uint32_t status;

    status = InvokeMethod("org.rdk.System.1", "getCoreTemperature", params, result);
    EXPECT_EQ(0U, status);
}




