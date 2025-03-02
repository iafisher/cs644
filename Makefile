CC = gcc
CFLAGS = -Wall -Wpedantic -Werror
OUTDIR = out

all: ${OUTDIR}/crashcourse ${OUTDIR}/redact ${OUTDIR}/switch.o

${OUTDIR}/crashcourse: week1/crashcourse.c
	$(CC) $(CFLAGS) -o $@ $<

${OUTDIR}/redact: week1/solutions/redact.c
	$(CC) $(CFLAGS) -o $@ $<

${OUTDIR}/switch.o: week1/switch.c
	$(CC) $(CFLAGS) -o $@ -c $<

.PHONY: clean
clean:
	rm -f ${OUTDIR}/*
