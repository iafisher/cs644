#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

const unsigned int PORT = 2345;

struct sockaddr_in get_localhost() {
  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_port = htons(PORT);
  addr.sin_addr.s_addr = inet_addr("127.0.0.1");
  memset(addr.sin_zero, 0, sizeof addr.sin_zero);
  return addr;
}

int client() {
  int sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0) {
    perror("socket");
    return 1;
  }

  struct sockaddr_in addr = get_localhost();
  int r = connect(sockfd, (struct sockaddr*)&addr, sizeof addr);
  if (r < 0) {
    perror("connect");
    return 1;
  }

  for (;;) {
    char buf[128];
    ssize_t nread = recv(sockfd, buf, sizeof buf, 0);
    if (nread < 0) {
      perror("recv");
      return 1;
    } else if (nread == 0) {
      puts("connection closed");
      break;
    } else {
      printf("read: %.*s\n", (int)nread, buf);
    }
  }

  close(sockfd);
  return 0;
}

int server() {
  int sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0) {
    perror("socket");
    return 1;
  }

  struct sockaddr_in addr = get_localhost();
  int r = bind(sockfd, (struct sockaddr*)&addr, sizeof addr);
  if (r < 0) {
    perror("bind");
    return 1;
  }

  r = listen(sockfd, 128);
  if (r < 0) {
    perror("listen");
    return 1;
  }
  printf("listening on port %d\n", PORT);

  for (;;) {
    struct sockaddr addr;
    socklen_t addrlen;
    int conn = accept(sockfd, &addr, &addrlen);
    if (conn < 0) {
      perror("accept");
      return 1;
    }

    puts("got connection");
    const char message[] = "hello\n";
    write(conn, message, sizeof message);
    close(conn);
  }

  return 0;
}

int main(int argc, char* argv[]) {
  if (argc != 2) {
    return 1;
  }

  if (strcmp(argv[1], "client") == 0) {
    return client();
  } else if (strcmp(argv[1], "server") == 0) {
    return server();
  } else {
    return 1;
  }
}
