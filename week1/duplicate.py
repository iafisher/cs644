"""
Solution to the in-class exercise "Duplicating a file" for week 1 of CS644.

https://iafisher.com/cs644/summer2026/week1-files-pt-1
"""

import argparse
import os


def duplicate(inpath: str, outpath: str, *, block_size: int = 4096) -> None:
    """
    Copy all the bytes of `inpath` to `outpath`, creating the latter.
    """
    ifd = os.open(inpath, os.O_RDONLY)
    # `os.O_EXCL` causes `os.open` to raise an error if the file already exists,
    # ensuring we do not overwrite an existing file.
    #
    # The third argument to `os.open` sets the permissions of the newly-created
    # file. Unix file permissions are covered in Week 2.
    ofd = os.open(outpath, os.O_WRONLY | os.O_CREAT | os.O_EXCL, 0o644)

    try:
        # `os.read` reads a fixed number of bytes, but the size of the contents of
        # `inpath` is variable. We could call `os.read` with a very large number,
        # but (a) this could still fail on a very large file, and (b) it would be
        # wasteful of memory.
        #
        # Solution: Call `os.read` and `os.write` in a loop until the end of the
        # file is reached.
        while True:
            b = os.read(ifd, block_size)
            if len(b) == 0:
                break
            os.write(ofd, b)
    finally:
        # For this particular program, it's not really important to call `os.close`
        # as the operating system will clean up any open file descriptors when the
        # program exits. However, it's a good habit to get into.
        os.close(ifd)
        os.close(ofd)


def duplicate_range(inpath: str, outpath: str, *, range_start: int, range_end: int) -> None:
    ifd = os.open(inpath, os.O_RDONLY)
    ofd = os.open(outpath, os.O_WRONLY | os.O_CREAT | os.O_EXCL, 0o644)

    try:
        os.lseek(ifd, range_start, os.SEEK_SET)
        # For simplicity, I make one call to `os.read` with the whole range since
        # we know ahead of time how many bytes we want to copy.
        #
        # In a real-world program, it would be advisable to read in smaller chunks
        # in case `range_end - range_start + 1` is very large.
        b = os.read(ifd, range_end - range_start + 1)
        os.write(ofd, b)
    finally:
        os.close(ifd)
        os.close(ofd)


if __name__ == "__main__":
    ap = argparse.ArgumentParser()
    ap.add_argument("inpath")
    ap.add_argument("outpath")
    args = ap.parse_args()

    duplicate(args.inpath, args.outpath)
