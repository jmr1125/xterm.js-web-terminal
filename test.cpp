#include <arpa/inet.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
int main() {
  char servaddr[] = {"localhost"};
  sockaddr_in sockaddr;
  memset(&sockaddr, 0, sizeof sockaddr);
  sockaddr.sin_family = AF_INET;
  sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  sockaddr.sin_port = htons(8001);

  int listenfd = socket(AF_INET, SOCK_STREAM, 0);
  if (-1 == bind(listenfd, (struct sockaddr *)&sockaddr, sizeof sockaddr)) {
    fprintf(stderr, "error on bind");
    return 1;
  }
  if (-1 == listen(listenfd, 1024)) {
    fprintf(stderr, "error on listen");
    return 1;
  }
  for (int i = 0; i < 50; ++i) {
    int connfd = accept(listenfd, NULL, NULL);
    if (connfd == -1) {
      fprintf(stderr, "error %s %d", strerror(errno), errno);
      break;
    }
    char buf[10];
    int n = recv(connfd, buf, 10, 0);
    buf[n] = 0;
    printf("msg: %s", buf);
    close(connfd);
  }
  close(listenfd);
}
