override CFLAGS += -g -Wall -O0 -pipe
INC             += -I../include -Igtest-1.7.0/include
CC              := gcc
CXX             := g++

BINARY := test.out
DEPS := String.o

all: $(BINARY)

%.o: %.cpp libgtest.a
	$(CXX) $(CFLAGS) $(DEFINES) $(INC) -c $< -o $@

$(BINARY): $(DEPS)
	$(CXX) $(CFLAGS) $(DEFINES) $(INC) -o $(BINARY) main.cpp $(DEPS) ../build/*.o libgtest.a -pthread

libgtest.a:
	wget http://googletest.googlecode.com/files/gtest-1.7.0.zip
	unzip gtest-1.7.0.zip
	$(CXX) -I gtest-1.7.0/include -I gtest-1.7.0 -c gtest-1.7.0/src/gtest-all.cc
	ar -rv libgtest.a gtest-all.o

.PHONY: test

test: $(BINARY)
	./$(BINARY)

.PHONY: clean

clean:
	rm -rf $(BINARY) $(DEPS)