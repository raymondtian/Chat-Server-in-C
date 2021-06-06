CC=gcc
CFLAGS=-Wall -Werror -Wvla -std=c11 -fsanitize=address -std=gnu11
PFLAGS=-fprofile-arcs -ftest-coverage
DFLAGS=-g
HEADERS=server.h
SRC=server.c

procchat: $(SRC) $(HEADERS)
	$(CC) $(CFLAGS) $(DFLAGS) $(SRC) -o $@

test:
	$(CC) $(CFLAGS) $(PFLAGS) $(SRC) -o $@

clean:
	rm -f procchat

