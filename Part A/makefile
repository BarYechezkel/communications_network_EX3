CC = gcc
CFLAGS = -Wall -Wextra -std=c99

all: TCP_Sender TCP_Receiver

TCP_Sender: TCP_Sender.o
	$(CC) $(CFLAGS) -o TCP_Sender TCP_Sender.o

TCP_Receiver: TCP_Receiver.o
	$(CC) $(CFLAGS) -o TCP_Receiver TCP_Receiver.o

TCP_Sender.o: TCP_Sender.c
	$(CC) $(CFLAGS) -c TCP_Sender.c

TCP_Receiver.o: TCP_Receiver.c
	$(CC) $(CFLAGS) -c TCP_Receiver.c

clean:
	rm -f TCP_Sender TCP_Receiver *.o
