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
  hints.ai_socktype = SOCK_STREAM; // TCP
  struct addrinfo* result;
  int status = getaddrinfo("localhost", PORT, &hints, &result);
  if (status != 0) {
    fprintf(stderr, "error: getaddrinfo failed: %s\n", gai_strerror(status));
    exit(1);
  }
  return result;
}

void server() {
  int sleep_secs = 5;
  printf("server: waiting %ds before calling bind\n", sleep_secs);
  sleep(sleep_secs);

  struct addrinfo* addr = get_localhost();
  int sockfd = BAIL_IF_ERR(socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol));

  BAIL_IF_ERR(bind(sockfd, addr->ai_addr, addr->ai_addrlen));
  printf("server: called bind\n");

  freeaddrinfo(addr);
}

void client() {
  struct addrinfo* addr = get_localhost();
  int sockfd = BAIL_IF_ERR(socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol));

  printf("client: calling connect on fd %d\n", sockfd);
  BAIL_IF_ERR(connect(sockfd, addr->ai_addr, addr->ai_addrlen));
  printf("client: connect succeeded\n");
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
