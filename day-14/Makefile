all: part-1 anim

part-1: 1_2.c
	$(CC) 1_2.c -g -ggdb -Wall -W -pedantic -O2 -o part-1_2

anim: anim.c
	$(CC) anim.c -g -ggdb -Wall -W -pedantic -O2 -o anim

clean:
	rm -rf part-1_2 anim
	rm -rf *.dSYM
