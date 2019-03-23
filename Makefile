OPT = -Wall

Exex:	boundaries.o polygonDisplay.o
	gcc $(OPT) -o  a4  boundaries.o polygonDisplay.o

boundaries.o:	boundaries.c polygonSet.h polygonDisplay.h
	gcc $(OPT) -c boundaries.c

polygonDisplay.o:  polygonDisplay.c  polygonDisplay.h polygonSet.h
	gcc $(OPT) -c polygonDisplay.c

clean:
	rm -vf *.o a4
