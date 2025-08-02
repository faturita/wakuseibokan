ODEF:=$(shell ode-config --cflags)
ODEFL:=$(shell ode-config --libs)
CC = g++
CFLAGS = -std=c++17 -w -g -Wall $(ODEF) -I/usr/include -I/usr/include/GL -I/System/Library/Frameworks/OpenGL.framework/Headers -I../stk/include -fpermissive
PROG = waku

SCS = src/usercontrols.cpp src/savegame.cpp src/camera.cpp src/odeutils.cpp src/map.cpp src/board.cpp src/hud.cpp src/imageloader.cpp src/ThreeMaxLoader.cpp src/md2model.cpp src/math/vec3f.cpp src/propertystore.cpp src/math/yamathutil.cpp src/math/geometry.cpp src/openglutils.cpp src/FractalNoise.cpp src/math/uuid.cpp src/terrain/Terrain.cpp src/font/DrawFonts.cpp $(shell ls src/units/*.cpp) $(shell ls src/structures/*.cpp) $(shell ls src/actions/*.cpp) $(shell ls src/weapons/*.cpp) src/sounds/sounds.cpp src/sounds/Player.cpp src/sounds/soundtexture.cpp src/engine.cpp src/entities.cpp src/commandline.cpp src/control.cpp src/ai.cpp src/profiling.cpp src/networking/telemetry.cpp src/networking/ledger.cpp src/networking/lobby.cpp
SRCS = $(SCS) src/keplerivworld.cpp src/testbox.cpp src/carrier.cpp

TSRCS = $(SCS) src/testbox.cpp src/carrier.cpp
TOBJS = $(TSRCS:.cpp=.o)

SSRC = $(SCS) src/keplerivworld.cpp src/carrier.cpp 
OBJS = $(SSRC:.cpp=.o)

TCSRCS = $(SCS) src/carrier.cpp src/tests/tester.cpp src/tests/testcase.cpp src/tests/testcase_$(TC).cpp
TCOBJS = $(TCSRCS:.cpp=.o)

TESTSRCS = src/opengltemplate.cpp src/openglutils.cpp src/imageloader.cpp

#g++ -lstk -I../stk/include/ -oplayaudio playaudio.cpp -lpthread -framework CoreAudio -framework CoreMIDI -framework CoreFoundation

ifeq ($(shell uname),Darwin)
	LIBS = -framework ApplicationServices -framework OpenGL -framework GLUT $(ODEFL) -lstk -lpthread -framework CoreAudio -framework CoreMIDI -framework CoreFoundation 
else
	LIBS = -L/usr/local/lib -I/usr/local/include   -L/usr/lib/x86_64-linux-gnu/ -lGL -lGLU -lglut  $(ODEFL) -pthread -lbsd -lstk
endif

all: $(PROG)

testall:
	g++ src/testall.cpp -otestall

OdeWorld:
	$(CC) $(CFLAGS) -o OdeWorld OdeWorld.cpp $(LIBS)

test:		$(TOBJS)
	@echo "Building test version (testbox)"
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)


testcase:	$(TCOBJS)
	@echo "Building test cases"
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
	rm -f testcase
	rm -f testcase.o
	rm -f $(TOBJS)
	rm -f $(TCOBJS)

version:
	@echo "Building version $(VERSION)"

	@echo "namespace WAKU { const char version[] = \"$(VERSION)\"; }" > src/version.h

	sed -i 's/Version=.*/Version=$(VERSION)/' system/linux/waku.desktop
	sed -i 's/#define DEBUG/\/\/#define DEBUG/' src/profiling.h

	git commit -m"Packging version" .
	git tag -a $(VERSION) -m"Packaging version"
	git push origin $(VERSION)


pack: version clean $(PROG)
	tar cvzf waku.tgz waku conf/ data/ dependencies/ docs/ images/ savegames/ scripts/ sky/ sounds/ structures/ terrain/ system/ units/ water/
	tar cvzf waku_$(VERSION).tgz waku.tgz system/linux/install.sh

	sed -i 's/\/\/#define DEBUG/#define DEBUG/' src/profiling.h


