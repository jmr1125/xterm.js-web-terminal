#include "socket.h"
#include "uv.h"
#include <cstddef>
#include <netinet/in.h>
#include <queue>
#include <stdexcept>
typedef struct {
  uv_write_t req;
  uv_buf_t buf;
} write_req_t;

void free_write_req(uv_write_t *req) {
  write_req_t *wr = (write_req_t *)req;
  free(wr->buf.base);
  free(wr);
}

void alloc_buffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf) {
  buf->base = (char *)malloc(suggested_size);
  buf->len = suggested_size;
}

struct Sock_t {
  uv_stream_t *client;
  uv_stream_t *server;
  std::queue<message_t> messages;
};
Sock_t pool[MAX_CLIENT];
bool pool_used[MAX_CLIENT];
int new_id() {
  int res;
  for (res = 0; pool_used[res]; ++res)
    ;
  if (res < MAX_CLIENT) {
    pool_used[res] = true;
    return res;
  }
  throw std::runtime_error("no more client");
}
void delete_id(int id) { pool_used[id] = false; }
Socket::Socket(string Addr, int port) {
  loop = uv_default_loop();
  server.data = this;
  uv_tcp_init(loop, &server);
  struct sockaddr_in addr;
  uv_ip4_addr(Addr.c_str(), port, &addr);
  uv_tcp_bind(&server, (const struct sockaddr *)&addr, 0);
}
void Socket::listen() {
  uv_listen((uv_stream_t *)&server, 16, [](uv_stream_t *server, int status) {
    // printf("new connection\n");
    uv_tcp_t *client = (uv_tcp_t *)malloc(sizeof(uv_tcp_t));
    int id = new_id();
    ((Socket *)server->data)->id.push_back(id);
    client->data = new int(id);
    uv_tcp_init(((Socket *)server->data)->loop, client);
    pool[id].client = (uv_stream_t *)client;
    pool[id].server = (uv_stream_t *)&((Socket *)server->data)->server;
    if (uv_accept(pool[id].server, pool[id].client) == 0) {
      uv_read_start(
          pool[id].client, alloc_buffer,
          [](uv_stream_t *client, ssize_t nread, const uv_buf_t *buf) {
            // printf("read: %s %zd\n", buf->base, nread);
            pool[*(int *)(client->data)].messages.push(
                std::make_pair(nread, buf->base));
            if (nread <= 0) {
              free(buf->base);
            }
          });
    } else {
      uv_close((uv_handle_t *)pool[id].client, NULL);
    }
  });
  th = new (thread)(uv_run, loop, UV_RUN_DEFAULT);
}
optional<message_t> Socket::read(int i) {
  int Id = id[i];
  if (pool[Id].messages.size()) {
    message_t tmp = pool[Id].messages.front();
    pool[Id].messages.pop();
    return make_optional(tmp);
  } else {
    return {};
  }
}
message_t Socket::waitread(int i) {
  optional<message_t> tmp;
  while (!(tmp = read(i)).has_value())
    ;
  return tmp.value();
}
int Socket::write(string s, int i) {
  int Id = id[i];
  static thread_local int err = 0;
  write_req_t *req = (write_req_t *)malloc(sizeof(write_req_t));
  char *str = (char *)calloc(sizeof(char), s.length());
  s.copy(str, s.size());
  req->buf = uv_buf_init(str, s.length());
  uv_write((uv_write_t *)req, pool[Id].client, &req->buf, 1,
           [](uv_write_t *req, int status) {
             free_write_req(req);
             if (status) {
               err = status;
             }
           });
  return err;
}
Socket::~Socket() {
  for (int i = 0; i < id.size(); ++i) {
    close(i);
  }
  uv_stop(loop);
  uv_loop_close(loop);
  th->detach();
  delete th;
}
void Socket::close(int i) {
  delete (int *)pool[id[i]].client->data;
  uv_close((uv_handle_t *)pool[id[i]].client, NULL);
  delete_id(id[i]);
  id.erase(id.begin() + i);
}
