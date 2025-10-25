/**
 * https://iafisher.com/cs644/fall2025/week9
 */

#define _GNU_SOURCE // for accept4
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include "cs644.h"

#define PORT "4545"

int self_pipe_write_end = -1;

struct addrinfo* get_localhost();

void signal_handler(int signo) {
  char buf[1];
  buf[0] = signo;
  write(self_pipe_write_end, buf, 1);
}

int main() {
  printf("server: PID %d\n", getpid());

  struct addrinfo* addr = get_localhost();
  int sockfd = BAIL_IF_ERR(socket(addr->ai_family, addr->ai_socktype | O_NONBLOCK, addr->ai_protocol));

  BAIL_IF_ERR(bind(sockfd, addr->ai_addr, addr->ai_addrlen));
  freeaddrinfo(addr);

  BAIL_IF_ERR(listen(sockfd, 128));
  printf("server: listening on port %s\n", PORT);

  int epollfd = BAIL_IF_ERR(epoll_create(1 /* kernel checks that it is positive but otherwise ignores it */));

  struct epoll_event listen_event = { .events = EPOLLIN, .data = { .fd = sockfd } };
  BAIL_IF_ERR(epoll_ctl(epollfd, EPOLL_CTL_ADD, sockfd, &listen_event));

  int pipefd[2];
  BAIL_IF_ERR(pipe2(pipefd, O_NONBLOCK));
  int self_pipe_read_end = pipefd[0];
  self_pipe_write_end = pipefd[1];

  struct epoll_event self_pipe_event = { .events = EPOLLIN, .data = { .fd = self_pipe_read_end } };
  BAIL_IF_ERR(epoll_ctl(epollfd, EPOLL_CTL_ADD, self_pipe_read_end, &self_pipe_event));

  struct sigaction act;
  act.sa_handler = signal_handler;
  sigemptyset(&act.sa_mask);
  act.sa_flags = 0;
  BAIL_IF_ERR(sigaction(SIGTERM, &act, NULL));

  unsigned int max_events = 256;
  struct epoll_event events[max_events];
  char read_buffer[4096];
  while (1) {
    int count = BAIL_IF_ERR_EXCEPT(epoll_wait(epollfd, events, max_events, -1), EINTR);
    // slightly tricky: if EINTR is encountered, then count will be -1 and the for loop will not execute
    for (int i = 0; i < count; i++) {
      struct epoll_event event = events[i];
      int fd = event.data.fd;
      if (fd == sockfd) {
        // handle incoming connection
        while (1) {
          int connfd = BAIL_IF_ERR_EXCEPT2(accept4(sockfd, NULL, NULL, SOCK_NONBLOCK), EAGAIN, EWOULDBLOCK);
          if (connfd < 0) {
            // EAGAIN or EWOULDBLOCK
            break;
          } else {
            printf("server: received connection on fd %d\n", connfd);
            struct epoll_event read_event = { .events = EPOLLIN | EPOLLRDHUP | EPOLLHUP, .data = { .fd = connfd } };
            BAIL_IF_ERR(epoll_ctl(epollfd, EPOLL_CTL_ADD, connfd, &read_event));
          }
        }
      } else if (fd == self_pipe_read_end) {
        // handle signal
        char buf[1];
        ssize_t nread = BAIL_IF_ERR(read(fd, buf, sizeof buf));
        if (nread == 0) {
          puts("server: read 0 bytes from signal pipe; something's not right...");
        } else {
          printf("server: shutting down due to signal %d\n", buf[0]);
          return 0;
        }
      } else {
        // assume any other file descriptor is a socket connection
        int close_connection = 0;
        if (event.events & (EPOLLHUP | EPOLLRDHUP | EPOLLERR)) {
          close_connection = 1;
        } else {
          while (1) {
            ssize_t nread = BAIL_IF_ERR_EXCEPT2(read(fd, read_buffer, sizeof read_buffer), EAGAIN, EWOULDBLOCK);
            if (nread == 0) {
              close_connection = 1;
              break;
            } else if (nread < 0) {
              // EAGAIN or EWOULDBLOCK
              break;
            } else {
              printf("server: echoing %ld byte(s) of data on fd %d\n", nread, fd);
              BAIL_IF_ERR(write(fd, read_buffer, nread));
            }
          }
        }

        if (close_connection) {
          printf("server: closing connection on fd %d\n", fd);
          BAIL_IF_ERR(epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, NULL));
          BAIL_IF_ERR(close(fd));
        }
      }
    }
  }
}

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
