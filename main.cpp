#include "sha1.h"
#include "ws.h"
#include <cstdio>
#include <iostream>
#include <mutex>
#include <ostream>
#include <queue>
#include <regex>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <thread>
#include <uv.h>
using std::cout;
using std::endl;
using std::mutex;
using std::queue;
using std::regex;
using std::string;
using std::thread;

uv_loop_t *loop;
struct sockaddr_in addr;
uv_stream_t *Client;
struct Queue {
  void push(const string &s) {
    M.lock();
    message.push(s);
    M.unlock();
  }
  int front(string &s) {
    M.lock();
    int size = message.size();
    if (size) {
      s = message.front();
      M.unlock();
      return 0;
    } else {
      M.unlock();
      return 1;
    }
  }
  void pop() {
    M.lock();
    if (message.size()) {
      message.pop();
    }
    M.unlock();
  }
  mutex M;
  queue<string> message;
};
Queue message;

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

void write(uv_write_t *req, int status) {
  // printf("write\n");
  // printf("message: %s\n%zu", ((write_req_t *)req)->buf.base,
  //        ((write_req_t *)req)->buf.len);
  if (status) {
    fprintf(stderr, "Write error %s\n", uv_strerror(status));
  }
  free_write_req(req);
}

void read(uv_stream_t *client, ssize_t nread, const uv_buf_t *buf) {
  // printf("i=%d\n", *(int *)client->data);
  if (nread > 0) {
    // printf("(\n%s\n)\n", buf->base);
    message.push(buf->base);
    return;
  }
  if (nread < 0) {
    if (nread != UV_EOF)
      fprintf(stderr, "Read error %s\n", uv_err_name(nread));
    uv_close((uv_handle_t *)client, NULL);
  }

  free(buf->base);
}

void on_new_connection(uv_stream_t *server, int status) {
  printf("new connection\n%d=1\n", *(int *)server->data);
  if (status < 0) {
    fprintf(stderr, "New connection error %s\n", uv_strerror(status));
    // error!
    return;
  }

  uv_tcp_t *client = (uv_tcp_t *)malloc(sizeof(uv_tcp_t));
  client->data = server->data;
  Client = (uv_stream_t *)client;
  uv_tcp_init(loop, client);
  if (uv_accept(server, (uv_stream_t *)client) == 0) {
    uv_read_start((uv_stream_t *)client, alloc_buffer, read);
  } else {
    uv_close((uv_handle_t *)client, NULL);
  }
}
int write(const string &s) {
  write_req_t *req = (write_req_t *)malloc(sizeof(write_req_t));
  char *str = (char *)calloc(sizeof(char), s.length());
  s.copy(str, s.size());
  req->buf = uv_buf_init(str, s.length());
  // free(str);
  return uv_write((uv_write_t *)req, Client, &req->buf, 1, write);
}
int read(string &s) {
  string str;
  if (!message.front(str)) {
    message.pop();
    s = str;
    return 0;
  }
  return 1;
}
string waitread() {
  string s;
  for (; read(s);)
    ;
  return s;
}
int main() {
  loop = uv_default_loop();

  uv_tcp_t server;
  int i = 1;
  server.data = &i;
  uv_tcp_init(loop, &server);

  uv_ip4_addr("127.0.0.1", 8080, &addr);

  uv_tcp_bind(&server, (const struct sockaddr *)&addr, 0);
  int r = uv_listen((uv_stream_t *)&server, 16, on_new_connection);
  if (r) {
    fprintf(stderr, "Listen error %s\n", uv_strerror(r));
    return 1;
  }
  thread th(uv_run, loop, UV_RUN_DEFAULT);
  th.detach();
  // while (true)
  {
    string s = waitread();
    cout << "get : " << s << endl;
    const regex secwskey("Sec-WebSocket-Key: (.+)");
    std::smatch res;
    if (regex_search(s, res, secwskey)) {
      cout << "get key";
      cout << res[1] << endl;
    }
    string handshake;
    handshake.append("HTTP/1.1 101 Switching Protocols\r\n");
    handshake.append("Upgrade: websocket\r\n");
    handshake.append("Connection: Upgrade\r\n");
    handshake.append("Sec-WebSocket-Accept: ");
    handshake.append(get_Sec_ws_Accept(res[1]));
    handshake.append("\r\n\r\n");
    write(handshake);
    for (int i = 0;; ++i) {
      string s = waitread();
      cout << "=====READ=====" << endl
           << i
           << endl
           // << s << endl
           << "-----" << endl;
      // cout << to_vector(s) << endl << "==========" << endl;
      wsFrame frame = to_frame(s);
      cout << frame << endl << "===================" << endl;
      cout << "data: [" << frame.data << "]" << endl;
      if (frame.opcode == 8) {
        cout << endl << "=====DISCONNECT===" << endl;
        break;
      }
      {
        string data;
        std::cin >> data;
        wsFrame tmp = get_frame(data);
        cout << "write: [\n" << tmp << endl << "\n" << endl;
        write(from_frame(tmp));
      }
    }
  }
  uv_stop(loop);
  return 0;
}
