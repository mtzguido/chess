.PHONY:clean all re run doc
CFLAGS=-Wc++-compat -Wall -Wextra -Wno-unused-parameter $(CFLAGS_EXTRA)
LFLAGS=
LFLAGS_UTILS=
SHELL=/bin/bash
TARGET=ice

ifeq (${V},1)
	Q=
else
	Q=@
endif

SAY = echo

.config:
	$(Q)$(SAY) "DEFCONFIG"
	$(Q)./scripts/defconfig

-include .config

ifeq (${CONFIG_PROFOPT},y)
	CFLAGS += -fprofile-generate -fprofile-use
	LFLAGS += -fprofile-generate
endif

ifeq (${CONFIG_FIXOPTS},y)
	CFLAGS += -DFIXOPTS
endif

ifeq (${CONFIG_DEBUG},y)
else
	CFLAGS += -DNDEBUG
endif

ifeq (${CONFIG_RELEASE},y)
	CFLAGS += -O99 -flto
	LFLAGS += -flto=4 -fwhole-program
else
	CFLAGS += -g -pg
	LFLAGS += -pg
endif

CFLAGS += -DCFG_ZTABLE_SIZE=${CONFIG_ZTABLE_SIZE}
CFLAGS += -DCFG_TTABLE_SIZE=${CONFIG_TTABLE_SIZE}
CFLAGS += -DCHESS_VERSION='"$(shell git describe --dirty --tags)"'
CFLAGS += -DCHESS_BUILD_DATE='"$(shell date)"'
CFLAGS += -DCHESS_BUILD_HOST='"$(shell hostname)"'

mods=	ai	\
	search	\
	board	\
	check	\
	eval	\
	legal	\
	moves	\
	succs	\
	pgn	\
	ztable	\
	zobrist	\
	autoversion \
	addon_trans	\
	addon_killer	\
	addon_cm	\
	addon_trivial	\
	addon	\
	common	\
	user_input	\
	piece-square	\
	main

automods=	book	\
		masks

utils=	mask-gen	\
	book-gen

ifeq (${CONFIG_FIXOPTS},y)
else
	mods += opts
endif
objs=$(patsubst %,%.o,$(mods) $(automods))

$(TARGET): $(objs) .config
	$(Q)$(SAY) "  LD	$@"
	$(Q)$(CC) $(LFLAGS) $(objs) -o $(TARGET)

all: $(TARGET) doc

book.o: book.gen
book.gen: book.txt book-gen
	$(Q)$(SAY) "BOOKGEN"
	$(Q)./book-gen < book.txt > book.gen || (rm -f book.gen && false)

book-gen: book-gen.o board.o masks.o common.o piece-square.o succs.o \
	  zobrist.o ztable.o moves.o legal.o check.o
	$(Q)$(SAY) "  LD	$@"
	$(Q)$(CC) $(LFLAGS_UTILS) $^	-o $@

masks.c: mask-gen
	$(Q)$(SAY) "MASKGEN"
	$(Q)./mask-gen > masks.c

mask-gen: mask-gen.o
	$(Q)$(SAY) "  LD	$@"
	$(Q)$(CC) $(LFLAGS_UTILS) $<	-o $@

%.o: %.c .config
	$(Q)$(SAY) "  CC	$@"
	$(Q)$(CC) $(CFLAGS) -c $<	-o $@

%.s: %.c .config
	$(Q)$(SAY) "  AS	$@"
	$(Q)$(CC) $(CFLAGS) -S -fverbose-asm $<	-o $@

%.i: %.c .config
	$(Q)$(SAY) "  CPP	$@"
	$(Q)$(CC) $(CFLAGS) -E $<	-o $@

clean:
	$(Q)$(SAY) "CLEAN"
	$(Q)rm -f $(TARGET) gmon.out
	$(Q)rm -f book.gen book-gen mask-gen masks.c
	$(Q)rm -f $(patsubst %,%.o, $(automods))
	$(Q)rm -f $(patsubst %,%.i, $(automods))
	$(Q)rm -f $(patsubst %,%.s, $(automods))
	$(Q)rm -f $(patsubst %,%.o, $(mods))
	$(Q)rm -f $(patsubst %,%.i, $(mods))
	$(Q)rm -f $(patsubst %,%.s, $(mods))
	$(Q)rm -f $(patsubst %,%.o, $(utils))
	$(Q)rm -f $(patsubst %,%.i, $(utils))
	$(Q)rm -f $(patsubst %,%.s, $(utils))
	$(Q)$(MAKE) -s -C doc clean
	$(Q)rm -f .deps

re: clean $(TARGET)

doc:
	$(Q)$(SAY) "  DOC"
	$(Q)$(MAKE) -s -C doc

.deps: $(patsubst %,%.c,$(mods))
	$(Q)$(SAY) "DEPS"
	$(Q)$(CC) -MM $(patsubst %,%.c,$(utils) $(mods)) > .deps

ifneq ($(MAKECMDGOALS),clean)
-include .deps
endif
