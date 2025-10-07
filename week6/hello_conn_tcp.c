/**
 * https://iafisher.com/cs644/fall2025/week6
 */
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include "cs644.h"

const char* PORT = "2345";

struct addrinfo* get_localhost() {
  struct addrinfo hints;
  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_INET;       // IPv4
  hints.ai_socktype = SOCK_STREAM; // TCP
  struct addrinfo* result;
  int status = getaddrinfo("localhost", PORT, &hints, &result);
  if (status != 0) {
    fprintf(stderr, "error: getaddrinfo failed: %s\n", gai_strerror(status));
    exit(1);
  }
  return result;
}

int client() {
  struct addrinfo* addr = get_localhost();
  int sockfd = BAIL_IF_ERR(socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol));

  BAIL_IF_ERR(connect(sockfd, addr->ai_addr, addr->ai_addrlen));
  freeaddrinfo(addr);

  for (;;) {
    char buf[128];
    ssize_t nread = BAIL_IF_ERR(recv(sockfd, buf, sizeof buf, 0));
    if (nread == 0) {
      puts("connection closed");
      break;
    } else {
      printf("read: %.*s\n", (int)nread, buf);
    }
  }

  BAIL_IF_ERR(close(sockfd));
  return 0;
}

int server() {
  struct addrinfo* addr = get_localhost();
  int sockfd = BAIL_IF_ERR(socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol));

  BAIL_IF_ERR(bind(sockfd, addr->ai_addr, addr->ai_addrlen));
  freeaddrinfo(addr);

  BAIL_IF_ERR(listen(sockfd, 128));
  printf("listening on port %s\n", PORT);

  for (;;) {
    struct sockaddr addr;
    socklen_t addrlen;
    int conn = BAIL_IF_ERR(accept(sockfd, &addr, &addrlen));

    puts("got connection");
    const char message[] = "hello\n";
    BAIL_IF_ERR(write(conn, message, sizeof message));
    BAIL_IF_ERR(close(conn));
  }

  return 0;
}

void usage(char* progname) {
  fprintf(stderr, "usage: %s {client|server}\n", progname);
}

int main(int argc, char* argv[]) {
  if (argc != 2) {
    usage(argv[0]);
    return 1;
  }

  if (strcmp(argv[1], "client") == 0) {
    return client();
  } else if (strcmp(argv[1], "server") == 0) {
    return server();
  } else {
    usage(argv[0]);
    return 1;
  }
}
