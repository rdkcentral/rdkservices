#include <string>
#include <vector>
#include <map>
#include <memory>

#pragma once

namespace WPEFramework
{
    namespace Plugin
    {
        struct ILocalStore
        {
            virtual ~ILocalStore() = default;
            virtual bool Open(const std::string &path) = 0;
            virtual bool CreateTable(const std::string &table) = 0;
            virtual bool SetLimit(const std::string &table, uint32_t limit) = 0;
            virtual std::pair<uint32_t, uint32_t> GetEntriesCount(const std::string &table, uint32_t start, uint32_t maxCount) const = 0;
            virtual std::vector<std::string> GetEntries(const std::string &table, uint32_t start, uint32_t count) const = 0;
            virtual bool RemoveEntries(const std::string &table, uint32_t start, uint32_t end) = 0;
            virtual bool AddEntry(const std::string &table, const std::string &entry) = 0;
        };

        using ILocalStorePtr = std::shared_ptr<ILocalStore>;

    }
}