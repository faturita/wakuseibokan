ODEF=$(shell pkg-config ode --cflags)
ODEFL=$(shell pkg-config ode --libs)
CC = g++
CFLAGS = -Wall $(ODEF) -I/System/Library/Frameworks/OpenGL.framework/Headers -I/Users/rramele/work/ode/include/
PROG = waku

SRCSR = main.cpp camera.cpp odeutils.cpp terrain/imageloader.cpp md2model.cpp carrier/vec3f.cpp carrier/yamathutil.cpp openglutils.cpp FractalNoise.cpp terrain/Terrain.cpp font/DrawFonts.cpp units/*.cpp

SRCS = carrier.cpp keplerivworld.cpp usercontrols.cpp camera.cpp odeutils.cpp terrain/imageloader.cpp md2model.cpp carrier/vec3f.cpp carrier/yamathutil.cpp openglutils.cpp FractalNoise.cpp terrain/Terrain.cpp font/DrawFonts.cpp units/*.cpp


ifeq ($(shell uname),Darwin)
	LIBS = -framework OpenGL -framework GLUT $(ODEFL)
else
	LIBS = -lglut $(ODEFL)
endif

all: $(PROG)

$(PROG):	$(SRCS)
	$(CC) $(CFLAGS) -o $(PROG) $(SRCS) $(LIBS)
    
base:
	$(CC) $(CFLAGS) -o cube $(SRCSR) $(LIBS)


clean:
	rm -f $(PROG)
