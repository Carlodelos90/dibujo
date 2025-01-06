CC      = gcc
CFLAGS  = -Wall -Wextra
LDFLAGS = -lncurses
TARGET  = dibujo

all: $(TARGET)

$(TARGET): main.o
	$(CC) $(CFLAGS) -o $@ main.o $(LDFLAGS)

main.o: main.c
	$(CC) $(CFLAGS) -c main.c

clean:
	rm -f main.o $(TARGET)
