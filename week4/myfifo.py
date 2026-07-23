"""
Solution to the in-class exercise "FIFOs" for week 4 of CS644.

https://iafisher.com/cs644/summer2026/week4-ipc
"""
import argparse
import os
import time


if __name__ == "__main__":
    ap = argparse.ArgumentParser()
    ap.add_argument("fifo_path")
    args = ap.parse_args()

    # Create the FIFO (named pipe). This merely creates it; it does not open it.
    os.mkfifo(args.fifo_path)
    try:
        print("Reading from the pipe.")
        print("Try:")
        print(f"  echo hello > {args.fifo_path}")
        print()
        print("To exit:")
        print(f"  echo exit > {args.fifo_path}")

        # Outer loop calls `open`, which blocks until there is a writer.
        while True:
            fd = os.open(args.fifo_path, os.O_RDONLY)
            should_exit = False

            # Inner loop calls `read`. Why not `open` outside the loop and have a single
            # `read` loop, like a regular file?
            #
            # As soon as `echo hello > test.fifo` exits, `read` will immediately start
            # returning the empty bytestring since there are no more writers, and will
            # continue to do so until a new writer opens the FIFO. So we'd either have to
            # busy-loop on `read` or add a sleep interval. Better to let `open` do the
            # blocking.
            while True:
                b = os.read(fd, 4096)
                if len(b) == 0:
                    break

                print("Read:", b)
                if b == b"exit\n":
                    should_exit = True
                    break

            if should_exit:
                break
    finally:
        os.unlink(args.fifo_path)
