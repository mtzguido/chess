.PHONY:clean all re run
CFLAGS=-Wall -g -pg
LFLAGS= -pg -static
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
