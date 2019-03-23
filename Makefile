OPT = -Wall

All: sim rob stop

sim:  simulator.c  display.c simulator.h
	gcc $(OPT) -o sim simulator.c  -lm -lpthread -lX11

rob:	robotClient.c simulator.h
	gcc $(OPT) -o rob robotClient.c  -lm -lpthread

stop:	 stop.c simulator.h
	gcc $(OPT) -o stop stop.c -lm -lpthread

clean:
	rm -vf *.o sim rob stop
