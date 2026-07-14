"""
Solution to the in-class exercise "Re-implement subprocess.run" for week 3 of CS644.

https://iafisher.com/cs644/summer2026/week3-process-control
"""
import argparse
import os
from typing import List


def run(prog: str, args: List[str]) -> int:
    pid = os.fork()
    if pid == 0:
        try:
            os.execve(prog, [prog] + args, os.environ)
        finally:
            os._exit(127)
    else:
        _, waitstatus = os.waitpid(pid, 0)
        return os.waitstatus_to_exitcode(waitstatus)


if __name__ == "__main__":
    argparse.ArgumentParser().parse_args()  # handle --help flag

    print("==> running echo")
    r = run("/usr/bin/echo", ["hello", "world"])
    print(f"==> returned: {r}")

    print("==> running grep")
    r = run("/usr/bin/grep", ["cs644", "/etc/hosts"])
    print(f"==> returned: {r}")

    print("==> running non-existent-command")
    r = run("non-existent-command", [])
    print(f"==> returned: {r}")
