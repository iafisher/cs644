"""
https://iafisher.com/cs644/fall2025/week4

Run with `strace` to see what syscalls `subprocess.run` makes.
"""
import subprocess

subprocess.run(["echo", "hello", "world"])
