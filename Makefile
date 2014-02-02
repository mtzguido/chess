.PHONY:clean all re run
CFLAGS=-Wall -g
SHELL=/bin/bash
TARGET=chess

mods=main board moves ai
objs=$(patsubst %,%.o,$(mods))

all: $(TARGET)

$(TARGET): $(objs)
	gcc $(objs) -o $(TARGET)

%.o: %.c $(wildcard *.h)
	gcc ${CFLAGS} -c $<	-o $@

clean:
	rm -f $(TARGET) $(objs)

re: clean all

run: $(TARGET)
	./$(TARGET)
