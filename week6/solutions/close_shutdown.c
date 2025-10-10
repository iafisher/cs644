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
  struct addrinfo* addr = get_localhost();
  int sockfd = BAIL_IF_ERR(socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol));

  BAIL_IF_ERR(bind(sockfd, addr->ai_addr, addr->ai_addrlen));
  freeaddrinfo(addr);

  BAIL_IF_ERR(listen(sockfd, 128));

  int conn = BAIL_IF_ERR(accept(sockfd, NULL, NULL));

  char buf[4096];
  while (1) {
    ssize_t msglen = BAIL_IF_ERR(recv(conn, buf, sizeof buf - 1, 0));
    if (msglen == 0) {
      printf("server: EOF received\n");
      break;
    }
    printf("server: received: %ld byte(s)\n", msglen);

    sleep(1);

    ssize_t r = send(conn, buf, msglen, 0);
    if (r < 0) {
      perror("server: failed to send message to client");
    } else {
      printf("server: sent message to client\n");
    }
  }
}

void client() {
  int sleep_secs = 1;
  printf("client: waiting %ds before attempting to connect\n", sleep_secs);
  sleep(sleep_secs);

  struct addrinfo* addr = get_localhost();
  int sockfd = BAIL_IF_ERR(socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol));

  BAIL_IF_ERR(connect(sockfd, addr->ai_addr, addr->ai_addrlen));

  const char message[] = "hello world";
  BAIL_IF_ERR(send(sockfd, message, sizeof message, 0));
  printf("client: sent: %s\n", message);

  char buf[4096];
  ssize_t msglen = BAIL_IF_ERR(recv(sockfd, buf, sizeof buf - 1, 0));
  printf("client: received %ld byte(s)\n", msglen);

  int sockfd2 = BAIL_IF_ERR(dup(sockfd));

  BAIL_IF_ERR(shutdown(sockfd, SHUT_RDWR));
  printf("client: called shutdown on original sockfd\n");
  //BAIL_IF_ERR(close(sockfd));
  //printf("client: called close on original sockfd\n");

  BAIL_IF_ERR(send(sockfd2, message, sizeof message, 0));
  printf("client: sent on sockfd2: %s\n", message);
  msglen = BAIL_IF_ERR(recv(sockfd2, buf, sizeof buf - 1, 0));
  printf("client: received %ld byte(s) on sockfd2\n", msglen);

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
