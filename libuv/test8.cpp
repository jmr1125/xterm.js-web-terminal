#include "shell_runner.h"
#include <cctype>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <unistd.h>
using std::cout;
using std::endl;
int main() {
  pty x = start("zsh");
  char buf[256]{0};
  fd_set SET;
  while (true) {
    memset(buf, 0, sizeof buf);
    FD_ZERO(&SET);
    FD_SET(STDIN_FILENO, &SET);
    FD_SET(x.x, &SET);
    timeval timeout;
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;
    int num = select(x.x + 1, &SET, NULL, NULL, &timeout);
    // cout << "status: " << get_status(x) << endl << "num: " << num << endl;
    if (num < 0) {
      cout << "error select" << endl;
    }
    if (num > 0) {
      if (FD_ISSET(x.x, &SET)) {
        int ret = read(x.x, buf, sizeof buf);
        if (ret > 0) {
          // cout << "data: len= " << ret << endl;
          // cout << "data: [" << endl;
          for (int i = 0; i < sizeof buf; ++i) {
            if (std::isprint(buf[i])) {
              cout << buf[i];
            } else {
              // cout << "(" << (int)buf[i] << ")";
              cout << buf[i];
            }
          }
        } else {
          cout << "ret = " << ret;
          if (ret < 0) {
            cout << "(" << std::strerror(ret) << ")";
          } else {
            int ret = kill(x, 0);
            if (ret == ESRCH) {
              cout << "progress NOT exist" << endl;
              break;
            }
          }
          // cout << "\n]" << endl;
        }
      }
      if (FD_ISSET(STDIN_FILENO, &SET)) {
        int ret = read(STDIN_FILENO, buf, sizeof buf);
        if (ret < 0) {
          cout << "error" << endl;
          continue;
        }
        ret = write(x.x, buf, strlen(buf));
        // cout << "write: ret= " << ret << endl;
        if (strncmp(buf, "exit", 4) == 0) {
          break;
        }
      }
    }
  }
  close(x.x);
  cout << "return: " << kill(x, SIGKILL) << endl;
  cout << "status: " << get_status(x) << endl;
}
