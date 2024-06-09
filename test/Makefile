# Little XML Makefile
# Linux

PROGRAM = testmain

SRCS    = $(PROGRAM).c
OBJS    = $(SRCS)
INC     = -I../
LIBS    = 
DEBUG   = -g
CFLAGS  = -Wall -Wextra $(DEBUG)
LFLAGS  =
CC      = gcc
RM      = rm -f

all: $(PROGRAM)

$(PROGRAM) : $(OBJS)
	$(CC) -o $@ $(CFLAGS) $(OBJS) $(INC) $(LIBS)

pedantic:
	$(CC) -o $@ $(CFLAGS) -std=c89 --pedantic $(OBJS) $(INC) $(LIBS)

clean:
	$(RM) $(PROGRAM)

