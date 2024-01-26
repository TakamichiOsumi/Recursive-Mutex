CC	= gcc
CFLAGS	= -O0 -Wall
PROGRAM	= executable

$(PROGRAM): rec_mutex.o
	$(CC) $(CFLAGS) test_rec_mutex.c rec_mutex.c -o $@

rec_mutex.o:
	$(CC) $(CFLAGS) -c rec_mutex.c

.PHONY: clean test

clean:
	rm -rf $(PROGRAM) rec_mutex.o

test: $(PROGRAM)
	@./$(PROGRAM) && echo "Successful if the result is zero >>> $$?"
