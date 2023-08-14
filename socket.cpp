#include "socket.h"
#include "uv.h"
#include <algorithm>
#include <cstddef>
#include <netinet/in.h>
#include <queue>
#include <stdexcept>

Sock_t pool[MAX_CLIENT];
bool pool_used[MAX_CLIENT];
static std::mutex mtx;
int new_id() {
  mtx.lock();
  int res;
  for (res = 0; pool_used[res]; ++res)
    ;
  if (res < MAX_CLIENT) {
    pool_used[res] = true;
    mtx.unlock();
    // printf("new %d", res);
    return res;
  }
  mtx.unlock();
  throw std::runtime_error("no more client");
}
void delete_id(int id) {
  static std::mutex id_mtx;
  id_mtx.lock();
  pool_used[id] = false;
  pool[id].client = pool[id].server = NULL;
  while (pool[id].messages.size() > 0) {
    pool[id].messages.pop();
  }
  // printf("delete %d\n", id);
  pool[id].status = client_status::DONE;
  id_mtx.unlock();
}
bool Pool_used(int i) { return ::pool_used[i]; }
Sock_t get_pool(int i) { return ::pool[i]; }
void alloc_buffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf) {
  buf->base = (char *)malloc(suggested_size);
  buf->len = suggested_size;
}
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
    client->data = new int(id);
    uv_tcp_init(((Socket *)server->data)->loop, client);
    pool[id].client = (uv_stream_t *)client;
    pool[id].server = (uv_stream_t *)&((Socket *)server->data)->server;
    pool[id].status = client_status::NEW;
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
      uv_close((uv_handle_t *)pool[id].client, // NULL
               [](uv_handle_t *handle) {
                 delete (int *)handle->data;
                 free(handle);
               });
      delete_id(id);
    }
  });
  th = new (thread)(uv_run, loop, UV_RUN_DEFAULT);
}
optional<message_t> Socket::read(int i) {
  int Id = i;
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
  int Id = i;
  static thread_local int err = 0;
  // write_req_t *req = (write_req_t *)malloc(sizeof(write_req_t));
  // char *str = (char *)calloc(sizeof(char), s.length());
  // s.copy(str, s.size());
  // req->buf = uv_buf_init(str, s.length());
  // uv_write((uv_write_t *)req, pool[Id].client, &req->buf, 1,
  //          [](uv_write_t *req, int status) {
  //            free_write_req(req);
  //            if (status) {
  //              err = status;
  //            }
  //          });
  // char *str = (char *)malloc(s.size());
  // s.copy(str, s.size());
  // uv_buf_t buf = uv_buf_init(str, s.size());
  uv_buf_t buf;
  size_t step = 2048;
  for (size_t off = 0; off < s.size(); off += step) {
    buf.base = s.data() + off;
    buf.len = std::min(s.size() - 1 - off, step);
    uv_write_t *req = (uv_write_t *)malloc(sizeof(uv_write_t));
    uv_write(req, pool[i].client, &buf, 1, [](uv_write_t *req, int status) {
      err = status;
      free(req);
    });
  }
  return err;
}
Socket::~Socket() {
  uv_stop(loop);
  uv_loop_close(loop);
  th->detach();
  delete th;
}
void Socket::close(int i) {
  // if (pool[id[i]].client->data) {
  //   // delete (int *)pool[id[i]].client->data;
  //   pool[id[i]].client->data = NULL;
  // }
  uv_close((uv_handle_t *)pool[i].client, [](uv_handle_t *handle) {
    delete (int *)handle->data;
    free(handle);
  });
  delete_id(i);
}
