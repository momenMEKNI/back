# Règle finale pour lier tous les .o et créer l'exécutable
puzzle: main.o puzzle.o
	gcc main.o puzzle.o -o puzzle -lSDL2 -lSDL2_image -lSDL2_mixer -lSDL2_ttf -g

# Règle pour compiler main.c en main.o
main.o: main.c puzzle.h
	gcc -c main.c -g
	
puzzle.o: puzzle.c 
	  gcc -c puzzle.c -g
