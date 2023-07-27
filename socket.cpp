#include "socket.h"
#include <atomic>
#include <cstdlib>
#include <map>
#include <mutex>
#include <queue>
#include <stdexcept>
#include <uv.h>
using std::atomic;
using std::runtime_error;
namespace Socket_READ {
mutex use_var;
string s;
void read(uv_stream_t *client, ssize_t nread, const uv_buf_t *buf) {
  if (nread > 0) {
    write_req_t *req = (write_req_t *)malloc(sizeof(write_req_t));
    req->buf = uv_buf_init(buf->base, nread);
    // printf("(\n%s\n)\n", buf->base);
    s = buf->base;
    return;
  }
  if (nread < 0) {
    if (nread != UV_EOF)
      fprintf(stderr, "Read error %s\n", uv_err_name(nread));
    uv_close((uv_handle_t *)client, NULL);
  }

  free(buf->base);
}
} // namespace Socket_READ
using std::map;
namespace Socket_CONNECT {
mutex use_var;
uv_loop_t *LOOP;
uv_stream_t **CLIENT;
map<uv_stream_t *, queue<string>> mp;
void on_new_connection(uv_stream_t *server, int status) {
  uv_loop_t *loop = LOOP;
  printf("new connection\n");
  if (status < 0) {
    char str[128];
    snprintf(str, 128, "New connection error %s\n", uv_strerror(status));
    // error!
    // return;
    throw runtime_error(str);
  }

  uv_tcp_t *client = (uv_tcp_t *)malloc(sizeof(uv_tcp_t));
  uv_tcp_init(loop, client);
  uv_stream_t **CLIENTT = CLIENT;
  *CLIENTT = (uv_stream_t *)client;
  if (uv_accept(server, (uv_stream_t *)client) == 0) {
    uv_read_start((uv_stream_t *)client, alloc_buffer,
                  [](uv_stream_t *client, ssize_t nread, const uv_buf_t *buf) {
                    Socket_READ::use_var.lock();
                    Socket_READ::read(client, nread, buf);
                    mp[client].push(Socket_READ::s);
                    Socket_READ::use_var.unlock();
                  });
  } else {
    uv_close((uv_handle_t *)client, NULL);
  }
}
}; // namespace Socket_CONNECT
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
void Socket::listen_accept() {
  Socket_CONNECT::use_var.lock();
  Socket_CONNECT::LOOP = loop;
  Socket_CONNECT::CLIENT = &client;
  try {
    int ret = uv_listen((uv_stream_t *)&server, 16,
                        Socket_CONNECT::on_new_connection);
  } catch (const std::runtime_error &e) {
    printf("run time error \n(%s)", e.what());
  }
  Socket_CONNECT::use_var.unlock();
  th = new thread{uv_run, loop, UV_RUN_DEFAULT};
  // uv_run(loop, UV_RUN_DEFAULT);
}

int Socket::write(const string &str) {
  write_req_t *req;
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
int Socket::read(string &s) {
  if (Socket_CONNECT::mp[client].size() == 0) {
    return 1;
  }
  s = Socket_CONNECT::mp[client].front();
  Socket_CONNECT::mp[client].pop();
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