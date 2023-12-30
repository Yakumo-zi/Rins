CXX=g++
CFLAGS=-g -O2 -Wall -fPIC -Wno-deprecated 

INC=-I../../include
LIB=-L../../lib -lreactor -lpthread
OBJS = $(addsuffix .o, $(basename $(wildcard *.cc)))

all:
	$(CXX) -o server $(CFLAGS)  server.cc $(INC) $(LIB)
	$(CXX) -o client $(CFLAGS)  client.cc $(INC) $(LIB)

clean:
	-rm -f *.o server client