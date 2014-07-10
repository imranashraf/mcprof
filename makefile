CC      = g++
CFLAGS  = -g -O2 -MD
INCS    = -I.
LFLAGS  = -L.
LIBS    = -lm

EXEC	= exec
RUN_CMD	=./$(EXEC)
SRCS	= globals.cpp shadow.cpp shadow_test.cpp
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

.PHONY: clean run all archive
