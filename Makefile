.PHONY:clean all re run doc
CFLAGS=-Wall -Wextra -Wno-unused-parameter -funroll-loops $(CFLAGS_EXTRA)
LFLAGS=-lsigsegv
SHELL=/bin/bash
TARGET=chess
CC=gcc

ifeq (${V},1)
	Q=
	SAY= echo
else
	Q = @
	SAY = echo
endif

.config:
	@echo USING DEFAULT CONFIG
	./scripts/defconfig

include .config

ifeq (${CONFIG_ALPHABETA},y)
	CFLAGS += -DCFG_ALPHABETA
endif

ifeq (${CONFIG_KILLER},y)
	CFLAGS += -DCFG_KILLER
endif

ifeq (${CONFIG_COUNTERMOVE},y)
	CFLAGS += -DCFG_COUNTERMOVE
endif

ifeq (${CONFIG_TRANSPOSITION},y)
	CFLAGS += -DCFG_TRANSPOSITION
endif

ifeq (${CONFIG_OWNMEM},y)
	CFLAGS += -DCFG_OWNMEM
endif

ifeq (${CONFIG_RELEASE},y)
	CFLAGS += -O99
endif

ifeq (${CONFIG_SHUFFLE},y)
	CFLAGS += -DCFG_SHUFFLE
endif

ifeq (${CONFIG_PROFILE},y)
	LFLAGS += -pg
	CFLAGS += -pg
endif

ifeq (${CONFIG_DEBUG},y)
	CFLAGS += -g
endif

ifeq (${CONFIG_SUGGEST},y)
	CFLAGS += -DCFG_SUGG
endif

CFLAGS += -DCFG_DEPTH=${CONFIG_DEPTH}
CFLAGS += -DCFG_MEMSZ=${CONFIG_MEMSZ}
CFLAGS += -DCFG_ZTABLE_SIZE=${CONFIG_ZTABLE_SIZE}
CFLAGS += -DCFG_TTABLE_SIZE=${CONFIG_TTABLE_SIZE}

mods=	main	\
	ai	\
	board	\
	move	\
	succs	\
	pgn	\
	mem	\
	ztable	\
	zobrist	\
	addon_trans	\
	addon_killer	\
	addon_cm	\
	addon	\
	common	\
	user_input

objs=$(patsubst %,%.o,$(mods))

all: $(TARGET)

$(TARGET): $(objs)
	$(Q)$(SAY) "LD	$@"
	$(Q) $(CC) $(LFLAGS) $(objs) -o $(TARGET)

%.o: %.c $(wildcard *.h) .config
	$(Q)$(SAY) "CC	$<"
	$(Q)$(CC) $(CFLAGS) -c $<	-o $@

clean:
	$(Q)$(SAY) "CLEAN"
	$(Q)rm -f $(TARGET) $(objs) gmon.out
	$(Q)rm -f bpipe wpipe
	$(Q)$(MAKE) -C doc clean
	$(Q)rm -f FINISHLOG full_log gamelog_*

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
