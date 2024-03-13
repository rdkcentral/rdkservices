#include "StoreCache.h"

namespace WPEFramework {
namespace Plugin {
    class SqliteStoreCache : public Sqlite::StoreCache {};
    SERVICE_REGISTRATION(SqliteStoreCache, 1, 0);
} // namespace Plugin
} // namespace WPEFramework
