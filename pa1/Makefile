compiler=clang
flags=-std=c99 -Wall -pedantic -Werror
executable=main

all:
	$(compiler) $(flags) *.c -o $(executable)

run: all
	./$(executable)

clean:
	rm $(executable)
