"""
Solutions to the in-class exercise "UDS edge cases" for week 4 of CS644.

https://iafisher.com/cs644/summer2026/week4-ipc
"""
import argparse
import os
import socket


def incompatible():
    socketpath = "test.sock"
    server_sock = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
    server_sock.bind(socketpath)
    try:
        client_sock = socket.socket(socket.AF_UNIX, socket.SOCK_DGRAM)
        client_sock.connect(socketpath)
    finally:
        os.unlink(socketpath)


def read_closed():
    server_sock, client_sock = socket.socketpair()
    pid = os.fork()
    if pid == 0:
        try:
            server_sock.close()
        finally:
            os._exit(0)
    else:
        client_sock.close()


if __name__ == "__main__":
    argparse.ArgumentParser().parse_args()

    incompatible()
