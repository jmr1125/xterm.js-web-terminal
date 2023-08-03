#include <optional>
#include <string>
#include <thread>
#include <utility>
#include <uv.h>
#define MAX_CLIENT 16
typedef std::pair<ssize_t, std::string> message_t;
using std::optional;
using std::pair;
using std::string;
using std::thread;
using std::vector;
struct Socket {
  Socket(string, int);
  ~Socket();
  void listen();
  optional<message_t> read(int);
  message_t waitread(int);
  int write(string s, int);
  void close(int);
  uv_loop_t *loop;
  uv_tcp_t server;
  thread *th;
  vector<int> id;
};
