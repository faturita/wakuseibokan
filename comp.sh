g++ -std=c++17 -w -g -Wall -O2 -I/c/freeglut/include -I/usr/local/include -msse2 -mmmx -mstackrealign -MP -MD -lglu32 -lopengl32 -lpthread -lstdc++ -lfreeglut -fpermissive -c src/usercontrols.cpp -o src/usercontrols.o