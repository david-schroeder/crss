CFLAGS		:= -std=c23 -Wall -Werror -Wno-unused-command-line-argument -fsanitize=address -g -O0 -D DEBUG -Ilib -lpthread `pkg-config gtk4 libzmq epoxy --cflags --libs`
CC			:= clang
OBJFILES    := build/rss-server.o \
				build/version.o \
				build/utils.o \
				build/settings.o \
				build/internals.o \
				build/resources.o \
				build/logger/logger.o \
				build/logger/table.o \
				build/logger/help.o \
				build/gui/gui.o \
				build/gui/resources.o \
				build/gui/crss_gl.o \
				build/core/coretypes.o \
				build/core/hashmap.o \
				build/core/core.o \
				build/core/plots.o \
				build/core/plotcommands.o \
				build/core/redsynth/graph.o

VERSION     := v1.0-snapshot-0 # must be accordingly updated
NAME        := rss-server-$(VERSION)

run: $(OBJFILES) | build
	@$(CC) $(CFLAGS) -o build/$(NAME) $^
	@./build/$(NAME) -d verbose

build:
	mkdir -p $@

resources:
	glib-compile-resources src/gui/crss.gresource.xml --generate-source --generate --sourcedir=src/gui --target=src/gui/resources.c

build/%.o: src/%.c | build
	@mkdir -p $(@D)
	@$(CC) $(CFLAGS) -c -o $@ $^

clean:
	rm -rf build
