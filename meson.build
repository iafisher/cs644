project('cs644', 'c')

warning_level = 'everything'
werror = true

incdir = include_directories('include')
libfile = 'lib/cs644.c'

# final projects
# executable('db', 'final/db/main.c', libfile, include_directories: incdir)
# executable('http', 'final/http/main.c', libfile, include_directories: incdir)

# week 1
executable('crashcourse', 'week1/crashcourse.c')
executable('switch', 'week1/switch.c')
# executable('redact', 'week1/solutions/redact.c')

# week 2
executable('count-whitespace', 'week2/solutions/count-whitespace.c', libfile, include_directories: incdir)
executable('line-by-line', 'week2/solutions/line-by-line.c', libfile, include_directories: incdir)
executable('open-errors', 'week2/solutions/open-errors.c')
executable('read-after-write', 'week2/solutions/read-after-write.c', libfile, include_directories: incdir)
executable('simultaneous', 'week2/solutions/simultaneous.c', libfile, include_directories: incdir)
 
# # week 3
# executable('simultaneous-with-locking', 'week3/simultaneous-with-locking.c', libfile, include_directories: incdir)
# executable('printperm', 'week3/solutions/printperm.c', libfile, include_directories: incdir)
executable('cp', 'week3/solutions/cp.c', libfile, include_directories: incdir)
executable('getdents-atomic', 'week3/solutions/getdents-atomic.c', libfile, include_directories: incdir)
executable('files-are-hard', 'week3/solutions/files-are-hard.c', libfile, include_directories: incdir)

# # week 4
executable('forkexec', 'week4/forkexec.c')
executable('execs', 'week4/solutions/execs.c', libfile, include_directories: incdir)
executable('mytime', 'week4/solutions/mytime.c', libfile, include_directories: incdir)

# # week 5
# executable('shm_example', 'week5/shm_example.c')
executable('pipebuffer', 'week5/solutions/pipebuffer.c', libfile, include_directories: incdir)
executable('ringbuffer', 'week5/solutions/ringbuffer.c', libfile, include_directories: incdir)

# # week 6
# executable('hello_conn', 'week6/hello_conn.c', libfile, include_directories: incdir)

# # week 7
# thread_dep = dependency('threads')
# executable('locks', 'week7/solutions/locks.c', libfile, dependencies: [thread_dep], include_directories: incdir)
# executable('main_returns', 'week7/solutions/main_returns.c', libfile, dependencies: [thread_dep], include_directories: incdir)
# executable('deadlock', 'week7/solutions/deadlock.c', libfile, dependencies: [thread_dep], include_directories: incdir)

# # week 8
# executable('sigexample', 'week8/sigexample.c', libfile, include_directories: incdir)

# # week 9
# executable('open_nonblock', 'week9/open_nonblock.c', libfile, include_directories: incdir)
# executable('double_delivery', 'week9/solutions/double_delivery.c', libfile, include_directories: incdir)
# executable('reentrant_handler', 'week9/solutions/reentrant_handler.c', libfile, include_directories: incdir)
# executable('multiple_pending', 'week9/solutions/multiple_pending.c', libfile, include_directories: incdir)
# executable('sig_fork', 'week9/solutions/sig_fork.c', libfile, include_directories: incdir)
# executable('sig_pre_exec', 'week9/solutions/sig_pre_exec.c', libfile, include_directories: incdir)
# executable('sig_post_exec', 'week9/solutions/sig_post_exec.c', libfile, include_directories: incdir)
# executable('signalfd', 'week9/solutions/signalfd.c', libfile, include_directories: incdir)
