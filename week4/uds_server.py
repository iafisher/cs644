"""
Solution to the in-class exercise "UDS client and server" for week 4 of CS644.

https://iafisher.com/cs644/summer2026/week4-ipc
"""
import argparse
import os
import socket


def client(socketpath: str) -> None:
    sock = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
    sock.connect(socketpath)
    sock.send(b"hello!")
    msg = sock.recv(4096)
    print("Received:", msg)


def server(socketpath: str) -> None:
    sock = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
    sock.bind(socketpath)
    try:
        sock.listen()
        print(f"server: listening at {socketpath}")
        while True:
            conn, _ = sock.accept()
            print("server: got connection")
            msg = conn.recv(4096)
            conn.send(msg[::-1])
            conn.close()
    finally:
        os.unlink(socketpath)


if __name__ == "__main__":
    ap = argparse.ArgumentParser()
    ap.add_argument("mode", choices=("client", "server"))
    ap.add_argument("--socket", required=True)
    args = ap.parse_args()

    match args.mode:
        case "client":
            client(args.socket)
        case "server":
            server(args.socket)
        case _:
            raise Exception(args.mode)
