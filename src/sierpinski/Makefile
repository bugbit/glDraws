.PHONY: clean all

CFLAGS = -o3 -g
LIBS = 
BUILD := ../../build/sierpinski
EXE := ${BUILD}/sierpinski
ifdef EMSCRIPTEN
	override CFLAGS += -s USE_GLFW=3 -s FULL_ES2=1
	BUILD := ${BUILD}/wasm
	EXE := ${BUILD}/sierpinski.html
else
	override LIBS += -lglfw -lGL -lGLEW
endif

all: main.cpp 
	mkdir -p ${BUILD} && $(CXX) $(CFLAGS) -o ${EXE} main.cpp $(LIBS)

clean: $(RM) main.o