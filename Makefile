override CFLAGS += -g -Wall -O2 -pipe
INC             += -Iinclude
CC              := gcc
CXX             := g++

BINARY := tripping-cyril
DEPS := build/String.o build/Global.o

all: $(BINARY)

build:
	mkdir build

build/%.o: src/%.cpp include/%.h build
	$(CXX) $(CFLAGS) $(DEFINES) $(INC) -c $< -o $@

$(BINARY): $(DEPS)
	$(CXX) $(CFLAGS) $(DEFINES) $(INC) -o $(BINARY) main.cpp $(DEPS)

.PHONY: test

test: $(BINARY)
	$(MAKE) -C test test

.PHONY: clean

clean:
	rm -rf $(BINARY) build
	$(MAKE) -C test clean