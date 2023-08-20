CC = g++
CFLAGS = -std=c++17 -w -g -Wall -I/usr/include -I../../../stk/include -fpermissive
PROG = playaudio

SCS = playaudio.cpp
OBJS = $(SCS:.cpp=.o)

all: $(PROG)

LIBS = -lstk -lpthread -framework CoreAudio -framework CoreMIDI -framework CoreFoundation 

$(PROG):	$(OBJS)
	@echo "Object files are $(OBJS)"
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

.cpp.o:		$(SCS)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(PROG)
	rm -f $(OBJS)
