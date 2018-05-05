CC=mingw32-g++

CFLAGS= -Wall -pedantic -O2 -fomit-frame-pointer -mtune=i686
LDFLAGS= -s

all: lyxass

.dep:
	$(CC) -MM *.c >.dep

.c.o :
	$(CC) -MD -c $(CFLAGS) $(INC) $< -o $@
	@if [ ! -e $*.d ] ; then mv `basename $*.d` $*.d; fi
	@cp $*.d $*.P;\
	    sed -e 's/#.*//' -e 's/^[^:]*: *//' -e 's/ *\\$$//'\
		-e '/^$$/ d' -e 's/$$/ :/' < $*.d >>$*.P
	@rm $*.d

#.c.o:
#	$(CC) $(CFLAGS) -c $<

SRC = lyxass.c
SRC += parser.c
SRC += debug.c
SRC += label.c
SRC += opcode.c
SRC += pseudo.c
SRC += mnemonics.c
SRC += expression.c
SRC += macro.c
SRC += ref.c
SRC += error.c
SRC += jaguar.c

OBJ = $(SRC:.c=.o)


-include $(SRC:.c=.P)

lyxass: $(OBJ)
	$(CC) $(LDFLAGS) $+ -o $@



clean:
	rm -f .dep
	rm -f *.o
	rm -f lyxass
	rm -f lyxass.exe
	rm -f *~
	rm -f *.P
