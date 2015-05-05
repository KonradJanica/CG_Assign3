
PLATFORM := $(shell uname)

ifneq (, $(findstring CYGWIN, $(PLATFORM)))
    GL_LIBS = -lopengl32 -lglut -lglu -lglew
	EXT = .exe
    CPPFLAGS =-DWIN32
endif

ifneq (, $(findstring Linux, $(PLATFORM)))
    GL_LIBS = -lGL -lglut -lGLEW 
    GL_LIBS += -L/usr/lib/nvidia-304 # necessary for some NVidia drivers;
	EXT = 
    CPPFLAGS =
endif

ifneq (, $(findstring Darwin, $(PLATFORM)))
    CPPFLAGS   = -DMacOSX -D__APPLE__ -m32 -arch x86_64 
    CPPFLAGS += -Wno-deprecated # if you get sick of deprecated warnings..
    GL_LIBS = -framework GLUT -framework OpenGL -framework CoreFoundation -lGLEW
	EXT = 
endif

CC = g++
LINK = model_data.o shader.o model.o terrain.o camera.o renderer.o main.o

.PHONY:  clean

all : run$(EXT)

run$(EXT) : $(LINK)
	$(CC) $(CPPFLAGS) -o assign2 $(LINK) $(GL_LIBS)

main.o : model_data.h shader.hpp model.h camera.h renderer.h main.cpp 
	$(CC) $(CPPFLAGS) -c main.cpp

renderer.o : renderer.cc renderer.h camera.h terrain.h model.h
	$(CC) $(CPPFLAGS) -c renderer.h renderer.cc

camera.o : camera.cc camera.h
	$(CC) $(CPPFLAGS) -c camera.h camera.cc

terrain.o : terrain.cc terrain.h
	$(CC) $(CPPFLAGS) -c terrain.h terrain.cc

model.o: model.cc model.h model_data.h
	$(CC) $(CPPFLAGS) -c model.h model.cc

shader.o : shader.cpp shader.hpp
	$(CC) $(CPPFLAGS) -c shader.cpp

model_data.o: model_data.cc model_data.h
	$(CC) $(CPPFLAGS) -c model_data.h model_data.cc

clean:
	rm -f *.o assign2$(EXT)
