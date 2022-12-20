#include "rdkshell.h"

#define RDKSHELL_FPS 40
int gCurrentFramerate = RDKSHELL_FPS;
std::shared_ptr<RdkShell::RdkShellEventListener> RdkShell::gRdkShellEventListener = nullptr;
