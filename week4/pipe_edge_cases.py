"""
Solutions to the in-class exercise "Pipe edge cases" for week 4 of CS644.

https://iafisher.com/cs644/summer2026/week4-ipc
"""
import argparse
import os
import signal


def read_empty():
    # What does `read` return when nothing has been written to the pipe?
    r, w = os.pipe()

    b = _read_with_timeout(r, secs=1)
    print("Read:", b)


def read_closed():
    # What does `read` return when the write end of the pipe is closed?
    r, w = os.pipe()
    os.close(w)

    b = _read_with_timeout(r, secs=1)
    print("Read:", b)


def write_closed():
    # What does `write` return when the read end of the pipe is closed?
    r, w = os.pipe()
    os.close(r)

    n = _write_with_timeout(w, b"Hello!\n", secs=1)
    print("Wrote:", n)


def write_limit():
    # Is there a limit to how much you can write to a pipe without reading anything?
    r, w = os.pipe()

    n = 0
    try:
        for _ in range(100_000):
            _write_with_timeout(w, b"0", secs=1)
            n += 1
    except:
        print(n)
        raise


def write_atomic():
    # Bonus: Can you show whether writes to a pipe are atomic?
    r, w = os.pipe()

    # TODO: when num_writes is set to 11, the last write blocks, even though pipe capacity should be
    # below the maximum
    num_writes = 10
    write_size = 5000

    def _write(i: int, fd: int):
        print(f"child {i} started")
        b = bytes([ord('a') + i] * write_size)
        os.write(fd, b)
        print(f"child {i} done")

    _fork_n(num_writes, _write, w)
    for _ in range(num_writes):
        b = os.read(r, write_size)
        if not all(b[0] == c for c in b):
            print("inter-leaved write:", b)
            break


def _fork_n(n: int, f, *args):
    if n == 0:
        return

    pid = os.fork()
    if pid == 0:
        try:
            f(n, *args)
        finally:
            os._exit(0)
    else:
        _fork_n(n - 1, f, *args)
        os.waitpid(pid, 0)


def _read_with_timeout(fd: int, *, secs: int) -> bytes:
    signal.alarm(secs)
    r = os.read(fd, 4096)
    signal.alarm(0)
    return r


def _write_with_timeout(fd: int, b: bytes, *, secs: int) -> int:
    signal.alarm(secs)
    r = os.write(fd, b)
    signal.alarm(0)
    return r


def _set_up_timeouts():
    def sighandler(*args):
        raise Exception("signal received")

    signal.signal(signal.SIGALRM, sighandler)


if __name__ == "__main__":
    argparse.ArgumentParser().parse_args()

    _set_up_timeouts()

    #read_empty()
    #read_closed()
    #write_closed()
    #write_limit()
    write_atomic()
