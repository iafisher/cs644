CC = clang
CFLAGS = -Wall -Wpedantic -Werror
OUTDIR = out

all: ${OUTDIR}/crashcourse ${OUTDIR}/switch.o

${OUTDIR}/crashcourse: week1/crashcourse.c
	$(CC) $(CFLAGS) -o $@ $<

${OUTDIR}/switch.o: week1/switch.c
	$(CC) $(CFLAGS) -o $@ -c $<

.PHONY: clean
clean:
	rm -f ${OUTDIR}/*
