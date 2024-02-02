#pragma once

#include <gmock/gmock.h>
#include <interfaces/IStoreCache.h>

class StoreCacheMock : public WPEFramework::Exchange::IStoreCache {
public:
    ~StoreCacheMock() override = default;
    MOCK_METHOD(uint32_t, FlushCache, (), (override));
    BEGIN_INTERFACE_MAP(StoreCacheMock)
    INTERFACE_ENTRY(IStoreCache)
    END_INTERFACE_MAP
};
