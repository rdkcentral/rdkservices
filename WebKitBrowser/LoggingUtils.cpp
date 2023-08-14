/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
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
 */

#include <errno.h>
#include <fcntl.h>
#include <stddef.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <syslog.h>
#include <signal.h>
#include <glib.h>

#include "Module.h"

namespace WPEFramework {
namespace Plugin {

bool RedirectAllLogsToService(const string& target_service)
{
  if (target_service.empty()) {
    fprintf(stderr, "RedirectLog: Invalid argument. Logs target service expected\n");
    return false;
  }
  const string kServiceExt = ".service";
  if (target_service.size() <= kServiceExt.size() ||
      target_service.substr(target_service.size() - kServiceExt.size(), kServiceExt.size()) != kServiceExt) {
    fprintf(stderr, "RedirectLog: Invalid serivce name: *.service format expected\n");
    return false;
  }

  const string targetServiceName = target_service.substr(0, target_service.size() - kServiceExt.size());
  const string kSystemdCgroupTargetTasksFilePath = "/sys/fs/cgroup/systemd/system.slice/" + target_service + "/tasks";
  if (!g_file_test(kSystemdCgroupTargetTasksFilePath.c_str(), G_FILE_TEST_EXISTS)) {
    fprintf(stderr, "RedirectLog: %s unit cgroup doesn't exist\n", target_service.c_str());
    return false;
  }

  // Add all threads of current process to target systemd cgroup
  WPEFramework::Core::Directory taskDir("/proc/self/task");
  while (taskDir.Next() == true) {
    if (taskDir.Name() == "." || taskDir.Name() == "..")
      continue;
    FILE* f = fopen(kSystemdCgroupTargetTasksFilePath.c_str(), "a");
    if (f) {
      fprintf(f, "%s", taskDir.Name().c_str());
      fclose(f);
    } else {
      fprintf(stderr, "RedirectLog: cannot move %s thread to '%s': %s\n",
              taskDir.Name().c_str(), kSystemdCgroupTargetTasksFilePath.c_str(), strerror(errno));
    }
  }

  // Redirect stdout / stderr
  sockaddr_un sa;
  memset (&sa, 0, sizeof (sa));
  sa.sun_family = AF_UNIX;
  g_strlcpy (sa.sun_path, "/run/systemd/journal/stdout", sizeof(sa.sun_path));

  int fd = -1;
  fd = socket(AF_UNIX, SOCK_STREAM, 0);
  if (fd < 0) {
    perror("RedirectLog: socket() failed");
    return false;
  }

  int r = connect(fd, (const sockaddr*)&sa, sizeof(sa));
  if (r < 0) {
    perror("RedirectLog: couldn't connect");
    close(fd);
    return false;
  }

  shutdown(fd, SHUT_RD);

  int value = (8*1024*1024);
  setsockopt(fd, SOL_SOCKET, SO_SNDBUF, &value, sizeof(value));

  char* header = g_strdup_printf("%s\n%s\n%i\n%i\n0\n0\n0\n", targetServiceName.c_str(), target_service.c_str(), (LOG_DAEMON|LOG_INFO), 1);
  char *p = header;
  size_t nbytes = strlen(header);

  do {
    ssize_t k;
    k = write(fd, p, nbytes);
    if (k < 0) {
      if (errno == EINTR)
        continue;
      perror("RedirectLog: write() failed");
      break;
    }
    p += k;
    nbytes -= k;
  }
  while (nbytes > 0);
  g_free(header);

  if (nbytes != 0) {
    perror("RedirectLog: write() not completed");
    close(fd);
    return false;
  }

#ifdef SIGPIPE
  signal (SIGPIPE, SIG_IGN);
#endif

  dup3(fd, STDOUT_FILENO, 0);
  dup3(fd, STDERR_FILENO, 0);

  close(fd);
  return true;
}

}
}
