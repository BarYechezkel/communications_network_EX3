
all: RUDP_Receiver RUDP_Sender


RUDP_Receiver: RUDP_Receiver.o RUDP_API.o
	gcc -Wall -o RUDP_Receiver RUDP_Receiver.o RUDP_API.o

RUDP_Sender: RUDP_Sender.o RUDP_API.o
	gcc -Wall -o RUDP_Sender RUDP_Sender.o RUDP_API.o

RUDP_Sender.o: RUDP_Sender.c 
	gcc -Wall -c RUDP_Sender.c

RUDP_Receiver.o: RUDP_Receiver.c
	gcc -Wall -c RUDP_Receiver.c


RUDP_API.o: RUDP_API.c RUDP_API.h
	gcc -Wall -c RUDP_API.c -o RUDP_API.o


clean:
	rm -f RUDP_Receiver RUDP_Sender *.o
