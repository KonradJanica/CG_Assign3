CC = g++

all: tiny_obj_loader.cc tiny_obj_loader.o

tiny_obj_loader.o : tiny_obj_loader.h
	$(CC) -c tiny_obj_loader.cc
	
clean:
	rm -rf *.o
