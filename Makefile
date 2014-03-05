.PHONY:clean all re run
CFLAGS=-Wall -g -pg -O99
LFLAGS= -pg -static -O99
SHELL=/bin/bash
TARGET=chess
CC=gcc

mods=main ai board move
objs=$(patsubst %,%.o,$(mods))

all: $(TARGET)

$(TARGET): $(objs)
	$(CC) $(LFLAGS) $(objs) -o $(TARGET)

%.o: %.c $(wildcard *.h)
	$(CC) $(CFLAGS) -c $<	-o $@

clean:
	rm -f $(TARGET) $(objs)

re: clean all

run: $(TARGET)
	./$(TARGET)

prof: $(TARGET)
	$(TARGET) < prof_input
	gprof $(TARGET) gmon.out >prof
