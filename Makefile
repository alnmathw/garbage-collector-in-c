CFLAGS=-Wall -Wextra -Werror -std=c11 -pedantic -ggdb

heap: src/main.c src/heap.c src/heap.h
	$(CC) $(CFLAGS) -o src/heap src/main.c src/heap.c