compiler=clang
flags=-std=c99 -Wall -pedantic -Werror -L. -lruntime
executable=main

# Default value for debug print: do not print.
# To enable debug print provide it as argument, e.g.: make run DC_DEBUG_PRINT=1
# Value provided to program as compile-time #define by -D flag
DC_DEBUG_PRINT := 0

all:
	$(compiler) $(flags) -D DC_DEBUG_PRINT='${DC_DEBUG_PRINT}' *.c -o $(executable)

run: all
	LD_PRELOAD="${PWD}/libruntime.so" ./$(executable) -p 3 10 20 30

help:
	@echo "# run commands:"
	@echo 'export LD_LIBRARY_PATH="$${LD_LIBRARY_PATH}:$$(pwd)"'
	@echo 'make DC_DEBUG_PRINT=0 && LD_PRELOAD="$${PWD}/libruntime.so" ./$(executable) -p 3 10 20 30'

clean:
	rm -f $(executable)
	rm -f *.log
	rm -rf cmake-build-debug/
