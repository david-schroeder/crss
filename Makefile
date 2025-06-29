CFLAGS		:= -Wall -g -O0 -D DEBUG -Igl/include -Ilib -lGL -lglfw -lm -lX11 -lXrandr -lpthread -ldl
CC			:= clang
OBJFILES    := build/rss-server.o \
				build/version.o \
				build/utils.o \
				build/settings.o \
				build/gui/window.o \
				build/gui/text.o \
				build/core/redsynth/graph.o

VERSION     := v1.0-snapshot-0 # must be accordingly updated
NAME        := rss-server-$(VERSION)

run: $(OBJFILES) | build
	@$(CC) $(CFLAGS) -o build/$(NAME) $^
	@./build/$(NAME) -d verbose

build:
	mkdir -p $@

build/%.o: src/%.c | build
	@mkdir -p $(@D)
	@$(CC) $(CFLAGS) -c -o $@ $^

clean:
	rm -rf build
