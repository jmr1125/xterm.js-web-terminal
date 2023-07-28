#include "socket.h"
#include <atomic>
#include <cstdlib>
#include <map>
#include <mutex>
#include <queue>
#include <stdexcept>
#include <uv.h>
using std::atomic;
using std::map;
using std::runtime_error;
Socket::Socket(string address, int port) {
  loop = uv_default_loop();
  uv_tcp_init(loop, &server);
  uv_ip4_addr(address.c_str(), port, &addr);
  uv_tcp_bind(&server, (const sockaddr *)&addr, 0);
}
void Socket::stop() {
  uv_read_stop(client);
  uv_stop(loop);
  th->detach();
  delete th;
}
Socket::~Socket() { stop(); }
void Socket::listen_accept(uv_connection_cb func) {
  int ret = uv_listen((uv_stream_t *)&server, 16, func);
  th = new thread{uv_run, loop, UV_RUN_DEFAULT};
  // uv_run(loop, UV_RUN_DEFAULT);
}

int Socket::write(const string &str) {
  write_req_t *req = (write_req_t *)std::malloc(sizeof(write_req_t));
  char *s = (char *)std::calloc(sizeof(char), str.size());
  for (int i = 0; i < str.size(); ++i) {
    s[i] = str[i];
  }
  req->buf = uv_buf_init(s, str.size());
  uv_write((uv_write_t *)req, client, &req->buf, 1,
           [](uv_write_t *req, int status) {
             free_write_req(req);
             if (status) {
               throw runtime_error(
                   ((string) "Write error " + uv_strerror(status)).c_str());
             }
           });
  return 0;
}

void free_write_req(uv_write_t *req) {
  write_req_t *wr = (write_req_t *)req;
  free(wr->buf.base);
  free(wr);
}

void alloc_buffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf) {
  buf->base = (char *)malloc(suggested_size);
  buf->len = suggested_size;
}
