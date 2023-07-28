#include <cstddef>
#include <mutex>
#include <netinet/in.h>
#include <queue>
#include <stdexcept>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <thread>
#include <uv.h>
using std::mutex;
using std::queue;
using std::string;
using std::thread;
struct write_req_t;
struct Socket {
  Socket() = delete;
  Socket(string addr, int port);
  void listen_accept(uv_connection_cb);
  int read(string &);
  int write(const string &);
  void stop();
  ~Socket();
  uv_loop_t *loop;

  uv_stream_t *client;

  struct sockaddr_in addr;
  uv_tcp_t server;
  thread *th;
};

struct write_req_t {
  uv_write_t req;
  uv_buf_t buf;
};

void free_write_req(uv_write_t *req);
void alloc_buffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf);
