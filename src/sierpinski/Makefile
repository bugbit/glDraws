.PHONY: clean all

CFLAGS = -o3 -g -I../common
LIBS = 
BUILD := ../../build/sierpinski
EXE := ${BUILD}/sierpinski
ifdef EMSCRIPTEN
	override CFLAGS += -s USE_GLFW=3 -s USE_WEBGL2=1
	BUILD := ../../build/wasm/sierpinski
	EXE := ${BUILD}/sierpinski.html
else
	override LIBS += -lglfw -lGL -lGLEW
endif

all: main.cpp 
	mkdir -p ${BUILD} && $(CXX) $(CFLAGS) -o ${EXE} main.cpp $(LIBS)

clean: $(RM) main.o