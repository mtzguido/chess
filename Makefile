.PHONY:clean all re run doc
CFLAGS= -Wall -pg -g -funroll-loops -DNDEBUG -O99 $(CFLAGS_EXTRA)
LFLAGS= -pg 
SHELL=/bin/bash
TARGET=chess
CC=gcc

mods=main ai board move succs pgn piece-square
objs=$(patsubst %,%.o,$(mods))

all: $(TARGET) doc

$(TARGET): $(objs)
	$(CC) $(LFLAGS) $(objs) -o $(TARGET)

%.o: %.c $(wildcard *.h)
	$(CC) $(CFLAGS) -c $<	-o $@

clean:
	rm -f $(TARGET) $(objs) gmon.out
	rm -f bpipe wpipe
	$(MAKE) -C doc clean

re: clean all

run: $(TARGET)
	./$(TARGET)

prof: $(TARGET) prof_input
	./$(TARGET) < prof_input # >/dev/null
	gprof $(TARGET) gmon.out >prof

vprof: prof
	gprof2dot.py prof | xdot

test: | $(TARGET)
	./scripts/make_tests.sh

doc:
	$(MAKE) -C doc
