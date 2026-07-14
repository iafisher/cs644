import argparse
import os
import random
import string


def make_tree(
    path: str, *, depth: int, with_symlinks: bool, randomize_permissions: bool, exist_ok: bool
) -> None:
    if depth == 0:
        return

    try:
        os.mkdir(path)
    except FileExistsError:
        if exist_ok:
            return
        else:
            raise

    nfiles = random.randint(3, 8)
    ndirs = random.randint(3, 8)
    nlinks = random.randint(0, 2) if with_symlinks else 0

    filenames = []
    for _ in range(nfiles):
        filename = random_letters(5) + ".txt"
        filenames.append(filename)

        make_random_file(os.path.join(path, filename), randomize_permissions=randomize_permissions)

    for _ in range(ndirs):
        dirname = random_letters(4)
        make_tree(
            os.path.join(path, dirname),
            depth=depth - 1,
            with_symlinks=with_symlinks,
            randomize_permissions=randomize_permissions,
            exist_ok=True,
        )

    for _ in range(nlinks):
        target = random.choice(filenames)
        linkname = os.path.join(path, random_letters(3) + ".link")
        try:
            os.symlink(target, linkname)
        except FileExistsError:
            pass


def make_random_file(path: str, *, randomize_permissions: bool) -> None:
    perms = random.choice([0o644, 0o600, 0o666]) if randomize_permissions else 0o644
    fd = os.open(path, os.O_CREAT | os.O_WRONLY, perms)
    try:
        contents = random_letters(random.randint(10, 500)) + "\n"
        os.write(fd, contents.encode("ascii"))
    finally:
        os.close(fd)


def random_letters(n: int) -> str:
    return "".join(random.choices(string.ascii_lowercase, k=n))


if __name__ == "__main__":
    ap = argparse.ArgumentParser(description="Create a directory tree with dummy entries.")
    ap.add_argument("path")
    ap.add_argument("--depth", type=int, default=5)
    ap.add_argument("--with-symlinks", action="store_true")
    ap.add_argument("--randomize-permissions", action="store_true")
    ap.add_argument("--random-seed")
    args = ap.parse_args()

    random.seed(a=args.random_seed)
    make_tree(
        args.path,
        depth=args.depth,
        with_symlinks=args.with_symlinks,
        randomize_permissions=args.randomize_permissions,
        exist_ok=False,
    )
