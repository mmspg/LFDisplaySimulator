CC = g++
CFLAGS = -g -Wall

# SRCS1 = main.cpp myLayerOptimize.cpp myLayerOptimizeWithFocalStack.cpp myFocalStack.cpp myLayerStack.cpp myLightField.cpp myTransformCoordinate.cpp
# HEAD1 = myPSNRcalculator.h myLayerOptimize.h myLayerOptimizeWithFocalStack.h myFocalStack.h myLayerStack.h myLightField.h myTransformCoordinate.h
# PROG1 = calclayer

SRCS2 = simulator.cpp
PROG2 = simulator

OPENCV = `pkg-config opencv --cflags --libs --cflags glfw3`
LIBS1 = $(OPENCV)

# OPENGL = -lglut
# for OSX
OPENGL = -L/usr/local/Cellar/glfw/3.2.1/lib -framework GLUT -framework OpenGL -Wno-deprecated

LIBS2 = $(OPENGL)

all: $(PROG2)

# $(PROG1):$(SRCS1) $(HEAD1)
# 	$(CC) -o $(PROG1) $(SRCS1) $(LIBS1)

$(PROG2):$(SRCS2)
	$(CC) $(STR) -o $(PROG2) $(SRCS2) $(LIBS1) $(LIBS2)



clean:
	rm $(PROG2)


