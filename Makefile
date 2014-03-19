include ./make.conf

PKGS            += libevent libevent_openssl openssl
override CFLAGS += -g -Wall -O2 -pipe $(shell pkg-config --cflags $(PKGS))
INC             += -Iinclude
LDFLAGS         := -Wl,--export-dynamic $(shell pkg-config --libs $(PKGS)) -ldl
CC              := gcc
CXX             := g++

BINARY := tripping-cyril
DEPS := build/String.o build/Global.o build/Socket.o build/Files.o build/Timer.o \
build/Module.o build/SimpleHTTPSocket.o build/TermUtils.o build/StackTrace.o \
build/Thread.o build/Pipe.o build/JobThread.o build/Listener.o build/Postgres.o \
build/Compressor.o build/Decompressor.o build/Event.o build/StatsdClient.o build/Writer.o

all: $(BINARY) modules

build:
	mkdir build

build/%.o: src/%.cpp include/%.h
	$(CXX) $(CFLAGS) $(DEFINES) $(INC) -c $< -o $@

$(BINARY): build $(DEPS)
	$(CXX) $(CFLAGS) $(DEFINES) $(INC) -o $(BINARY) main.cpp $(DEPS) $(LDFLAGS)

.PHONY: modules

modules:
	$(MAKE) -C modules

.PHONY: test

test: $(BINARY)
	$(MAKE) -C test test

.PHONY: clean

clean:
	rm -rf $(BINARY) build
	$(MAKE) -C modules clean
	$(MAKE) -C test clean

doc: include/*.h
	doxygen