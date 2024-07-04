CC = gcc
CFLAGS = `pkg-config --cflags gtk+-3.0` -Wall -O3 -fsanitize=address -MMD
LDLIBS = `pkg-config --libs gtk+-3.0` -lSDL2_image -lSDL2 -lm
LIB_PATH = -L/path/to/network_library


SRC = music-player.c src/play.c
OBJ = ${SRC:.c=.o}
DEP = ${SRC:.c=.d}

music-player: ${OBJ}
	${CC} ${CFLAGS} ${OBJ} -o $@ ${LDLIBS} ${LIB_PATH} ${LIBS}

%.o: %.c
	${CC} ${CFLAGS} -c -o $@ $<

-include ${DEP}

.PHONY: clean

clean:
	${RM} ${OBJ} ${DEP} music-player
