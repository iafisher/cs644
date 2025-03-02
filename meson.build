project('cs644', 'c')

warning_level = 'everything'
werror = true

incdir = include_directories('include')
libfile = 'lib/cs644.c'

executable('crashcourse', 'week1/crashcourse.c')
executable('switch', 'week1/switch.c')
executable('redact', 'week1/solutions/redact.c')
executable('week2-example', 'week2/example.c')
executable('count-whitespace', 'week2/solutions/count-whitespace.c')
executable('line-by-line', 'week2/solutions/line-by-line.c', libfile, include_directories: incdir)
executable('open-errors', 'week2/solutions/open-errors.c')
executable('read-after-write', 'week2/solutions/read-after-write.c', libfile, include_directories: incdir)
executable('simultaneous', 'week2/solutions/simultaneous.c', libfile, include_directories: incdir)
