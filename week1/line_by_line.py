"""
Solution to the bonus exercise for week 1 of CS644:

    Using `os.read`, write a Python function or class that reads a file
    line-by-line.

https://iafisher.com/cs644/summer2026/week1-files-pt-1
"""

import argparse
import os


DEFAULT_CHUNK_SIZE = 4096


class LineReader:
    def __init__(self, fd: int, *, chunk_size = DEFAULT_CHUNK_SIZE) -> None:
        self.fd = fd
        self.buffer = bytearray()
        self.eof = False
        self.chunk_size = chunk_size

    def read_line(self) -> bytes:
        """
        Read the next line of the file.

        Returns an empty bytestring at end of file.
        """

        if self.eof:
            return b""

        while True:
            # We want to read the file line-by-line, but `os.read` will only read a
            # fixed number of bytes.
            #
            # Solution: Store what we read in a buffer. If it undershot (did not read
            # until the end of the line), then keep reading into the buffer. If it
            # overshot (read past the end of the line), then take the line and leave
            # the remainder in the buffer.

            # First, see if the next line is entirely in the buffer.
            newline_index = self.buffer.find(b"\n")
            if newline_index != -1:
                # If so, simply return it.
                return self._consume_from_buffer(newline_index + 1)
            else:
                # Edge case: We've reached the end of the file and it does not end
                # with a newline. We'll consider the trailing bytes to be the final
                # line and return that.
                if self.eof:
                    return self._consume_from_buffer(len(self.buffer))

                # Otherwise, read more into the buffer and loop.
                self._read_into_buffer(self.chunk_size)

    def _read_into_buffer(self, n: int) -> None:
        b = os.read(self.fd, n)
        if len(b) == 0:
            self.eof = True
        else:
            self.buffer += b

    def _consume_from_buffer(self, n: int) -> bytes:
        r = self.buffer[:n]
        self.buffer = self.buffer[n:]
        return bytes(r)


if __name__ == "__main__":
    ap = argparse.ArgumentParser()
    ap.add_argument("filepath")
    ap.add_argument("--chunk-size", type=int, default=DEFAULT_CHUNK_SIZE)
    args = ap.parse_args()

    fd = os.open(args.filepath, os.O_RDONLY)
    try:
        reader = LineReader(fd, chunk_size=args.chunk_size)
        while True:
            line = reader.read_line()
            if len(line) == 0:
                break
            print(repr(line))
    finally:
        os.close(fd)
