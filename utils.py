import os
import sys
import traceback


def multiprocess(f1, f2, *args, **kwargs) -> None:
    """
    Call `f1(*args, **kwargs)` and `f2(*args, **kwargs)` simultaneously in different processes.
    """
    pid = os.fork()
    if pid == 0:
        try:
            f1(*args, **kwargs)
        except:
            print(traceback.format_exc(), file=sys.stderr)
            os._exit(1)
        else:
            os._exit(0)
    else:
        f2(*args, **kwargs)

        _, waitstatus = os.waitpid(pid, 0)
        exitcode = os.waitstatus_to_exitcode(waitstatus)
        if exitcode != 0:
            raise Exception(f"child process exited with non-zero code: {exitcode}")


class Waitpoint:
    NOT_DONE = 0
    DONE = 1

    def __init__(self):
        self.fd = os.eventfd(self.NOT_DONE)

    def wait(self) -> None:
        os.eventfd_read(self.fd)

    def mark_done(self) -> None:
        os.eventfd_write(self.fd, self.DONE)
