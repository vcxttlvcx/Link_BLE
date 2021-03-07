qq: rb.o sb.o ui.o
	gcc rb.o sb.o ui.o -o qq -lpthread -lm -lwiringPi

rb.o: receiveBeacon.c
	gcc -c -o rb.o receiveBeacon.c -lpthread -lm -lwiringPi

ui.o: ui.c
	gcc -c -o ui.o ui.c -lwiringPi

sb.o: sendBeacon.c
	gcc -c -o sb.o sendBeacon.c
