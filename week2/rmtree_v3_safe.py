"""
Solution to the in-class exercise "Deleting a file tree" for week 2 of CS644.

https://iafisher.com/cs644/summer2026/week2-files-pt-2
"""
import argparse
import os

# There were two subtle issues with `rmtree_v2_dangerous.py`:
#
#   - A timing attack: if Eve can change a directory to a symlink to a directory in between the
#     calls to `is_dir` and `rmtree`, she can still trick Bob into deleting his own files.
#   - Symlinks in parent directories: If `rmtree` is in the middle of deleting a/b/c/, an
#     attacker can change a/b to be a symlink pointing to a directory they control, e.g., Bob is
#     deleting a/b/c/, Eve creates a/b/c/silver.txt and a/b/c/gold.txt, then changes a/b to be a
#     symlink pointing to /tmp/devious, where she has already created a symlink /tmp/devious/c
#     that points to /home/bob/valuables. Now, when Bob tries to remove a/b/c/silver.txt, he will
#     actually remove /home/bob/valuables/silver.txt.
#
# The crux of both of these issues is that we are resolving each path multiple times, but the path
# can change under our feet. For instance, we call `os.scandir("a/b")` and get back that entry `c`
# is a directory and not a symlink. Satisfied, we then recursively call `rmtree` and make a call to
# `os.scandir("a/b/c")`. We re-resolve `a/b/c` which by now could have been turned into a symlink.
#
# The solution is to only resolve each path once and do the symlink/directory checks simultaneously
# with resolving. We'll pass around file descriptors instead of paths that could be manipulated.


def rmtree_v3_safe(path: str) -> None:
    """
    Recursively remove the directory at `path`.
    """

    # `O_DIRECTORY` ensures that the path is a directory, and `O_NOFOLLOW` tells the kernel not to
    # follow a symlink in the final component of `path`. (It does _not_ prevent the kernel from
    # following symlinks in the prefix of `path`.)
    fd = os.open(path, os.O_RDONLY | os.O_DIRECTORY | os.O_NOFOLLOW)
    try:
        rmtree_fd(fd)
        # There's a small flaw here in that we call `rmdir` on the path itself, which could have
        # been turned into a symlink in the meantime. In practice, this only lets an attacker trick
        # you into deleting an empty directory, which probably doesn't matter.
        os.rmdir(path)
    finally:
        os.close(fd)


def rmtree_fd(dir_fd: int) -> None:
    # Calling `list` on `os.scandir` forces Python to evaluate all the directory entries up front,
    # which reduces the window for any timing attacks.
    entries = list(os.scandir(dir_fd))
    for entry in entries:
        # First, check if the entry was a non-symlink directory at the time it was read by
        # `os.scandir`.
        if entry.is_dir(follow_symlinks=False):
            # If so, try to open it with `os.O_DIRECTORY | os.O_NOFOLLOW`, which will succeed
            # _only_ if the entry is in fact a non-symlink directory.
            #
            # We also pass a third parameter, `dir_fd`. This tells the kernel to evaluate the path
            # relative to the directory identified by `dir_fd` rather than the current working
            # directory, which is the default. Since `entry.name` is a single path component, this
            # defeats any attempts to make a parent directory a symlink.
            try:
                subdir_fd = os.open(
                    entry.name, os.O_RDONLY | os.O_DIRECTORY | os.O_NOFOLLOW, dir_fd=dir_fd
                )
            except OSError:
                # If this failed, fall back to unlinking it. (We could also raise an exception or
                # print a warning here as some mischief is likely going on.)
                os.unlink(entry.name, dir_fd=dir_fd)
            else:
                # If `os.open` succeeded, we now have a file descriptor that we know points to a
                # real directory and can never be changed into a symlink.
                try:
                    rmtree_fd(subdir_fd)
                    os.rmdir(entry.name, dir_fd=dir_fd)
                finally:
                    os.close(subdir_fd)
        else:
            os.unlink(entry.name, dir_fd=dir_fd)


# `O_DIRECTORY` and `O_NOFOLLOW` exist on Linux but are not universally available. The traditional
# cross-platform solution is:
#
#  - Call `lstat` on the path to be opened.
#  - If `lstat` reports that it is a directory, call `open`.
#  - On the file descriptor returned by `open`, call `fstat`. If the inode and device number are
#    the same, we know that the file has not been tampered and we can safely continue. If they are
#    different, then something is fishy.
#
# In each of the syscalls, we still need to use `dir_fd` to prevent from parent-symlink attacks.


if __name__ == "__main__":
    ap = argparse.ArgumentParser()
    ap.add_argument("path")
    args = ap.parse_args()

    rmtree_v3(args.path)
