CC=g++
CFLAGS= -std=c++0x -pthread -O3  -Wall -Werror -Wextra 

obj/urcu.o: ./urcu.cpp include/urcu.hpp
		$(CC) $(CFLAGS) -c $< -o $@

examples/example: examples/example.cpp
	cd examples && $(CC) $(CFLAGS) ../obj/urcu.o  example.cpp -o example

example: examples/example