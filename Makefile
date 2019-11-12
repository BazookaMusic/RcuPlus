CC=g++
CFLAGS= -std=c++0x -pthread -O3  -Wall -Werror -Wextra 

obj/urcu.o: ./urcu.cpp include/urcu.hpp
		$(CC) $(CFLAGS) -c $< -o $@

examples/example: examples/example.cpp
	cd examples && $(CC) $(CFLAGS) ../obj/urcu.o  example.cpp -o example

examples: examples/example

obj/urcu_test_main.o: urcu_test_main.cpp
	$(CC) $(CFLAGS) -c urcu_test_main.cpp -o $@

DEPS= obj/urcu_test_main.o urcu_test.cpp obj/urcu.o
tests: $(DEPS)
	$(CC) $(CFLAGS) $(DEPS) -o $@

run-tests: tests
	./tests
	