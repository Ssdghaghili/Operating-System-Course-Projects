CXX     := g++
CXXFLAG := -std=c++11 -ggdb
INC     := $(shell pkg-config --cflags opencv4)
LIB     := $(shell pkg-config --libs opencv4)

all: ImageFilters.out

ImageFilters.out: main.cpp
	$(CXX) $(CXXFLAG) $(INC) $< -o $@ $(LIB)

clean:
	rm -f *.out