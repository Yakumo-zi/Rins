CXX=g++
CFLAGS=-g -O2 -Wall -fPIC -Wno-deprecated 

INC=-I../../include
LIB=-L../../lib -lreactor -lpthread -lprotobuf

all:
	$(CXX) -o server $(CFLAGS)  server.cc echo_message.pb.cc $(INC) $(LIB)
	$(CXX) -o client $(CFLAGS)  client.cc echo_message.pb.cc $(INC) $(LIB)
clean:
	rm -f *.o server client