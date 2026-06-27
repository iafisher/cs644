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
  hints.ai_family = AF_INET;      // IPv4
  hints.ai_socktype = SOCK_DGRAM; // UDP
  struct addrinfo* result;
  int status = getaddrinfo("localhost", PORT, &hints, &result);
  if (status != 0) {
    fprintf(stderr, "error: getaddrinfo failed: %s\n", gai_strerror(status));
    exit(1);
  }
  return result;
}

int client(const char* message) {
  struct addrinfo* addr = get_localhost();
  int sockfd = BAIL_IF_ERR(socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol));

  BAIL_IF_ERR(sendto(sockfd, message, strlen(message), 0, addr->ai_addr, addr->ai_addrlen));
  freeaddrinfo(addr);

  char buf[4096];
  ssize_t msglen = BAIL_IF_ERR(recv(sockfd, buf, sizeof buf - 1, 0));
  printf("%.*s\n", (int)msglen, buf);

  BAIL_IF_ERR(close(sockfd));
  return 0;
}

int server() {
  struct addrinfo* addr = get_localhost();
  int sockfd = BAIL_IF_ERR(socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol));

  BAIL_IF_ERR(bind(sockfd, addr->ai_addr, addr->ai_addrlen));
  freeaddrinfo(addr);

  printf("server: listening on port %s (UDP)\n", PORT);

  size_t buflen = 4096;
  char buf[buflen];
  for (;;) {
    struct sockaddr_storage src_addr;
    socklen_t addrlen;
    ssize_t msglen = BAIL_IF_ERR(recvfrom(sockfd, buf, buflen - 1, 0, (struct sockaddr*)&src_addr, &addrlen));

    char ipstr[INET_ADDRSTRLEN];
    printf("server: got message from %s (%ld byte(s))\n", inet_ntop(AF_INET, &((struct sockaddr_in*)&src_addr)->sin_addr, ipstr, sizeof ipstr), msglen);

    rot13(buf);
    BAIL_IF_ERR(sendto(sockfd, buf, msglen, 0, (struct sockaddr*)&src_addr, addrlen));
  }

  return 0;
}

void usage(char* progname) {
  fprintf(stderr, "usage:\n  %s client ARG\n  %s server\n", progname, progname);
}

int main(int argc, char* argv[]) {
  if (argc < 2) {
    usage(argv[0]);
    return 1;
  }

  if (strcmp(argv[1], "client") == 0 && argc == 3) {
    return client(argv[2]);
  } else if (strcmp(argv[1], "server") == 0 && argc == 2) {
    return server();
  } else {
    usage(argv[0]);
    return 1;
  }
}
