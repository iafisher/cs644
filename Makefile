CC = clang
CFLAGS = -Wall -Wpedantic -Werror
OUTDIR = out

${OUTDIR}/crashcourse: crashcourse.c
	$(CC) $(CFLAGS) -o $@ $<

.PHONY: clean
clean:
	rm -f ${OUTDIR}/*
