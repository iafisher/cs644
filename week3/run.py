"""
Solution to the in-class exercise "Re-implement subprocess.run" for week 3 of CS644.

https://iafisher.com/cs644/summer2026/week3-process-control
"""
import argparse
import os
import sys
from typing import List, Optional


def run(prog: str, args: List[str], *, stdout: Optional[str] = None) -> int:
    pid = os.fork()
    if pid == 0:
        # PID 0 indicates that `fork()` returned into the child process.

        # The entire child process must be wrapped in a `try-finally` block that ends with
        # `os._exit`, to ensure that the child process never "escapes" out of this function
        # into the rest of the program which could result in undesirable behavior. For
        # instance, imagine if the parent process called `run` inside of a `TemporaryDirectory`
        # context manager. If the child process "escaped", it could call the clean-up
        # handler for the temporary directory and remove it under the parent's feet!
        try:
            if stdout is not None:
                # Arrange for standard output to be redirected to a file. First, open the
                # file normally.
                fd = os.open(stdout, os.O_WRONLY | os.O_APPEND | os.O_CREAT)
                # Then, use `os.dup2` to replace the file descriptor for standard output with
                # the regular file's FD. We call `sys.stdout.fileno()` but could just as well
                # hard-coded the number 1, which is always the file descriptor for standard
                # output.
                os.dup2(fd, sys.stdout.fileno())

            # Call `execve` to replace the currently-running program with `prog`. Unless there
            # is an error, `execve` will never return.
            os.execve(prog, [prog] + args, os.environ)
        finally:
            # If we exit the block at all, something has gone wrong, so exit with an error.
            # Call `_exit` to exit immediately; if we called `exit` (no underscore), we would
            # raise the `SystemExit` exception in Python which would run all the clean-up
            # handlers, which is normally a good idea but definitely not what we want in a
            # child process created from `fork`.
            os._exit(127)
    else:
        # The parent waits for the child to finish. The return value of `waitpid` is a wait
        # status, which includes some extra information aside from the exit code (for example,
        # whether the child process was terminated by a single or exited normally). Use the
        # library function `waitstatus_to_exitcode` to extract the exit code.
        _, waitstatus = os.waitpid(pid, 0)
        return os.waitstatus_to_exitcode(waitstatus)


if __name__ == "__main__":
    argparse.ArgumentParser().parse_args()  # handle --help flag

    print("==> running echo")
    r = run("/usr/bin/echo", ["hello", "world"])
    print(f"==> returned: {r}")

    print("==> running echo, redirected to /tmp/hello.txt")
    r = run("/usr/bin/echo", ["hello", "world"], stdout="/tmp/hello.txt")
    print(f"==> returned: {r}")

    print("==> running grep")
    r = run("/usr/bin/grep", ["cs644", "/etc/hosts"])
    print(f"==> returned: {r}")

    print("==> running non-existent-command")
    r = run("non-existent-command", [])
    print(f"==> returned: {r}")
