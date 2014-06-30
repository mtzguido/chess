.PHONY:clean all re run doc
CFLAGS=-Wall -Wextra -Wno-unused-parameter $(CFLAGS_EXTRA)
LFLAGS=
SHELL=/bin/bash
TARGET=chess

ifeq (${V},1)
	Q=
else
	Q = @
endif

SAY = echo

.config:
	@echo USING DEFAULT CONFIG
	./scripts/defconfig

include .config

ifeq (${CONFIG_PROFOPT},y)
	CFLAGS += -fprofile-generate -fprofile-use
	LFLAGS += -fprofile-generate
endif

ifeq (${CONFIG_OWNMEM},y)
	CFLAGS += -DCFG_OWNMEM
endif

ifeq (${CONFIG_RELEASE},y)
	CFLAGS += -O99 -DNDEBUG
else
	CFLAGS += -g -pg
	LFLAGS += -pg
endif


ifeq (${CONFIG_SUGGEST},y)
	CFLAGS += -DCFG_SUGG
endif

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
	addon_trivial	\
	addon	\
	common	\
	user_input	\
	piece-square

objs=$(patsubst %,%.o,$(mods))

all: $(TARGET)

$(TARGET): $(objs)
	$(Q)$(SAY) "LD	$@"
	$(Q)$(CC) $(LFLAGS) $(objs) -o $(TARGET)

%.o: %.c $(wildcard *.h) .config
	$(Q)$(SAY) "CC	$@"
	$(Q)$(CC) $(CFLAGS) -c $<	-o $@

%.s: %.c $(wildcard *.h) .config
	$(Q)$(SAY) "AS	$@"
	$(Q)$(CC) $(CFLAGS) -S $<	-o $@

%.i: %.c $(wildcard *.h) .config
	$(Q)$(SAY) "CPP	$@"
	$(Q)$(CC) $(CFLAGS) -E $<	-o $@

clean:
	$(Q)$(SAY) "CLEAN"
	$(Q)rm -f $(TARGET) $(objs) gmon.out
	$(Q)rm -f bpipe wpipe
	$(Q)$(MAKE) -s -C doc clean
	$(Q)rm -f FINISHLOG full_log gamelog_*

re: clean all

run: $(TARGET)
	./$(TARGET)

doc:
	$(Q)$(SAY) "DOC     "
	$(Q)$(MAKE) -s -C doc
