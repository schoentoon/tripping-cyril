override CFLAGS += -g -Wall -O0 -pipe $(shell gtest-config --cxxflags)
INC             += -I../include
LDFLAGS         := $(shell gtest-config --ldflags --libs)
CC              := gcc
CXX             := g++

BINARY := test.out
DEPS := String.o

all: $(BINARY)

%.o: %.cpp
	$(CXX) $(CFLAGS) $(DEFINES) $(INC) -c $< -o $@

$(BINARY): $(DEPS)
	$(CXX) $(CFLAGS) $(DEFINES) $(INC) -o $(BINARY) main.cpp $(DEPS) ../build/*.o $(LDFLAGS)

.PHONY: test

test: $(BINARY)
	./$(BINARY)

.PHONY: clean

clean:
	rm -rf $(BINARY) $(DEPS)