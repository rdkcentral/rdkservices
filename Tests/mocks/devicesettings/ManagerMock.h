#pragma once

#include <gmock/gmock.h>

#include "manager.hpp"

class ManagerImplMock : public ManagerImpl {
public:
    virtual ~ManagerImplMock() = default;

    MOCK_METHOD(void, Initialize, (), (override));
    MOCK_METHOD(void, DeInitialize, (), (override));
};
