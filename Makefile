XC_HOST := x86_64-w64-mingw32
CXX := $(XC_HOST)-g++
CC := $(XC_HOST)-gcc

CFLAGS += -I./inc -g
CXXFLAGS += -I./inc -g

JL_SHARE = $(shell julia -e 'print(joinpath(JULIA_HOME,Base.DATAROOTDIR,"julia"))')
CFLAGS += $(shell $(JL_SHARE)/julia-config.jl --cflags)
CXXFLAGS += $(shell $(JL_SHARE)/julia-config.jl --cflags)
LDFLAGS += $(shell $(JL_SHARE)/julia-config.jl --ldflags)
LDLIBS += -ljulia-debug -lopenlibm

CPPS := $(wildcard src/*.cpp)
OBJS := $(CPPS:%.cpp=%.o)

.PHONY : all clean

all : libjets.dll jets_test

libjets.dll : $(OBJS)
	$(CXX) -shared $(LDFLAGS) $(LDLIBS) -o $@ $(OBJS)

jets_test : test/jets_test.o libjets.dll
	$(CXX) $(LDFLAGS) $(LDLIBS) -o $@ $^

clean:
	-rm $(OBJS) test/jets_test.o libjets.dll jets_test
