ODEF=$(shell ode-config --cflags)
ODEFL=$(shell ode-config --libs)
CC = g++
CFLAGS = -w -g -Wall $(ODEF) -I/System/Library/Frameworks/OpenGL.framework/Headers
PROG = waku

SCS = usercontrols.cpp camera.cpp odeutils.cpp map.cpp terrain/imageloader.cpp ThreeMaxLoader.cpp md2model.cpp math/vec3d.cpp math/vec3f.cpp math/yamathutil.cpp openglutils.cpp FractalNoise.cpp terrain/Terrain.cpp font/DrawFonts.cpp $(shell ls units/*.cpp) $(shell ls structures/*.cpp) $(shell ls actions/*.cpp) sounds/sounds.cpp engine.cpp 
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
	LIBS = -lglut $(ODEFL)
endif

all: $(PROG)

testall:
	g++ testall.cpp -otestall

OdeWorld:
	$(CC) $(CFLAGS) -o OdeWorld OdeWorld.cpp $(LIBS)

test:		$(TOBJS)
	@echo "Building test version (testbox)"
	$(CC) $(CFLAGS) $(LIBS) -o $@ $^

$(PROG):	$(OBJS)
	@echo "Object files are $(OBJS)"
	$(CC) $(CFLAGS) $(LIBS) -o $@ $^
    
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

