.PHONY:clean all re run
CFLAGS=-Wall -pedantic -g
SHELL=/bin/bash
TARGET=ai

mods=main board
objs=$(patsubst %,%.o,$(mods))

all: $(TARGET)

$(TARGET): $(objs)
	gcc $(objs) -o $(TARGET)

%.o: %.c $(wildcard *.h)
	gcc ${CFLAGS} -c $<	-o $@

clean:
	rm -f $(TARGET) gen rand $(objs)

re: clean all

run: $(TARGET)
	./$(TARGET)
