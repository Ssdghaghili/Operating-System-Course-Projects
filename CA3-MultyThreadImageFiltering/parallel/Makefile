CXX     := g++
CXXFLAG := -std=c++11 -ggdb
INC     := $(shell pkg-config --cflags opencv4)
INCFLAG := $(shell pkg-config --libs opencv4)
LIB     := -lpthread

all: ImageFilters.out

ImageFilters.out: main.cpp
	$(CXX) $(CXXFLAG) $(INC) $< -o $@ $(INCFLAG)

clean:
	rm -f *.out