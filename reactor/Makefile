TARGET=lib/libreactor.a
CXX=g++
CFLAGS=-g -O2 -Wall -fPIC -Wno-deprecated 

SRC=./src

INC=-I./include

OBJS= $(addsuffix .o,$(basename $(wildcard $(SRC)/*.cc)))

%.o:%.cc
	$(CXX) $(CFLAGS) -c -o $@ $< $(INC) -lpthread

$(TARGET):$(OBJS)
	mkdir -p lib
	ar cqs $@ $^


.PHONY:clean
clean:
	rm -f src/*.o $(TARGET)
