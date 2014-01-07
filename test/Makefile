PKGS            := libevent libevent_openssl openssl
override CFLAGS += -g -Wall -O0 -pipe -Wno-sign-compare $(shell pkg-config --cflags $(PKGS))
INC             += -I../include -Igtest-1.7.0/include
LDFLAGS         := -Wl,--export-dynamic $(shell pkg-config --libs $(PKGS)) -ldl
CC              := gcc
CXX             := g++

BINARY := test.out
MODS := dummymod.so
DEPS := String.o Files.o Modules.o

all: $(BINARY)

%.o: %.cpp libgtest.a
	$(CXX) $(CFLAGS) $(DEFINES) $(INC) -c $< -o $@

%.so: %.cpp libgtest.a
	$(CXX) $(CFLAGS) $(DEFINES) $(INC) -shared -fPIC -Wl,--export-dynamic -o $@ $<

$(BINARY): $(DEPS) $(MODS)
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
	rm -rf $(BINARY) $(DEPS) $(MODS)