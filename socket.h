#include <assert.h>
#include <optional>
#include <queue>
#include <string>
#include <thread>
#include <utility>
#include <uv.h>
#define MAX_CLIENT 16
typedef std::pair<ssize_t, std::string> message_t;
using std::optional;
using std::queue;
using std::string;
using std::thread;
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
};

namespace client_status {
enum status { NON, NEW, DONE, RUNNING };
}
struct Sock_t {
  uv_stream_t *client;
  uv_stream_t *server;
  client_status::status status = client_status::NON;
  std::queue<message_t> messages;
};
bool Pool_used(int);
Sock_t &get_pool(int);
