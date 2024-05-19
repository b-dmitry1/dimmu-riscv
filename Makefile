CFLAGS+=-fsigned-char
LDFLAGS+=
LIBS=
LDLIBS=

CC=gcc

OBJ=$(wildcard *.c)

all:	riscv

clean:
	rm -f *.o
	rm -f riscv

riscv: $(OBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)
