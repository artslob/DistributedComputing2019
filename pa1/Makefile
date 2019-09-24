compiler=clang
flags=-std=c99 -Wall -pedantic -Werror
executable=main

# Default value for debug print: do not print.
# To enable debug print provide it as argument, e.g.: make run DC_DEBUG_PRINT=1
# Value provided to program as compile-time #define by -D flag
DC_DEBUG_PRINT := 0

all:
	$(compiler) $(flags) -D DC_DEBUG_PRINT='${DC_DEBUG_PRINT}' *.c -o $(executable)

run: all
	./$(executable)

clean:
	rm -f $(executable)
	rm -f *.log
	rm -rf cmake-build-debug/
