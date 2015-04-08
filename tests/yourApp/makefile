# Set the following variables if they are not already
# as an environment variables
PIN_ROOT?=/opt/applics/pin-2.14-67254
MCPROF_ROOT?=/data/repositories/mcprof

CC = gcc
EXEC=test
CMD=./$(EXEC)
CFLAGS=-O2
INCS=-I.
LIBS=-lm

SRCS = vectOps.c
OBJS=$(SRCS:.c=.o)

all: clean mcprof.compile mcprof.execute

$(EXEC):  $(OBJS)
	$(CC) $(LDFLAGS) -o $(EXEC) $^ $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) $(INCS) -c -o $@ $<

run: $(EXEC)
	$(CMD)

MCPROF_OPT:=-RecordStack 0  -TrackObjects 1 -Engine 2
mcprof.compile: CFLAGS= -O1 -g -fno-inline
mcprof.compile: clean $(EXEC)

mcprof.execute: $(EXEC)
	$(PIN_ROOT)/pin -t $(MCPROF_ROOT)/obj-intel64/mcprof.so $(MCPROF_OPT) -- $(CMD)
	$(MCPROF_ROOT)/scripts/dot2pdf.sh communication.dot
# 	dot -Tpdf communication.dot -o communication.pdf	
# 	$(MCPROF_ROOT)/scripts/plotScript.sh

clean:
	rm -f *.o $(EXEC) $(OBJS) *~ *.log gmon.out

.PHONY: clean all run mcprof
