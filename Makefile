CC = gcc
CFLAGS = -Wall -O2 -g
LIBS = -lSDL2 -lSDL2_image -lSDL2_ttf -lm

TARGET = dreamcore_game
SOURCES = main.c joueur.c menu.c

all: $(TARGET)

$(TARGET): $(SOURCES)
	$(CC) $(CFLAGS) -o $(TARGET) $(SOURCES) $(LIBS)
	@echo "Dreamcore - Lucid Dreams compiled! Run: ./$(TARGET)"

clean:
	rm -f $(TARGET)

run: $(TARGET)
	./$(TARGET)

.PHONY: all clean run
