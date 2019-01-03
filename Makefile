ODEF=$(shell ode-config --cflags)
ODEFL=$(shell ode-config --libs)
CC = g++
CFLAGS = -w -Wall $(ODEF) -I/System/Library/Frameworks/OpenGL.framework/Headers
PROG = waku

SRCS = carrier.cpp keplerivworld.cpp usercontrols.cpp camera.cpp odeutils.cpp map.cpp terrain/imageloader.cpp ThreeMaxLoader.cpp md2model.cpp math/vec3f.cpp math/yamathutil.cpp openglutils.cpp FractalNoise.cpp terrain/Terrain.cpp font/DrawFonts.cpp units/*.cpp

TESTSRC = opengltemplate.cpp openglutils.cpp imageloader.cpp


ifeq ($(shell uname),Darwin)
	LIBS = -framework OpenGL -framework GLUT $(ODEFL)
else
	LIBS = -lglut $(ODEFL)
endif

all: $(PROG)

OdeWorld:
	$(CC) $(CFLAGS) -o OdeWorld OdeWorld.cpp $(LIBS)

test:
	$(CC) $(CFLAGS) -o test $(TESTSRC) $(LIBS)

$(PROG):	$(SRCS)
	$(CC) $(CFLAGS) -o $(PROG) $(SRCS) $(LIBS)
    
base:
	$(CC) $(CFLAGS) -o cube $(SRCSR) $(LIBS)


clean:
	rm -f $(PROG)
	rm -f OdeWorld

