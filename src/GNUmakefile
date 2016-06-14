.PHONY : all


CFLAGS+= -Wall -g $(INCL)
CC= 	gcc
MPIROOT= /usr
MPICC=	$(MPIROOT)/bin/mpicc

INCL= -I$(MPIROOT)/include/mpi
SRCS= eleccion.c main.c control.c
OBJS= $(SRCS:%.c=%.o)
BIN= eleccion_lider

all: eleccion_lider

$(BIN): $(OBJS)
	$(MPICC) $(CFLAGS) -o $(BIN) $(OBJS) $(LIBS)

clean:
	rm -f $(OBJS) $(BIN)
