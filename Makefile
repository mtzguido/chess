.PHONY:clean all re run doc
CFLAGS= -Wall -pg -g -funroll-loops $(CFLAGS_EXTRA)
LFLAGS= -pg
SHELL=/bin/bash
TARGET=chess
CC=gcc

CONFIG_KILLER=y
CONFIG_COUNTERMOVE=y
CONFIG_EXTEND=y
CONFIG_TRANSPOSITION=n

ifeq (${CONFIG_KILLER},y)
	CFLAGS += -DCFG_KILLER
endif

ifeq (${CONFIG_COUNTERMOVE},y)
	CFLAGS += -DCFG_COUNTERMOVE
endif

ifeq (${CONFIG_EXTEND},y)
	CFLAGS += -DCFG_DEPTH_EXTENSION
endif

ifeq (${CONFIG_TRANSPOSITION},y)
	CFLAGS += -DCFG_TRANSPOSITION
endif

mods=main ai board move succs pgn
objs=$(patsubst %,%.o,$(mods))

all: $(TARGET)

$(TARGET): $(objs)
	$(CC) $(LFLAGS) $(objs) -o $(TARGET)

%.o: %.c $(wildcard *.h) Makefile
	$(CC) $(CFLAGS) -c $<	-o $@

clean:
	rm -f $(TARGET) $(objs) gmon.out
	rm -f bpipe wpipe
	$(MAKE) -C doc clean
	rm -f FINISHLOG full_log gamelog_*

re: clean all

run: $(TARGET)
	./$(TARGET)

prof: $(TARGET) prof_input
	time ./$(TARGET) w < prof_input || true # >/dev/null
	gprof $(TARGET) gmon.out >prof

vprof: prof
	gprof2dot.py prof | xdot

test: | $(TARGET)
	./scripts/make_tests.sh

doc:
	$(MAKE) -C doc
