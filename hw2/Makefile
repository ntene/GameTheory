CXX = g++ -O2 -std=c++11 -Wall
F_LINUX = -D LINUX
CLEAN = rm -rf
SRC_FILES = $(filter-out src/agent_2.cpp, $(wildcard src/*.cpp))

all:
	$(CXX) $(F_LINUX) $(SRC_FILES) -c
	$(CXX) src/agent.cpp -o agent
	$(CXX) src/agent_1.cpp -o a2
	$(CLEAN) -f *.o
clean:
	$(CLEAN) agent a2
	$(CLEAN) .log.*
