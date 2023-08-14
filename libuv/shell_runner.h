#include <string>
using std::string;
struct pty {
  int x;
  pid_t pid;
};
pty start(string, int x = 24, int y = 80);
int kill(pty, int);
int get_status(pty);
