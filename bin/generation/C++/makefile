# makefile for App.cpp
# Created on: 23.05.2012
all: compile run

compile: ModelGenerator.h ModelGenerator.cpp
	g++ -o App.exe ModelGenerator.cpp

run:
	./App

clean:
	rm -f *.o App