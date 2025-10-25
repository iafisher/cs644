project('cs644', 'c')

warning_level = 'everything'
werror = true

incdir = include_directories('include')
libfile = 'lib/cs644.c'

# week 1
executable('crashcourse', 'week1/crashcourse.c')
executable('switch', 'week1/switch.c')

# week 2
executable('count-whitespace', 'week2/solutions/count-whitespace.c', libfile, include_directories: incdir)
executable('line-by-line', 'week2/solutions/line-by-line.c', libfile, include_directories: incdir)
executable('open-errors', 'week2/solutions/open-errors.c')
executable('read-after-write', 'week2/solutions/read-after-write.c', libfile, include_directories: incdir)
executable('simultaneous', 'week2/solutions/simultaneous.c', libfile, include_directories: incdir)

# week 3
executable('cp', 'week3/solutions/cp.c', libfile, include_directories: incdir)
executable('getdents-atomic', 'week3/solutions/getdents-atomic.c', libfile, include_directories: incdir)
executable('files-are-hard', 'week3/solutions/files-are-hard.c', libfile, include_directories: incdir)

# week 4
executable('forkexec', 'week4/forkexec.c')
executable('execs', 'week4/solutions/execs.c', libfile, include_directories: incdir)
executable('mytime', 'week4/solutions/mytime.c', libfile, include_directories: incdir)

# week 5
executable('pipebuffer', 'week5/solutions/pipebuffer.c', libfile, include_directories: incdir)
executable('ringbuffer', 'week5/solutions/ringbuffer.c', libfile, include_directories: incdir)
executable('week5-in-class-example', 'week5/in-class-example.c', libfile, include_directories: incdir)

# week 6
executable('hello_conn_tcp', 'week6/hello_conn_tcp.c', libfile, include_directories: incdir)
executable('rot13_udp', 'week6/rot13_udp.c', libfile, include_directories: incdir)
executable('connect_bind', 'week6/solutions/connect_bind.c', libfile, include_directories: incdir)
executable('recv', 'week6/solutions/recv.c', libfile, include_directories: incdir)
executable('close_shutdown', 'week6/solutions/close_shutdown.c', libfile, include_directories: incdir)

# week 7
thread_dep = dependency('threads')
executable('week7_example', 'week7/in_class_example.c', libfile, dependencies: [thread_dep], include_directories: incdir)
executable('locks', 'week7/solutions/locks.c', libfile, dependencies: [thread_dep], include_directories: incdir)
executable('main_returns', 'week7/solutions/main_returns.c', libfile, dependencies: [thread_dep], include_directories: incdir)
executable('deadlock', 'week7/solutions/deadlock.c', libfile, dependencies: [thread_dep], include_directories: incdir)
executable('lock_n_exit', 'week7/solutions/lock_n_exit.c', libfile, dependencies: [thread_dep], include_directories: incdir)

# week 8
executable('sigcatch', 'week8/sigcatch.c', libfile, include_directories: incdir)

# week 9
executable('epoll_server', 'week9/epoll_server.c', libfile, include_directories: incdir)
# executable('open_nonblock', 'week9/open_nonblock.c', libfile, include_directories: incdir)
# executable('double_delivery', 'week9/solutions/double_delivery.c', libfile, include_directories: incdir)
# executable('reentrant_handler', 'week9/solutions/reentrant_handler.c', libfile, include_directories: incdir)
# executable('multiple_pending', 'week9/solutions/multiple_pending.c', libfile, include_directories: incdir)
# executable('sig_fork', 'week9/solutions/sig_fork.c', libfile, include_directories: incdir)
# executable('sig_pre_exec', 'week9/solutions/sig_pre_exec.c', libfile, include_directories: incdir)
# executable('sig_post_exec', 'week9/solutions/sig_post_exec.c', libfile, include_directories: incdir)
# executable('signalfd', 'week9/solutions/signalfd.c', libfile, include_directories: incdir)
