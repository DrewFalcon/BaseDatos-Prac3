CC=gcc
CFLAGS=-Wall -Wextra -g

OBJS=main.o index.o deleted.o record.o utils.o

library: $(OBJS)
	$(CC) $(CFLAGS) -o library $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o library

run:
	./library bestfit test