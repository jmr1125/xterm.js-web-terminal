#include <uv.h>

uv_loop_t *loop;

void on_new_connection(uv_stream_t *server, int status);
void on_read(uv_stream_t *client, ssize_t nread, const uv_buf_t *buf);

void alloc_buffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf) {
  buf->base = new char[suggested_size];
  buf->len = suggested_size;
}

void echo_write(uv_write_t *req, int status) {
  if (status) {
    fprintf(stderr, "Write error: %s\n", uv_strerror(status));
  }
  delete[] (char *)req->data;
  delete req;
}

void on_close(uv_handle_t *handle) { delete handle; }

void on_new_connection(uv_stream_t *server, int status) {
  if (status < 0) {
    fprintf(stderr, "New connection error: %s\n", uv_strerror(status));
    return;
  }

  uv_tcp_t *client = new uv_tcp_t;
  uv_tcp_init(loop, client);

  if (uv_accept(server, (uv_stream_t *)client) == 0) {
    uv_read_start((uv_stream_t *)client, alloc_buffer, on_read);
  } else {
    uv_close((uv_handle_t *)client, on_close);
    delete client;
  }
}

void on_read(uv_stream_t *client, ssize_t nread, const uv_buf_t *buf) {
  if (nread > 0) {
    uv_write_t *req = new uv_write_t;
    uv_buf_t wrbuf = uv_buf_init(buf->base, nread);
    req->data = (void *)buf->base;
    uv_write(req, client, &wrbuf, 1, echo_write);
    return;
  }

  if (nread < 0) {
    if (nread != UV_EOF) {
      fprintf(stderr, "Read error: %s\n", uv_err_name(nread));
    }
    uv_close((uv_handle_t *)client, on_close);
  }

  delete[] buf->base;
}

int main() {
  loop = uv_default_loop();

  uv_tcp_t server;
  uv_tcp_init(loop, &server);

  struct sockaddr_in addr;
  uv_ip4_addr("0.0.0.0", 8000, &addr);

  uv_tcp_bind(&server, (const struct sockaddr *)&addr, 0);

  int r = uv_listen((uv_stream_t *)&server, 128, on_new_connection);
  if (r) {
    fprintf(stderr, "Listen error: %s\n", uv_strerror(r));
    return 1;
  }

  return uv_run(loop, UV_RUN_DEFAULT);
}
