CC=gcc
EXECUTABLE=pokerev

all: pokerev

pokerev: pokerev.o
	$(CC) pokerev.o /usr/local/lib/libpoker-eval.a -o $(EXECUTABLE)
	strip $(EXECUTABLE)

pokerev.o: pokerev.c
	$(CC) -I include -c pokerev.c

clean:
	rm -f $(EXECUTABLE) pokerev.o
