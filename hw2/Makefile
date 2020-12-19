CXX = g++ -std=c++11 -Wall
F_LINUX = -D LINUX
CLEAN = rm -rf
SRC_FILES = $(filter-out src/agent_2.cpp, $(wildcard src/*.cpp))

all:
	$(CXX) $(F_LINUX) $(SRC_FILES) -c
	$(CXX) $(F_LINUX) main.o getch.o -o game
	$(CXX) baseline.o -o random
	$(CXX) -D CONSERVATIVE src/baseline.cpp -o conservative
	$(CXX) -D GREEDY src/baseline.cpp -o greedy
	$(CXX) src/agent.cpp -o agent
	$(CXX) src/agent_1.cpp -o a2
	$(CLEAN) -f *.o
clean:
	$(CLEAN) game greedy conservative random agent
	$(CLEAN) .log.*
