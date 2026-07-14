"""
Solution to the in-class exercise "Copying a file tree" for week 2 of CS644.

https://iafisher.com/cs644/summer2026/week2-files-pt-2
"""
import argparse
import os
import stat

from week1.duplicate import duplicate


def copytree(inpath: str, outpath: str, *, dry_run: bool, copy_permissions: bool) -> int:
    """
    Recursively copy all files and directories under `inpath` to `outpath`, creating the latter.

    Returns the number of bytes that were copied (only counting regular files).

    If `dry_run` is true, no changes are made, but the number of bytes that would be copied is
    still returned.

    If `copy_permissions` is true, then the permission bits of the source files and directories
    are copied to the destination.
    """
    bytes_copied = 0

    if not dry_run:
        os.mkdir(outpath)
        if copy_permissions:
            statres = os.stat(inpath)
            os.chmod(outpath, stat.S_IMODE(statres.st_mode))

    for entry in os.scandir(inpath):
        # `entry.name` is just the filename, so we have to join it to the directory to form a full
        # path.
        ip = os.path.join(inpath, entry.name)
        op = os.path.join(outpath, entry.name)

        # The default for the `is_XXX` functions is `follow_symlinks=True`, but we want to treat
        # symlinks specially.
        if entry.is_dir(follow_symlinks=False):
            # Directory case: simple recursion.
            bytes_copied += copytree(ip, op, dry_run=dry_run, copy_permissions=copy_permissions)
        elif entry.is_symlink():
            # Symlink case: copy the literal text of the symlink, retrieved via `os.readlink`.
            #
            # This is what `cp -r` does. Note that it may produce invalid symlinks. For example,
            # if /tmp/mydir/mylink points to `../../home/ian/myfile.txt`, then copying to the
            # destination /home/ian/mycopy will produce /home/ian/mycopy/mylink resolving to
            # /home/ian/mycopy/../../home/ian/myfile --> /home/home/ian/myfile, an invalid target.
            # You could imagine making the link an absolute path, but that is not what `cp` does.
            if not dry_run:
                text = os.readlink(ip)
                os.symlink(text, op)
        elif entry.is_file(follow_symlinks=False):
            # Regular file case: use our `duplicate` function from week 1.
            statres = os.stat(ip)
            bytes_copied += statres.st_size

            if not dry_run:
                duplicate(ip, op)
                if copy_permissions:
                    # `duplicate` will create a new file with default permissions. If we want to
                    # copy permissions, we need to explicitly change the permissions to match the
                    # old file. `stat.S_IMODE(statres.st_mode)` is the (somewhat verbose) way to
                    # get the old file's permission bits.
                    #
                    # The new file will also belong to whoever ran `copytree`, which may be
                    # different from the owner of the original file. However, `os.chown` requires
                    # you to be root, so there's nothing we can do about that.
                    os.chmod(op, stat.S_IMODE(statres.st_mode))
        else:
            print(f"WARNING: not copying unknown file type: {ip}")

    return bytes_copied


if __name__ == "__main__":
    ap = argparse.ArgumentParser()
    ap.add_argument("inpath")
    ap.add_argument("outpath")
    ap.add_argument("--dry-run", action="store_true")
    ap.add_argument("--copy-permissions", action="store_true")
    args = ap.parse_args()

    bytes_copied = copytree(
        args.inpath, args.outpath, dry_run=args.dry_run, copy_permissions=args.copy_permissions
    )
    if args.dry_run:
        print(f"Would have copied {bytes_copied} byte(s)")
