SRC = pr7.2.c

LIB = pr6_ctime.c pr6_signal.c pr6_wait.c pr6_table.c
INC = $(LIB:.c=.h)

ALL = $(SRC) $(LIB) $(INC)

OPT = -D_POSIX_PTHREAD_SEMANTICS

pr7: $(ALL)
	gcc -std=c99 -Wall -Wextra -o pr7 $(OPT) $(SRC) $(LIB)