DEPS := module.o

CUSTOM_TARGETS := submodule_a.so submodule_b.so
CUSTOM_CLEAN := $(CUSTOM_TARGETS)

include ../Makefile.flags

submodule_a.so: submodule_a.cpp
	$(CXX) $(CXXFLAGS) $(DEFINES) $(LDFLAGS) $(MODLINK) $(INC) -o submodule_a.so submodule_a.cpp

submodule_b.so: submodule_b.cpp
	$(CXX) $(CXXFLAGS) $(DEFINES) $(LDFLAGS) $(MODLINK) $(INC) -o submodule_b.so submodule_b.cpp