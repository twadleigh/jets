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
# LDLIBS += $(shell $(JL_SHARE)/julia-config.jl --ldlibs)

CPPS := $(wildcard src/*.cpp)
OBJS := $(CPPS:%.cpp=%.o)

.PHONY : all clean

all : libjlts.dll jlts_test

libjlts.dll : $(OBJS)
	$(CXX) -shared $(LDFLAGS) $(LDLIBS) -o $@ $(OBJS)

jlts_test : test/jlts_test.o libjlts.dll
	$(CXX) $(LDFLAGS) $(LDLIBS) -o $@ $^ 

clean:
	-rm $(OBJS) libjlts.dll jlts_test
