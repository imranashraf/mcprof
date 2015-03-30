PIN_DIR=/opt/applics/pin-2.14-67254
MCPROF_DIR=/data/repositories/mcprof

CC = gcc
EXEC=test
CMD=./$(EXEC)
CFLAGS=-O2
INCS=-I.
LIBS=-lm

SRCS = vectOps.c
OBJS=$(SRCS:.c=.o)

all: clean $(EXEC)
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
	$(PIN_DIR)/pin -t $(MCPROF_DIR)/obj-intel64/mcprof.so $(MCPROF_OPT) -- $(CMD)
# 	dot -Tpdf communication.dot -o communication.pdf
	$(MCPROF_DIR)/scripts/dot2pdf.sh communication.dot
# 	$(MCPROF_DIR)/scripts/plotScript.sh

clean:
	rm -f *.o $(EXEC) $(OBJS) *~ *.log gmon.out

.PHONY: clean all run mcprof
