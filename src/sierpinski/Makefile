.PHONY: clean all

BUILD := ../../build
EXE := ${BUILD}/sierpinski
ifdef EMSCRIPTEN
	EXE := ${BUILD}/sierpinski.html
endif

all: main.cpp 
	@echo $(EXE) \
	 @mkdir -p ${BUILD} && $(CXX) -o ${EXE} main.cpp

clean: $(RM) main.o