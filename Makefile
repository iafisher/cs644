CC = clang
CFLAGS = -Wall -Wpedantic -Werror
OUTDIR = out

${OUTDIR}/crashcourse: week1/crashcourse.c
	$(CC) $(CFLAGS) -o $@ $<

.PHONY: clean
clean:
	rm -f ${OUTDIR}/*
