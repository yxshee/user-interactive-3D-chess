# Build the 3D chess game.
#   make        -> ./chess3d
#   make run    -> build, then start the game
#   make test   -> build and run the rule tests (no OpenGL needed)
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

chess3d: $(SRCS)
	$(CXX) $(CXXFLAGS) -o $@ $(SRCS) $(LDLIBS)

run: chess3d
	./chess3d

# The rule tests use only Chess/, so they build without OpenGL or GLUT.
TEST_SRCS := tests/test_rules.cpp $(wildcard Chess/*.cpp)

test_rules: $(TEST_SRCS) $(wildcard Chess/*.h)
	$(CXX) $(CXXFLAGS) -o $@ $(TEST_SRCS)

test: test_rules
	./test_rules

clean:
	rm -f chess3d test_rules

.PHONY: run test clean
