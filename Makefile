CC = gcc
CFLAGS = -Wall -Wpedantic -Werror
OUTDIR = out

all: ${OUTDIR}/crashcourse ${OUTDIR}/redact ${OUTDIR}/switch.o ${OUTDIR}/open-errors ${OUTDIR}/count-whitespace ${OUTDIR}/line-by-line

${OUTDIR}/crashcourse: week1/crashcourse.c
	$(CC) $(CFLAGS) -o $@ $<

${OUTDIR}/redact: week1/solutions/redact.c
	$(CC) $(CFLAGS) -o $@ $<

${OUTDIR}/switch.o: week1/switch.c
	$(CC) $(CFLAGS) -o $@ -c $<

${OUTDIR}/open-errors: week2/solutions/open-errors.c
	$(CC) $(CFLAGS) -o $@ $<

${OUTDIR}/count-whitespace: week2/solutions/count-whitespace.c
	$(CC) $(CFLAGS) -o $@ $<

${OUTDIR}/line-by-line: week2/solutions/line-by-line.c
	$(CC) $(CFLAGS) -o $@ $<

.PHONY: clean
clean:
	rm -f ${OUTDIR}/*
