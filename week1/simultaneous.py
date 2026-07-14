"""
Solution to the in-class exercise "Simultaneous read/write" for week 1 of CS644.

https://iafisher.com/cs644/summer2026/week1-files-pt-1
"""
import argparse

import os
import uuid
from pathlib import Path

from utils import Waitpoint, multiprocess


def main(*, reps: int, chunk_size: int, preserve_file: bool = False) -> None:
    wp = Waitpoint()
    filepath = f"simultaneous-{uuid.uuid4()}.txt"
    try:
        multiprocess(reader, writer, filepath, wp, reps=reps, chunk_size=chunk_size)
    finally:
        if preserve_file:
            print(f"File: {filepath}")
        else:
            os.unlink(filepath)


def reader(filepath: str, wp: Waitpoint, *, reps: int, chunk_size: int) -> None:
    # Wait for the writer to create the file and do the first write.
    wp.wait()

    fd = os.open(filepath, os.O_RDONLY)
    try:
        print("reader: started")
        samples = []
        for i in range(reps):
            buf = os.read(fd, chunk_size)
            assert len(buf) == chunk_size
            samples.append(buf)
            os.lseek(fd, 0, os.SEEK_SET)

        print(f"reader: finished {reps}, checking samples")

        for i, sample in enumerate(samples, start=1):
            if any(c != sample[0] for c in sample):
                print(f"reader: detected partial write on rep {i}")
                if chunk_size <= 4096:
                    print()
                    print(sample)
                    print()
                return

        print("reader: no partial write detected")
    finally:
        os.close(fd)


def writer(filepath: str, wp: Waitpoint, *, reps: int, chunk_size: int) -> None:
    a = ord('a')
    bufs = [bytes([a + i] * chunk_size) for i in range(26)]

    fd = os.open(filepath, os.O_WRONLY | os.O_CREAT | os.O_EXCL, 0o600)
    try:
        print("writer: started")
        for i in range(reps):
            os.write(fd, bufs[i % len(bufs)])
            os.lseek(fd, 0, os.SEEK_SET)

            if i == 0:
                # First write completed, reader can now start.
                wp.mark_done()

        print(f"writer: finished {reps}")
    finally:
        os.close(fd)


if __name__ == "__main__":
    ap = argparse.ArgumentParser()
    ap.add_argument("--chunk-size", type=int, default=4096, help="Read/write in chunks of this size.")
    ap.add_argument("--reps", type=int, default=100_000, help="How many times to read and write.")
    ap.add_argument("--preserve-file", action="store_true", help="Don't delete the file at the end.")
    args = ap.parse_args()

    main(reps=args.reps, chunk_size=args.chunk_size, preserve_file=args.preserve_file)
