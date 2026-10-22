CC = gcc
CFLAGS = -Wall -Wextra -Werror -std=c99 -g

words: words.c
	$(CC) $(CFLAGS) -o words words.c

clean:
	rm -f words

.PHONY: clean