"""
Solution to the in-class exercise "Pipes and fork" for week 4 of CS644.

https://iafisher.com/cs644/summer2026/week4-ipc
"""
import argparse
import os
import sys
from typing import List


def run_and_capture_stdout(prog: str, args: List[str]) -> bytes:
    # The overall structure of this function is similar to `week3/run.py`; see that file for
    # commentary.

    # Create a pipe before forking.
    r, w = os.pipe()

    pid = os.fork()
    # After forking, there are now 2 copies of each file descriptor: the parent and the child
    # each have references to both the read and the write ends. It's a good idea for each
    # process to close the end that it does not need.
    if pid == 0:
        try:
            os.close(r)
            # Point standard output at the write end of the pipe.
            os.dup2(w, sys.stdout.fileno())
            os.execve(prog, [prog] + args, os.environ)
        finally:
            os._exit(127)
    else:
        os.close(w)
        # The child has had its standard output redirected to the pipe, so we read it here
        # until we reach end of file.
        #
        # It's important to call this *before* waiting for the child process to exit, in
        # case the child writes more bytes than the pipe's capacity as the child would block
        # without someone reading off the other end of the pipe.
        stdout = _read_all(r)
        os.close(r)

        _, waitstatus = os.waitpid(pid, 0)
        exitcode = os.waitstatus_to_exitcode(waitstatus)
        if exitcode != 0:
            raise Exception("child exited with non-zero status", exitcode)

        return stdout


def _read_all(fd: int) -> bytes:
    buf = bytearray()
    while True:
        b = os.read(fd, 4096)
        if len(b) == 0:
            break
        buf.extend(b)
    return bytes(buf)


if __name__ == "__main__":
    argparse.ArgumentParser().parse_args()  # handle --help flag

    print("==> running echo")
    stdout = run_and_capture_stdout("/usr/bin/echo", ["hello", "world"])
    print(f"==> captured: {stdout}")
