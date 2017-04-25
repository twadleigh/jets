XC_HOST := x86_64-w64-mingw32
CXX := $(XC_HOST)-g++

JULIA_DIR = /cygdrive/c/tw/Julia-0.6.0-pre.beta
INCLUDE_FLAGS := -I./inc -I$(JULIA_DIR)/include -I$(JULIA_DIR)/include/julia
LINK_FLAGS := -L$(JULIA_DIR)/bin -ljulia

CPPS := $(wildcard src/*.cpp)
OBJS := $(CPPS:%.cpp=%.o)

.PHONY : all

all : libjlts.dll jlts_test

libjlts.dll : $(OBJS)
	$(CXX) -shared $(LINK_FLAGS) -o $@ $(OBJS)

jlts_test : test/jlts_test.o libjlts.dll
	$(CXX) -L. -ljlts -o $@ $<

%.o : %.cpp
	$(CXX) -shared $(INCLUDE_FLAGS) -c -o $@ $^
