prog: main.o source.o
	gcc main.o source.o -o prog -lSDL2 -lSDL2_image -lSDL2_ttf -lm  -g

main.o: main.c
	gcc -c main.c -g

source.o: source.c
	gcc -c source.c -g

clean:
	rm -f *.o prog
