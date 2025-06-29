CFLAGS		:= -Wall -g -O0 -D DEBUG -Ilib
CC			:= clang
OBJFILES    := build/rss-server.o \
				build/version.o \
				build/utils.o \
				build/settings.o \
				build/gui/gui.o \
				build/core/redsynth/graph.o

VERSION     := v1.0-snapshot-0 # must be accordingly updated
NAME        := rss-server-$(VERSION)

run: $(OBJFILES) | build
	@$(CC) $(CFLAGS) `pkg-config gtk4 --cflags` -o build/$(NAME) $^ `pkg-config --libs gtk4`
	@./build/$(NAME) -d verbose

build:
	mkdir -p $@

build/%.o: src/%.c | build
	@mkdir -p $(@D)
	@$(CC) $(CFLAGS) `pkg-config gtk4 --cflags` -c -o $@ $^ `pkg-config --libs gtk4`

clean:
	rm -rf build
