ODEF:=$(shell ode-config --cflags)
ODEFL:=$(shell ode-config --libs)
CC = g++
CFLAGS = -w -g -Wall $(ODEF) -I/usr/include -I/usr/include/GL -I/System/Library/Frameworks/OpenGL.framework/Headers
PROG = waku

SCS = src/usercontrols.cpp src/savegame.cpp src/camera.cpp src/odeutils.cpp src/map.cpp src/terrain/imageloader.cpp src/ThreeMaxLoader.cpp src/md2model.cpp src/math/vec3f.cpp src/math/yamathutil.cpp src/openglutils.cpp src/FractalNoise.cpp src/terrain/Terrain.cpp src/font/DrawFonts.cpp $(shell ls src/units/*.cpp) $(shell ls src/structures/*.cpp) $(shell ls src/actions/*.cpp) src/sounds/sounds.cpp src/engine.cpp src/commandline.cpp src/control.cpp src/ai.cpp src/profiling.cpp
SRCS = $(SCS) src/keplerivworld.cpp src/testbox.cpp src/carrier.cpp

TSRCS = $(SCS) src/testbox.cpp src/carrier.cpp
TOBJS = $(TSRCS:.cpp=.o)

SSRC = $(SCS) src/keplerivworld.cpp src/carrier.cpp 
OBJS = $(SSRC:.cpp=.o)

TESTSRCS = src/opengltemplate.cpp src/openglutils.cpp src/imageloader.cpp

#g++ -lstk -I../stk/include/ -oplayaudio playaudio.cpp -lpthread -framework CoreAudio -framework CoreMIDI -framework CoreFoundation

ifeq ($(shell uname),Darwin)
	LIBS = -framework OpenGL -framework GLUT $(ODEFL)
else
	LIBS = -L/usr/local/lib -I/usr/local/include   -L/usr/lib/x86_64-linux-gnu/ -lGL -lGLU -lglut  $(ODEFL) -pthread
endif

all: $(PROG)

testall:
	g++ src/testall.cpp -otestall

OdeWorld:
	$(CC) $(CFLAGS) -o OdeWorld OdeWorld.cpp $(LIBS)

test:		$(TOBJS)
	@echo "Building test version (testbox)"
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

$(PROG):	$(OBJS)
	@echo "Object files are $(OBJS)"
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)
    
base:
	$(CC) $(CFLAGS) -o cube $(SRCSR) $(LIBS)

.cpp.o:		$(SRCS)		
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(PROG)
	rm -f OdeWorld
	rm -f $(OBJS)
	rm -f testall
	rm -f test

