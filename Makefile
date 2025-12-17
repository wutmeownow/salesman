CXXFLAGS = -Wall -g $(shell root-config --cflags)
LDFLAGS  = $(shell root-config --glibs) -lpcre2-8


default: all

all: datareader salesman

	
datareader: datareader.cpp
	# "building city data reader example"
	g++ -O -Wall -o datareader datareader.cpp

salesman: salesman.cpp salesman.h
	g++ $(CXXFLAGS) -o salesman salesman.cpp $(LDFLAGS)


clean:
	rm -f datareader *~ *png
