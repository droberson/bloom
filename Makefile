TARGET = bloom
CC = gcc
CFLAGS = -Wall -g
LDFLAGS = -lm

SRC = bloom.c mmh3.c main.c
OBJ = $(SRC:.c=.o)


all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $(TARGET) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)

