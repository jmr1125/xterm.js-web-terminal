#include <uv.h>

uv_loop_t* loop;
uv_tcp_t socket;

void alloc_buffer(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf) {
    buf->base = new char[suggested_size];
    buf->len = suggested_size;
}

void on_connect(uv_connect_t* req, int status) {
    if (status < 0) {
        fprintf(stderr, "Connect error: %s\n", uv_strerror(status));
        return;
    }

    fprintf(stdout, "Connected to server\n");

    uv_buf_t wrbuf = uv_buf_init("Hello server!", strlen("Hello server!"));
    uv_write_t write_req;
    uv_write(&write_req, (uv_stream_t*) &socket, &wrbuf, 1, [](uv_write_t* req, int status) {
        if (status < 0) {
            fprintf(stderr, "Write error: %s\n", uv_strerror(status));
        }
        delete[] req->data;
        delete req;
    });

    uv_read_start((uv_stream_t*) &socket, alloc_buffer,
                 [](uv_stream_t* handle, ssize_t nread, const uv_buf_t* buf) {
                     if (nread > 0) {
                         fprintf(stdout, "Received from server: %.*s\n", nread, buf->base);
                     } else if (nread < 0) {
                         if (nread != UV_EOF) {
                             fprintf(stderr, "Read error: %s\n", uv_err_name(nread));
                         }
                         uv_close((uv_handle_t*) handle, nullptr);
                     }
                     delete[] buf->base;
                 });
}

int main() {
    loop = uv_default_loop();

    uv_tcp_init(loop, &socket);
    struct sockaddr_in dest;

    uv_ip4_addr("127.0.0.1", 8000, &dest);

    uv_connect_t connect_req;
    uv_tcp_connect(&connect_req, &socket, (const struct sockaddr*) &dest, on_connect);

    return uv_run(loop, UV_RUN_DEFAULT);
}
