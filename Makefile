CXX=g++
CXXFLAGS=-W -Wall -std=c++11 -pedantic
LIBS=-lkmlbase -lkmldom -lkmlengine
INCLUDES=-I/usr/include

SRCS=main.cc loader.cc line_finder.cc
HEADERS=loader.h line_finder.h
OBJS = $(SRCS:.cc=.o)

MAIN=cover

all: $(MAIN)

clean:
	rm -rf $(MAIN) $(OBJS)

$(MAIN): $(OBJS)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $(MAIN) $(OBJS) $(LFLAGS) $(LIBS)

%.o: %.cc $(HEADERS)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@ 