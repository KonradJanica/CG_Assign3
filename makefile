
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

CC = g++ -std=c++0x
LINK = model_data.o model.o object.o terrain.o camera.o renderer.o light_controller.o controller.o main.o
LIB = lib/tiny_obj_loader/tiny_obj_loader.o lib/shader/shader.o

.PHONY:  clean

all : assign3$(EXT)

assign3$(EXT) : $(LINK) $(LIB)
	$(CC) $(CPPFLAGS) -o assign3 $(LINK) $(LIB) $(GL_LIBS)

main.o : model_data.h model.h camera.h renderer.h main.cpp 
	$(CC) $(CPPFLAGS) -c main.cpp

controller.o : controller.cc controller.h light_controller.h renderer.h camera.h terrain.h object.h model.h
	$(CC) $(CPPFLAGS) -c controller.cc

light_controller.o: light_controller.cc light_controller.h
	$(CC) $(CPPFLAGS) -c light_controller.cc

renderer.o : renderer.cc renderer.h camera.h terrain.h object.h model.h
	$(CC) $(CPPFLAGS) -c renderer.cc

camera.o : camera.cc camera.h
	$(CC) $(CPPFLAGS) -c camera.cc

terrain.o : terrain.cc terrain.h
	$(CC) $(CPPFLAGS) -c terrain.cc

model.o: model.cc model.h object.h model_data.h
	$(CC) $(CPPFLAGS) -c model.cc

object.o: object.cc object.h
	$(CC) $(CPPFLAGS) -c object.cc

model_data.o: model_data.cc model_data.h
	$(CC) $(CPPFLAGS) -c model_data.cc

$(LIB) :
	$(MAKE) -C lib/tiny_obj_loader
	$(MAKE) -C lib/shader

clean:
	rm -f *.o assign3$(EXT)
	$(MAKE) -C lib/tiny_obj_loader clean
	$(MAKE) -C lib/shader clean
