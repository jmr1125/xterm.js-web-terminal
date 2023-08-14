#include "shell_runner.h"
#include <csignal>
#include <cstddef>
#include <stdexcept>
#include <sys/fcntl.h>
#include <sys/wait.h>
#include <unistd.h>
#if __has_include(<util.h>)
#include <util.h>
#endif
#if __has_include(<pty.h>)
#include <pty.h>
#endif
using std::runtime_error;
pty start(string cmd, int x, int y) {
  winsize winp;
  winp.ws_col = y;
  winp.ws_row = x;
  pty res;
  int ret = forkpty(&res.x, NULL, NULL, NULL);
  // fcntl(res.x, F_SETFD, fcntl(res.x, F_GETFD) | O_NONBLOCK);
  if (ret == -1) {
    throw runtime_error("error openpty");
  }
  if (ret == 0) {
    close(res.x);
    // setsid(); //?
    execlp(cmd.c_str(), cmd.c_str(), NULL);
  }
  res.pid = ret;
  return res;
}
int kill(pty x, int sig) { return kill(x.pid, sig); }
int get_status(pty x) {
  int status;
  waitpid(x.pid, &status, WNOHANG | WUNTRACED);
  return status;
}
