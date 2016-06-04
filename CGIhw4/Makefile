CXX = g++
CXXFLAGS = -O3 -std=c++14

all: Main.o WebServer.o CGI.o
	${CXX} $^ ${CXXFLAGS} -o WebServer

%.o: %.cpp
	${CXX} $< ${CXXFLAGS} -c 	
	
clean:
	rm -rf *.o WebServer	