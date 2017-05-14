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
INCLUDE = $(OUTDIR)include/jonson/
HEADERS = jonson.h object.h array.h token.h ealloc.h

LIB = $(LIBDIR)libjonson.a
OBJ = jonson.o object.o array.o token.o ealloc.o strbuffer.o stack.o

all: lib
	echo $(CFLAGS)

lib: $(OBJ)
	mkdir -p $(OUTDIR) $(LIBDIR) $(INCLUDE)
	ar rcs $(LIB) $(OBJ)
	cp -f $(HEADERS) $(INCLUDE)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(LIB) $(OBJ)
	rm -rf $(addprefix $(INCLUDE), $(HEADERS))
