#include "Store2.h"

namespace WPEFramework {
namespace Plugin {
    class SqliteStore2 : public Sqlite::Store2 {};
    SERVICE_REGISTRATION(SqliteStore2, 1, 0);
} // namespace Plugin
} // namespace WPEFramework
