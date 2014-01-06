PKGS            := libevent libevent_openssl openssl
override CFLAGS += -g -Wall -O0 -pipe -Wno-sign-compare $(shell pkg-config --cflags $(PKGS))
INC             += -I../include -Igtest-1.7.0/include
LDFLAGS         := $(shell pkg-config --libs $(PKGS))
CC              := gcc
CXX             := g++

BINARY := test.out
DEPS := String.o Files.o

all: $(BINARY)

%.o: %.cpp libgtest.a
	$(CXX) $(CFLAGS) $(DEFINES) $(INC) -c $< -o $@

$(BINARY): $(DEPS)
	$(CXX) $(CFLAGS) $(DEFINES) $(INC) -o $(BINARY) main.cpp $(DEPS) ../build/*.o libgtest.a -pthread $(LDFLAGS)

libgtest.a:
	wget -q http://googletest.googlecode.com/files/gtest-1.7.0.zip
	unzip -qq gtest-1.7.0.zip
	$(CXX) -I gtest-1.7.0/include -I gtest-1.7.0 -c gtest-1.7.0/src/gtest-all.cc
	ar -rv libgtest.a gtest-all.o

.PHONY: test

test: $(BINARY)
	./$(BINARY)

.PHONY: clean

clean:
	rm -rf $(BINARY) $(DEPS)