.PHONY:clean all doc autoversion.c
CFLAGS=-Wc++-compat -Wall -Wextra -Wno-unused-parameter -fprofile-use $(CFLAGS_EXTRA)
LFLAGS=-fprofile-use
HOSTCFLAGS=-Wc++-compat -Wall -Wextra -Wno-unused-parameter $(CFLAGS_EXTRA)
HOSTLFLAGS=-g
SHELL=/bin/bash
TARGET=ice
HOSTCC?=gcc

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

MODS =
ifeq (${CONFIG_PROFOPT},y)
	CFLAGS += -fprofile-generate
	LFLAGS += -fprofile-generate
	MODS += PO
endif

ifeq (${CONFIG_FIXOPTS},y)
	CFLAGS += -DFIXOPTS
	MODS += FO
endif

ifeq (${CONFIG_FLIPBIT},y)
	CFLAGS += -DFLIPBIT

	# mask-gen cares about this one
	HOSTCFLAGS += -DFLIPBIT
	MODS += FB
endif

ifeq (${CONFIG_DEBUG},y)
	MODS += D
else
	CFLAGS += -DNDEBUG
endif

ifeq (${CONFIG_RELEASE},y)
	CFLAGS += -O99 -flto
	LFLAGS += -flto=4 -fwhole-program
	MODS += R
else
	CFLAGS += -g -pg
	LFLAGS += -pg
endif

CFLAGS += -DCFG_ZTABLE_SIZE=${CONFIG_ZTABLE_SIZE}
HOSTCFLAGS += -DCFG_ZTABLE_SIZE=${CONFIG_ZTABLE_SIZE}
CFLAGS += -DCFG_TTABLE_SIZE=${CONFIG_TTABLE_SIZE}

TAG = $(shell git describe --dirty --tags)
MODS_STRING = $(if $(MODS),($(shell echo $(MODS) | sed 's/ /,/g')),)
CFLAGS += -DCHESS_VERSION='"$(TAG)$(MODS_STRING)"'
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
book.gen: book.txt tools/book-gen
	$(Q)$(SAY) "BOOKGEN"
	$(Q)./tools/book-gen < book.txt > book.gen || (rm -f book.gen && false)

tools:
	$(Q)mkdir tools

tools/%.o: %.c .config | tools
	$(Q)$(SAY) "HOSTCC	$@"
	$(Q)$(HOSTCC) -c $(HOSTCFLAGS) $< -o $@

tools/book-gen: $(patsubst %, tools/%, book-gen.o board.o masks.o common.o \
				       piece-square.o succs.o zobrist.o \
				       ztable.o moves.o legal.o check.o \
				       masks.o)
	$(Q)$(SAY) "HOSTLD	$@"
	$(Q)$(HOSTCC) $(HOSTLFLAGS) $^	-o $@

masks.c: tools/mask-gen
	$(Q)$(SAY) "MASKGEN"
	$(Q)./tools/mask-gen > masks.c

tools/mask-gen: tools/mask-gen.o
	$(Q)$(SAY) "HOSTLD	$@"
	$(Q)$(HOSTCC) $(HOSTLFLAGS) $<	-o $@

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
	$(Q)rm -rf tools/
	$(Q)$(MAKE) -s -C doc clean
	$(Q)rm -f .deps

doc:
	$(Q)$(SAY) "  DOC"
	$(Q)$(MAKE) -s -C doc

.deps:
	$(Q)$(SAY) "DEPS"
	$(Q)$(CC) -MM $(patsubst %,%.c,$(utils) $(mods)) > .deps

ifneq ($(MAKECMDGOALS),clean)
-include .deps
endif
