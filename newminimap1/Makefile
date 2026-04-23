CC = gcc
CFLAGS = -Wall -g
SRC = main.c background.c minimap.c joueur.c
OBJ = $(SRC:.c=.o)
TARGET = minimap

all:	$(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJ) -lSDL2 -lSDL2_image -lSDL2_ttf -lm

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)

