#include "Store2.h"

namespace WPEFramework {
namespace Plugin {
    class GrpcStore2 : public Grpc::Store2 {};
    SERVICE_REGISTRATION(GrpcStore2, 1, 0);
} // namespace Plugin
} // namespace WPEFramework
