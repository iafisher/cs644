/**
 * https://iafisher.com/cs644/fall2025/week6
 */
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include "cs644.h"

const char* PORT = "3456";

struct addrinfo* get_localhost() {
  struct addrinfo hints;
  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_INET;       // IPv4
  hints.ai_socktype = SOCK_DGRAM; // UDP
  struct addrinfo* result;
  int status = getaddrinfo("localhost", PORT, &hints, &result);
  if (status != 0) {
    fprintf(stderr, "error: getaddrinfo failed: %s\n", gai_strerror(status));
    exit(1);
  }
  return result;
}

void server() {
  struct addrinfo* addr = get_localhost();
  int sockfd = BAIL_IF_ERR(socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol));

  BAIL_IF_ERR(bind(sockfd, addr->ai_addr, addr->ai_addrlen));
  freeaddrinfo(addr);

  int sleep_secs = 2;
  printf("server: waiting %ds before receiving packets\n", sleep_secs);
  sleep(sleep_secs);

  char buf[4];
  while (1) {
    ssize_t msglen = BAIL_IF_ERR(recv(sockfd, buf, sizeof buf - 1, 0));
    printf("server: received: %.*s\n", (int)msglen, buf);
  }
}

void client() {
  int sleep_secs = 1;
  printf("client: waiting %ds before attempting to connect\n", sleep_secs);
  sleep(sleep_secs);

  struct addrinfo* addr = get_localhost();
  int sockfd = BAIL_IF_ERR(socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol));

  char buf[2];
  const char message[] = "hello world";

  for (const char* p = message; *p != '\0'; p++) {
    buf[0] = *p;
    buf[1] = '\0';
    BAIL_IF_ERR(sendto(sockfd, buf, sizeof buf, 0, addr->ai_addr, addr->ai_addrlen));
    printf("client: sent: %s\n", buf);
  }

  // this message is bigger than the server's buffer
  BAIL_IF_ERR(sendto(sockfd, message, sizeof message, 0, addr->ai_addr, addr->ai_addrlen));
  printf("client: sent: %s\n", message);

  freeaddrinfo(addr);
}

int main() {
  pid_t pid = BAIL_IF_ERR(fork());
  if (pid == 0) {
    // child
    client();
  } else {
    // parent
    server();
  }
}
