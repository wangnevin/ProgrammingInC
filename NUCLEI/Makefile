CC      := clang
DEBUG   := -g3
OPTIM   := -O3
CFLAGS  := -Wall -Wextra -Wpedantic -Wfloat-equal -Wvla -std=c99 -Werror
RELEASE := $(CFLAGS) $(OPTIM)
SANI    := $(CFLAGS) -fsanitize=undefined -fsanitize=address $(DEBUG)
VALG    := $(CFLAGS)  $(DEBUG)
NCLS    := $(wildcard *.ncl)
PRES := $(NCLS:.ncl=.pres)
IRES := $(NCLS:.ncl=.ires)
LIBS    := -lm


parse: nuclei.c general.c nuclei.h readfile.c readfile.h tokens_table.h driver.c
	$(CC) nuclei.c general.c readfile.c driver.c $(RELEASE) -o parse $(LIBS)

parse_s: nuclei.c general.c nuclei.h readfile.c readfile.h tokens_table.h driver.c
	$(CC) nuclei.c general.c readfile.c driver.c $(SANI) -o parse_s $(LIBS)

parse_v: nuclei.c general.c nuclei.h readfile.c readfile.h tokens_table.h driver.c
	$(CC) nuclei.c general.c readfile.c driver.c $(VALG) -o parse_v $(LIBS)

all: parse parse_v parse_s interp interp_v interp_s

interp: nuclei.c linked.c general.c nuclei.h lisp.h general.h readfile.c readfile.h tokens_table.h driver.c
	$(CC) nuclei.c linked.c general.c readfile.c driver.c $(RELEASE) -DINTERP -o interp $(LIBS)

interp_s: nuclei.c linked.c general.c nuclei.h lisp.h general.h readfile.c readfile.h tokens_table.h driver.c
	$(CC) nuclei.c linked.c general.c readfile.c driver.c $(SANI) -DINTERP -o interp_s $(LIBS)

interp_v: nuclei.c linked.c general.c nuclei.h lisp.h general.h readfile.c readfile.h tokens_table.h driver.c
	$(CC) nuclei.c linked.c general.c readfile.c driver.c $(VALG) -DINTERP -o interp_v $(LIBS)

# For all .ncl files, run them and output result to a .pres (prase result) 
# or .ires (interpretted result) file.
runall : ./parse_s ./interp_s $(PRES) $(IRES)

parsertest: 
	./parse_s test > test_results.txt
interptest:
	./interp_s test >> test_results.txt

runtest: parsertest interptest

%.pres:
	-./parse_s  $*.ncl > $*.pres 2>&1
%.ires:
	-./interp_s $*.ncl > $*.ires 2>&1

clean:
	rm -f parse parse_s parse_v interp interp_s interp_v $(PRES) $(IRES)
