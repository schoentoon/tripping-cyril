include ../make.conf

PKGS              += libevent libevent_openssl openssl libconfig++
override CXXFLAGS += -g -Wall -O0 -pipe -Wno-sign-compare -D_TEST $(shell pkg-config --cflags $(PKGS))
INC               += -I../include -I. -Igtest-1.7.0/include
LDFLAGS           := -Wl,--export-dynamic $(shell pkg-config --libs $(PKGS)) -ldl -pthread
CC                := gcc
CXX               := g++

BINARY := test.out
MODS := dummymod.so
DEPS := $(patsubst %_test.cpp, %_test.o, $(shell find .. -name \*_test.cpp -type f))

all: $(BINARY)

%.o: %.cpp libgtest.a
	$(CXX) $(CXXFLAGS) $(DEFINES) $(INC) -c $< -o $@

%.so: %.cpp libgtest.a
	$(CXX) $(CXXFLAGS) $(DEFINES) $(INC) -shared -fPIC -Wl,--export-dynamic -o $@ $<

$(BINARY): $(DEPS) $(MODS) main.cpp
	$(CXX) $(CXXFLAGS) $(DEFINES) $(INC) -o $(BINARY) main.cpp $(DEPS) ../build/*.o libgtest.a -pthread $(LDFLAGS)

libgtest.a:
	wget -q https://googletest.googlecode.com/files/gtest-1.7.0.zip
	unzip -qq gtest-1.7.0.zip
	$(CXX) -I gtest-1.7.0/include -I gtest-1.7.0 -c gtest-1.7.0/src/gtest-all.cc
	ar -rv libgtest.a gtest-all.o

.PHONY: test

test: $(BINARY)
	./$(BINARY)

.PHONY: clean

clean:
	rm -rf $(BINARY) $(DEPS) $(MODS)