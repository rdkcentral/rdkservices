#include "StoreInspector.h"

namespace WPEFramework {
namespace Plugin {
    class SqliteStoreInspector : public Sqlite::StoreInspector {};
    SERVICE_REGISTRATION(SqliteStoreInspector, 1, 0);
} // namespace Plugin
} // namespace WPEFramework
