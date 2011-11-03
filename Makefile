CC="gcc"
EXECUTABLE="pokerev"

all: pokerev

pokerev: pokerev.c
	$(CC) pokerev.c -o $(EXECUTABLE)

clean:
	rm -f $(EXECUTABLE)
