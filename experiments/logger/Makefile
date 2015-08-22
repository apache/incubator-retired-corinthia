objects = useLogmaker.o log_maker.o
CC = gcc
CFLAGS = -ggdb -std=c99 -Wall 

logger: $(objects)
	$(CC) $(CFLAGS) -o logMaker $(objects)

useLogmaker.o:  log_maker.h

log_maker.o: log_maker.h

.PHONY : clean
clean :
	rm logMaker $(objects)
