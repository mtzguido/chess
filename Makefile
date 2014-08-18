.PHONY:clean all re run doc
CFLAGS=-Wall -Wextra -Wno-unused-parameter $(CFLAGS_EXTRA)
LFLAGS=-lm
LFLAGS_UTILS=
SHELL=/bin/bash
TARGET=chess

ifeq (${V},1)
	Q=
else
	Q=@
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

ifeq (${CONFIG_DEBUG},y)
else
	CFLAGS += -DNDEBUG
endif

ifeq (${CONFIG_RELEASE},y)
	CFLAGS += -O99 -flto
	LFLAGS += -flto
else
	CFLAGS += -g -pg
	LFLAGS += -pg
endif

CFLAGS += -DCFG_MEMSZ=${CONFIG_MEMSZ}
CFLAGS += -DCFG_ZTABLE_SIZE=${CONFIG_ZTABLE_SIZE}
CFLAGS += -DCFG_TTABLE_SIZE=${CONFIG_TTABLE_SIZE}

mods=	ai	\
	board	\
	eval	\
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
	piece-square	\
	book	\
	masks

objs=$(patsubst %,%.o,$(mods))
crap=$(patsubst %,%.i %.s,$(mods))

$(TARGET): main.o $(objs)
	$(Q)$(SAY) "LD	$@"
	$(Q)$(CC) $(LFLAGS) main.o $(objs) -o $(TARGET)

all: $(TARGET) doc

book.o: book.gen
book.gen: book.txt book-gen
	$(Q)$(SAY) "BOOKGEN"
	$(Q)./book-gen < book.txt > book.gen || (rm -f book.gen && false)

masks.c: mask-gen
	$(Q)$(SAY) "MASKGEN"
	$(Q)./mask-gen > masks.c

mask-gen.o: mask-gen.c
mask-gen: mask-gen.o
	$(Q)$(SAY) "LD	$@"
	$(Q)$(CC) $(LFLAGS_UTILS) $<	-o $@

%.o: %.c $(wildcard *.h) .config
	$(Q)$(SAY) "CC	$@"
	$(Q)$(CC) $(CFLAGS) -c $<	-o $@

%.s: %.c $(wildcard *.h) .config
	$(Q)$(SAY) "AS	$@"
	$(Q)$(CC) $(CFLAGS) -S -fverbose-asm $<	-o $@

%.i: %.c $(wildcard *.h) .config
	$(Q)$(SAY) "CPP	$@"
	$(Q)$(CC) $(CFLAGS) -E $<	-o $@

book-gen: book-gen.o board.o zobrist.o move.o piece-square.o masks.o \
	  mem.o succs.o common.o ztable.o
	$(Q)$(SAY) "LD	$@"
	$(Q)$(CC) $(LFLAGS_UTILS) $^ -o $@

clean:
	$(Q)$(SAY) "CLEAN"
	$(Q)rm -f $(TARGET) $(crap) gmon.out
	$(Q)rm -f main.o book.gen *.o book-gen
	$(Q)rm -f bpipe wpipe
	$(Q)$(MAKE) -s -C doc clean
	$(Q)rm -f FINISHLOG gamelog_*
	$(Q)rm -f masks.c mask-gen

re: clean $(TARGET)

doc:
	$(Q)$(SAY) "DOC     "
	$(Q)$(MAKE) -s -C doc
