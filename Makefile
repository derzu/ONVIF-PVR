FLAGS=-g -std=c++11 -lpthread -lX11
FLAGS_OPENCV=`pkg-config --cflags --libs opencv`
INCLUDES=-Iinclude
LIBS=-lm -L/usr/local/lib/x86_64-linux-gnu/
TARGET=OnvifPVR
CCP=g++

SOURCE= main.o \
		src/Camera.o \
		src/DiscoveryRTSP_IP.o \
		src/VideoListener.o \
		src/VideoPlayer.o \
		src/VideoWriterManager.o

OBJECTS=$(SOURCE)

all: executavel

executavel: $(OBJECTS)
	$(CCP) -o $(TARGET) $(OBJECTS) $(LIBS) $(INCLUDES) $(FLAGS) $(FLAGS_OPENCV)

.c.o: $<
	$(CCP) -DDEBUG_LEVEL=15 -c $< -o $@ $(LIBS) $(INCLUDES) $(FLAGS) 

.cpp.o: $<
	$(CCP) -DDEBUG_LEVEL=15 -c $< -o $@ $(LIBS) $(INCLUDES) $(FLAGS) $(FLAGS_OPENCV) -std=c++11 -fpermissive


clean:
	@find -iname "*.o" -exec rm {} \;
	@find -iname ".*.o" -exec rm {} \;
	@find -iname "*.so" -exec rm {} \;
	@find -iname "*~" -exec rm {} \;
	@find -iname "*.swp" -exec rm {} \;

