.PHONY:clean all re run
CFLAGS= -Wall -pg -g -funroll-loops $(CFLAGS_EXTRA)
LFLAGS= -pg 
SHELL=/bin/bash
TARGET=chess
CC=gcc

mods=main ai board move succs
objs=$(patsubst %,%.o,$(mods))

all: $(TARGET)

$(TARGET): $(objs)
	$(CC) $(LFLAGS) $(objs) -o $(TARGET)

%.o: %.c $(wildcard *.h)
	$(CC) $(CFLAGS) -c $<	-o $@

clean:
	rm -f $(TARGET) $(objs) gmon.out

re: clean all

run: $(TARGET)
	./$(TARGET)

prof: $(TARGET) prof_input
	./$(TARGET) < prof_input # >/dev/null
	gprof $(TARGET) gmon.out >prof

vprof: prof
	gprof2dot.py prof | xdot
