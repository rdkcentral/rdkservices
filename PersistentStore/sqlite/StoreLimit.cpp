#include "StoreLimit.h"

namespace WPEFramework {
namespace Plugin {
    class SqliteStoreLimit : public Sqlite::StoreLimit {};
    SERVICE_REGISTRATION(SqliteStoreLimit, 1, 0);
} // namespace Plugin
} // namespace WPEFramework
