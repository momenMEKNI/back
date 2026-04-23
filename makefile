quiz: main.o enigme.o
	gcc main.o enigme.o -o quiz -lSDL2 -lSDL2_image -lSDL2_mixer -lSDL2_ttf -g

main.o: main.c enigme.h
	gcc -c main.c -g

enigme.o: enigme.c
	gcc -c enigme.c -g

clean:
	rm -f *.o quiz
