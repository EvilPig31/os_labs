all: parent child

parent: parent.c
	gcc -o parent parent.c

child: child.c
	gcc -o child child.c

clean:
	rm -f parent child

.PHONY: all clean