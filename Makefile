#!/usr/bin/make -f
# === config ===

CC = gcc
BIN_NAME = bvchess
srcdir = ./src
includedir = ./include
builddir = build
DEFS =

CDEBUG = -g -Wall -Wextra -O3
CFLAGS = $(CDEBUG) -I$(includedir) -I$(srcdir) $(DEFS) -I/usr/include/SDL2 $(shell pkg-config --cflags sdl2) -lSDL2_image
LDFLAGS = -g $(shell pkg-config --libs sdl2) -lSDL2_image

OBJS = \
	$(builddir)/cli.o \
	$(builddir)/gui.o \
	$(builddir)/logging.o \
	$(builddir)/attack_update.o \
	$(builddir)/engine.o \
	$(builddir)/test.o \
	$(builddir)/util.o \
	$(builddir)/main.o

OBJSDIR = $(addprefix $(builddir)/,$(OBJS))

all: $(builddir) $(OBJS) cp_textures
	$(CC) -o $(builddir)/$(BIN_NAME) $(OBJS) $(LDFLAGS)

$(builddir):
	mkdir -p $(builddir)

$(builddir)/%.o: $(srcdir)/%.c | $(builddir)
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY : 	clean run debug debug_all valgrind valgrind_all \
		rm_lof_files konsole konsole_all clear debug_all_konsole

clean:
	rm -f $(builddir)/*

run: all
	SDL_VIDEODRIVER=x11 ./build/$(BIN_NAME)

debug_all: clean all debug

debug:
	gdb ./build/$(BIN_NAME)

debug_all_konsole: clean all
	konsole -e gdb ./build/$(BIN_NAME)

valgrind_all_konsole: clean all
	konsole -e valgrind ./build/$(BIN_NAME)

valgrind_all: clean all valgrind

valgrind:
	valgrind ./build/$(BIN_NAME)

rm_log_files:
	rm -f ~/.local/share/bvchess/*.log

konsole:
	konsole -e ./build/$(BIN_NAME)

konsole_all: all
	konsole -e SDL_VIDEODRIVER=x11 ./build/$(BIN_NAME)
konsole_all_gui: all
	konsole -e SDL_VIDEODRIVER=x11 ./build/$(BIN_NAME) gui
konsole_all_cli: all
	konsole -e ./build/$(BIN_NAME) cli
clear:
	clear

cp_textures:
	cp Textures/* ~/.local/share/bvchess/
