CC=gcc
EXECUTABLE=pokerev
OBJECTS=pokerev.o
INCLUDEDIR=/usr/local/include/poker-eval/
LIBPOKER=/usr/local/lib/libpoker-eval.a

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(OBJECTS) $(LIBPOKER) -o $@
	strip $(EXECUTABLE)

.c.o:
	$(CC) -I $(INCLUDEDIR) -c $< -o $@

test: $(EXECUTABLE)
	./selftest.sh

install: $(EXECUTABLE)
	install -D pokerev /usr/local/bin

clean:
	rm -f $(EXECUTABLE) $(OBJECTS)
