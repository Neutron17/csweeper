#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

#include "args.h"
#include "errors.h"
#include "common.h"
#include "board.h"
#include "act.h"

// Max depth for signal handler
#define SIGH_MAXD 2


enum ActType parseInput(const char *input);
/* returns: true: fail, false: success */
bool handleInput(enum ActType type, Board *realB, Board *virtB);

void signalHandler(int signal);
// static variable magic for signalHandler
Board *sigRBoard(Board *board);
Board *sigVBoard(Board *board);

bool isDebug = false;

/* TODO
 * open all closed nulls next to null
*/

int main(int argc, char *argv[]) {
	srand(time(NULL));
	signal(SIGSEGV, signalHandler);
	signal(SIGABRT, signalHandler);

	struct Args args = parseArgs(argc, argv);
	isDebug = args.isDebug;

	Board realBoard = boardCreate(args.w, args.h);
	Board virtB = boardCreate(args.w, args.h);
	sigRBoard(&realBoard);
	sigVBoard(&realBoard);
	printf("m %d\n", args.m);

	spawnMines(&realBoard, args.m);

	for(int y = 0; y < realBoard.h; y++) {
		for(int x = 0; x < realBoard.w; x++) {
			if(cellGet(realBoard, x, y).isMine)
				continue;
			int neighs = countNeighbourMines(realBoard, x, y);
			cellGet_ref(&realBoard, x, y)->value = neighs;
		}
	}
	boardCopy(&virtB, realBoard);
	bool running = true;
	char inpBuff[16];

	while(running) {
		boardPrint(virtB);
inp:;
    		printf("> ");
		fflush(stdout);
		if(fgets(inpBuff, 15, stdin) == NULL) {
			fprintf(stderr, "ERROR: Error while getting input\n");
			goto cleanUp;
		}
		enum ActType type = parseInput(inpBuff);
		if(handleInput(type, &realBoard, &virtB))
			goto inp;
	}
cleanUp:;
	raise(SIGINT);
	return E_SUCC;
}

bool handleInput(enum ActType type, Board *realB, Board *virtB) {
	if(type >= C_SAVE)
		return ActLUT[type](*realB, virtB, (void*)realB);
	else
		return ActLUT[type](*realB, virtB, NULL);
}

enum ActType parseInput(const char *input) {
	if(input == NULL)
		return C_ERR;
	else if(*input == '\n')
		return C_OPEN;
	char b[16];
	strncpy(b, input, 16);
	char *buff = &b[0];

	while(isspace(*buff))
		buff++;

	if(*buff == '\0' || strnlen(buff, 16) < 2)
		return C_NONE;
	if(strnlen(buff, 16) == 2) {
		switch(*buff) {
			case 'q':
				return C_QUIT;
			case '?':
			case 'H':
				return C_HELP;
			case 'l':
			case 'w':
				return C_UP;
			case 'h':
			case 'a':
				return C_LEFT;
			case 'j':
			case 's':
				return C_DOWN;
			case 'k':
			case 'd':
				return C_RIGHT;
			case 'm':
				return C_MINE;
			case 'p':
				return C_DPRINT;
			/*case EOF:
				return C_ERR;*/
			default:
				return C_UNKNOWN;
		}
	} else {
		if(strncasecmp(buff, "help", 4) == 0) {
			return C_HELP;
		} else if (strncasecmp(buff, "quit", 4) == 0) {
			return C_QUIT;
		} else if (strncasecmp(buff, "exit", 4) == 0) {
			return C_QUIT;
		} else if (strncasecmp(buff, "save", 4) == 0) {
			return C_SAVE;
		} else if (strncasecmp(buff, "load", 4) == 0) {
			return C_LOAD;
		} else if (strncasecmp(buff, "mine", 4) == 0) {
			return C_MINE;
		} else {
			return C_UNKNOWN;
		}
	}
	return C_ERR;
}

Board *sigRBoard(Board *board) {
	static Board *b;
	if(!board)
		b = board;
	return b;
}
Board *sigVBoard(Board *board) {
	static Board *b;
	if(!board)
		b = board;
	return b;
}

void signalHandler(int signal) {
	if(signal == SIGINT) {
		if(isDebug)
			puts("DEBUG: Cleaning up");
		boardDestroy(sigRBoard(NULL));
		boardDestroy(sigVBoard(NULL));
		if(isDebug)
			puts("DEBUG: Exiting");
		exit(0);
	}
	static int depth = 0;
	if(depth > SIGH_MAXD)
		exit(E_SIGH);
	const char *fname = "err.log";
	fprintf(stderr, "FATAL ERROR, Dumping contents of board to %s\n", fname);
	boardSave(sigRBoard(NULL), fname);
	if(isDebug)
		puts("DEBUG: Exiting");
}

