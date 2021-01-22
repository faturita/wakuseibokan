ODEF:=$(shell ode-config --cflags)
ODEFL:=$(shell ode-config --libs)
CC = g++
CFLAGS = -w -g -Wall $(ODEF) -I/usr/include -I/usr/include/GL -I/System/Library/Frameworks/OpenGL.framework/Headers
PROG = waku

SCS = usercontrols.cpp camera.cpp odeutils.cpp map.cpp terrain/imageloader.cpp ThreeMaxLoader.cpp md2model.cpp math/vec3f.cpp math/yamathutil.cpp openglutils.cpp FractalNoise.cpp terrain/Terrain.cpp font/DrawFonts.cpp $(shell ls units/*.cpp) $(shell ls structures/*.cpp) $(shell ls actions/*.cpp) sounds/sounds.cpp engine.cpp commandline.cpp control.cpp ai.cpp profiling.cpp
SRCS = $(SCS) keplerivworld.cpp testbox.cpp carrier.cpp

TSRCS = $(SCS) testbox.cpp carrier.cpp
TOBJS = $(TSRCS:.cpp=.o)

SSRC = $(SCS) keplerivworld.cpp carrier.cpp 
OBJS = $(SSRC:.cpp=.o)

TESTSRCS = opengltemplate.cpp openglutils.cpp imageloader.cpp

#g++ -lstk -I../stk/include/ -oplayaudio playaudio.cpp -lpthread -framework CoreAudio -framework CoreMIDI -framework CoreFoundation

ifeq ($(shell uname),Darwin)
	LIBS = -framework OpenGL -framework GLUT $(ODEFL)
else
	LIBS = -L/usr/local/lib -I/usr/local/include   -L/usr/lib/x86_64-linux-gnu/ -lGL -lGLU -lglut  $(ODEFL) -pthread
endif

all: $(PROG)

testall:
	g++ testall.cpp -otestall

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

