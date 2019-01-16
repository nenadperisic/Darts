PROGRAM=darts
CC=gcc
CFLAGS=-Wall
LDFLAGS=-lGL -lGLU -lglut -lm
	
$(PROGRAM): main.o image.o
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS)

image.o: image.c image.h
	$(CC) $(CFLAGS) -c $<

main.o: main.c 
	$(CC) $(CFLAGS) -c $<


.PHONY: clean dist

clean:
	-rm *.o $(PROGRAM)
