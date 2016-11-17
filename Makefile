############################################################################
# Makefile for run length encode/decode library and sample program
############################################################################
CC = mpicc
CFLAGS = -std=gnu99 -Wall -g


all:  		BaseStation WSN

BaseStation:    BaseStation.c 		
		$(CC) $(CFLAGS) -o BaseStation BaseStation.c

WSN:		WSN.c
		$(CC) $(CFLAGS) -o WSN WSN.c 

clean:
		$(DEL) *.o
