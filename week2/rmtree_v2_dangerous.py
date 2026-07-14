"""
Second attempt at a solution to the in-class exercise "Deleting a file tree"
for week 2 of CS644.

https://iafisher.com/cs644/summer2026/week2-files-pt-2
"""
import argparse
import os


def rmtree_v2_dangerous(path: str) -> None:
    """
    Recursively remove the directory at `path`.

    WARNING: This implementation is still unsafe.
    """
    for entry in os.scandir(path):
        p = os.path.join(path, entry.name)
        # The vulnerability in the previous version was that an attacker could create a symlink
        # that causes the user to inadvertently delete their own files.
        #
        # Example: Bob is calling `rmtree` on /tmp/myfiles. Eve symlinks /tmp/myfiles/mischief to
        # /home/bob/valuables (she can do this because everyone has write permissions under /tmp).
        # If we called `is_dir` on /tmp/myfiles/mischief, it would follow the symlink and return
        # true, and Bob would start deleting his own valuables! This attack does not require any
        # delicate timing, either: Eve could create her symlink as soon as she see Bob create a
        # temporary directory, before he has even started to delete it.
        #
        # `follow_symlinks=False` is a good start, but unfortunately it's not enough.
        if entry.is_dir(follow_symlinks=False):
            rmtree_v2_dangerous(p)
        else:
            os.unlink(p)

    os.rmdir(path)


if __name__ == "__main__":
    ap = argparse.ArgumentParser()
    ap.add_argument("path")
    args = ap.parse_args()

    rmtree_v2_dangerous(args.path)
