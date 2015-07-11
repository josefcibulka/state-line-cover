CXX=g++
CXXFLAGS=-W -Wall -std=c++11 -pedantic -O2
LIBS=-lkmlbase -lkmldom -lkmlengine
INCLUDES=-I/usr/include

HEADERS=loader.h line_finder.h max_set_list.h

SRCS1=collifinder-main.cc loader.cc line_finder.cc max_set_list.cc
OBJS1 = $(SRCS1:.cc=.o)
MAIN1=collifinder

SRCS2=set_coverer-main.cc max_set_list.cc line_finder.cc
OBJS2 = $(SRCS2:.cc=.o)
MAIN2=set_coverer

SRCS3=combiner-main.cc
OBJS3 = $(SRCS3:.cc=.o)
MAIN3=combiner

all: $(MAIN1) $(MAIN2) $(MAIN3)  

clean:
	rm -rf $(MAIN1) $(MAIN2) $(MAIN3) *.o

$(MAIN1): $(OBJS1)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $^ -o $@ $(LFLAGS) $(LIBS)

$(MAIN2): $(OBJS2)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $^ -o $@ $(LFLAGS) $(LIBS)

$(MAIN3): $(OBJS3)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $^ -o $@ $(LFLAGS) $(LIBS)

%.o: %.cc $(HEADERS)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@ 