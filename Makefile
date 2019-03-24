OPT = -Wall


All: sim rob stop

sim:  simo
	gcc $(OPT) -o simulator simulator.o  -lm -lpthread -I/opt/X11/include  -L/opt/X11/lib -lX11

simo:  simulator.c  display.c simulator.h
	gcc $(OPT) -c simulator.c

rob:	robotClient.c simulator.h
	gcc $(OPT) -o robotClient robotClient.c  -lm -lpthread

stop:	 stop.c simulator.h
	gcc $(OPT) -o stop stop.c -lm -lpthread

clean:
	rm -vf *.o simulator robotClient stop
