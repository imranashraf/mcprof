CC      = g++
CFLAGS  = -g -O2 -MD
INCS    = -I.
LFLAGS  = -L.
LIBS    = -lm

EXEC	= exec
RUN_CMD	=./$(EXEC)
SRCS	= globals.cpp shadow.cpp commatrix.cpp mcprof.cpp
OBJS	=$(SRCS:%.cpp=%.o)

all: $(EXEC)

$(EXEC) : $(OBJS)
	$(CC) $(OBJS) -o $(EXEC) $(LFLAGS) $(LIBS)

%.o : %.cpp
	$(CC) $(CFLAGS) $(INCS) -c $< -o $@
	@cp $*.d $*.P; \
	sed -e 's/#.*//' -e 's/^[^:]*: *//' -e 's/ *\\$$//' \
	-e '/^$$/ d' -e 's/$$/ :/' < $*.d >> $*.P; \
	rm -f $*.d

-include *.P

run: $(EXEC)
	$(RUN_CMD)

clean:
	rm -f $(EXEC) $(OBJS) *~ *.P
open:
	kate makefile globals.h globals.cpp shadow.h shadow.cpp commatrix.h commatrix.cpp mcprof.cpp &> /dev/null &

.PHONY: clean run all archive
