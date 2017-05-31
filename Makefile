CC      = /usr/bin/gcc
CFLAGS  = -std=c99 -pedantic -Wall -Wextra -Werror -Wno-unused-parameter \
	  -Wno-unused-function -Wmissing-prototypes -Wstrict-prototypes \
	  -Wold-style-definition

ifdef VALGRIND
CFLAGS += -g -O0
else
CFLAGS += -O3
endif

OUTDIR  = ../
LIBDIR  = $(OUTDIR)lib/

LIB = $(LIBDIR)libjonson.a
OBJ = jonson.o object.o array.o stream.o token.o ealloc.o strbuffer.o stack.o \
	chain/chain.o

all: lib
	echo $(CFLAGS)

lib: $(OBJ)
	mkdir -p $(OUTDIR) $(LIBDIR)
	ar rcs $(LIB) $(OBJ)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(LIB) $(OBJ)
