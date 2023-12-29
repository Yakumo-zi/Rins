CXX=g++
CFLAGS=-g -O2 -Wall -fPIC -Wno-deprecated 

INC=-I../../include
LIB=-L../../lib -lreactor
OBJS = $(addsuffix .o, $(basename $(wildcard *.cc)))

all:
	$(CXX) -o test $(CFLAGS)  main.cc $(INC) $(LIB) -lpthread

clean:
	-rm -f *.o test