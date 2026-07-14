"""
First attempt at a solution to the in-class exercise "Deleting a file tree"
for week 2 of CS644.

https://iafisher.com/cs644/summer2026/week2-files-pt-2
"""
import argparse
import os


def rmtree_v1_dangerous(path: str) -> None:
    """
    Recursively remove the directory at `path`.

    WARNING: This implementation is unsafe.
    """
    for entry in os.scandir(path):
        p = os.path.join(path, entry.name)
        if entry.is_dir():
            rmtree_v1_dangerous(p)
        else:
            os.unlink(p)

    os.rmdir(path)


if __name__ == "__main__":
    ap = argparse.ArgumentParser()
    ap.add_argument("path")
    args = ap.parse_args()

    rmtree_v1_dangerous(args.path)
