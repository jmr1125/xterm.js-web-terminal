#include "socket.h"
#include "uv.h"
#include "ws.h"
uv_loop_t *loop;
void echo_read(uv_stream_t *client, ssize_t nread, const uv_buf_t *buf) {
  if (nread > 0) {
    printf("(\n%s\n)\n", buf->base);
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
  printf("new connection\n");
  if (status < 0) {
    fprintf(stderr, "New connection error %s\n", uv_strerror(status));
    // error!
    return;
  }

  uv_tcp_t *client = (uv_tcp_t *)malloc(sizeof(uv_tcp_t));
  uv_tcp_init(loop, client);
  if (uv_accept(server, (uv_stream_t *)client) == 0) {
    uv_read_start((uv_stream_t *)client, alloc_buffer, echo_read);
  } else {
    uv_close((uv_handle_t *)client, NULL);
  }
}

int main() {
  Socket sock("127.0.0.1", 8080);
  loop = sock.loop;
  sock.listen_accept(on_new_connection);
  sock.th->join();
}
