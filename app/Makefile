# Makefile

CC = gcc
CFLAGS = `pkg-config --cflags gtk+-3.0` -Wall -O3 -fsanitize=address
LDLIBS = `pkg-config --libs gtk+-3.0` -lSDL2_image -Ineural_network -lSDL2 -lm

SRC = music-player.c 
OBJ = ${SRC:.c=.o}
DEP = ${SRC:.c=.d}

music-player: ${OBJ}
	${CC} ${CFLAGS} ${OBJ} -o $@ ${LDLIBS}

neural_network/OCR.o: neural_network/OCR.c
	${CC} ${CFLAGS} -c -o $@ $<

sudoku_solver/solver.o: sudoku_solver/solver.c
	${CC} ${CFLAGS} -c -o $@ $<

-include ${DEP}

.PHONY: clean

clean:
	${RM} ${OBJ}
	${RM} ${DEP}
	${RM} music-player

# END

