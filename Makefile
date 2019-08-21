CC=g++
CFLAGS= -std=c++0x -pthread -O3  -Wall -Werror -Wextra 

obj/urcu.o: ./urcu.cpp include/urcu.hpp
		$(CC) $(CFLAGS) -c $< -o $@
