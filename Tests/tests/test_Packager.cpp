
/**
 * If not stated otherwise in this file or this component's LICENSE
 * file the following copyright and licenses apply:
 *
 * Copyright 2022 RDK Management
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 **/

#include "gtest/gtest.h"
#include "FactoriesImplementation.h"
#include "Packager.h"
#include "PackagerImplementation.h"
#include "ServiceMock.h"
#include "COMLinkMock.h"
#include "IarmBusMock.h"


using namespace WPEFramework;

using ::testing::NiceMock;

namespace {
const string config = _T("Packager");
const string callSign = _T("Packager");
const string webPrefix = _T("/Service/Packager");
const string volatilePath = _T("/tmp/");
}

class PackagerTest : public ::testing::Test {
protected:
    NiceMock<IarmBusImplMock> iarmBusImplMock;
    Core::ProxyType<Plugin::PackagerImplementation> PackagerImplementation;
    Exchange::IPackager* interface;

    PackagerTest()
    {
        IarmBus::getInstance().impl = &iarmBusImplMock;

        PackagerImplementation = Core::ProxyType<Plugin::PackagerImplementation>::Create();

        interface = static_cast<Exchange::IPackager*>(
            PackagerImplementation->QueryInterface(Exchange::IPackager::ID));
    }
    virtual ~PackagerTest()
    {
        IarmBus::getInstance().impl = nullptr;
    }
};


TEST_F(PackagerTest, Make)
    {
	NiceMock<ServiceMock> service;
	
	EXPECT_CALL(service, ConfigLine())
        .Times(1)
        .WillOnce(::testing::Return(config));
        ON_CALL(service, WebPrefix())
            .WillByDefault(::testing::Return(webPrefix));
        ON_CALL(service, VolatilePath())
            .WillByDefault(::testing::Return(volatilePath));
	ON_CALL(service, DataPath())
            .WillByDefault(::testing::Return(_T("/tmp/")));
        ON_CALL(service, Callsign())
            .WillByDefault(::testing::Return(callSign));


	EXPECT_EQ(Core::ERROR_GENERAL, interface->Configure(&service));


	/*EXPECT_EQ(0, interface->Install("Test", "1.0", "arm"));
	std::this_thread::sleep_for(std::chrono::seconds(30));*/

}
