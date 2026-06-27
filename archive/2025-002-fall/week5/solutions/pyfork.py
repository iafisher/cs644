"""
https://iafisher.com/cs644/fall2025/week5
"""
import os
import sys
from typing import List


def run_command(args: List[str]) -> str:
    """
    Runs the command specified by `args`, returning its captured standard output.
    """
    r, w = os.pipe()
    pid = os.fork()
    if pid == 0:
        # child
        try:
            os.close(r)
            os.dup2(w, sys.stdout.fileno())
            os.execvp(args[0], args)
        except:
            os._exit(127)
    else:
        # parent
        os.close(w)
        _, exitcode = os.waitpid(pid, 0)
        print(f"Child exited with status {exitcode}.")
        with os.fdopen(r) as f:
            return f.read()


stdout = run_command(["echo", "hello", "world"])
print(f"Got output: {stdout!r}")
