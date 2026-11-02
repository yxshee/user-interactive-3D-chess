# Build the 3D chess game.
#   make        -> ./chess
#   make run    -> build, then start the game
CXX      ?= g++
CXXFLAGS ?= -std=c++11 -O2
SRCS     := main.cpp Model.cpp $(wildcard Chess/*.cpp)

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
  LDLIBS := -framework OpenGL -framework GLUT
  CXXFLAGS += -DGL_SILENCE_DEPRECATION
else
  LDLIBS := -lGL -lGLU -lglut
endif

chess: $(SRCS)
	$(CXX) $(CXXFLAGS) -o $@ $(SRCS) $(LDLIBS)

run: chess
	./chess

clean:
	rm -f chess

.PHONY: run clean
