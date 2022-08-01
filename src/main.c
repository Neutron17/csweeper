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

#define MINE 'X'
#define CLOSED 'C'

// Max depth for signal handler
#define SIGH_MAXD 2

typedef struct {
	uint8_t value;
	bool isMine;
	bool isOpen;
} Cell;

typedef struct {
	uint8_t w, h;
	int cursorPtr;
	Cell *cells;
	//char *cells;
} Board;

enum CmdType {
	C_QUIT,
	C_HELP,
	C_NONE,
	C_ERR,
	C_UNKNOWN,

	C_UP,
	C_DOWN,
	C_LEFT,
	C_RIGHT,
	C_SELECT,

	C_SAVE
};

Board createBoard(uint8_t w, uint8_t h);
void destroyBoard(Board *board);

// Board manipulation
Cell getCell(Board board, uint8_t x, uint8_t y);
Cell getCell_checked(Board board, uint8_t x, uint8_t y);
Cell *getCell_ref(const Board *board, uint8_t x, uint8_t y);
Cell *getCell_checked_ref(const Board *board, uint8_t x, uint8_t y);
void setCell(Board *board, uint8_t x, uint8_t y, Cell v);
void setCell_checked(Board *board, uint8_t x, uint8_t y, Cell v);

void printBoard(Board board);
void spawnMines(Board *board, size_t count);
int countNeighbourMines(Board board, uint8_t x, uint8_t y);
int setNeighbourZeros(Board *board, uint8_t x, uint8_t y);
void clearBoard(Board *board);
enum CmdType parseInput(const char *input);
/* returns: true: fail, false: success */
bool handleInput(enum CmdType type, Board *board);
void saveBoard(Board *board, const char *fname);

void signalHandler(int signal);

const char *cmdHelp =
	"Minesweeper - command help\n"
	"\tH,?: help\n"
	"\tq: quit\n";
bool isDebug = false;
// global board, should only be used/changed in main and signalHandler
Board gboard;

/* TODO
 * load
*/

int main(int argc, char *argv[]) {
	srand(time(NULL));
	signal(SIGSEGV, signalHandler);
	signal(SIGABRT, signalHandler);

	struct Args args = parseArgs(argc, argv);
	isDebug = args.isDebug;

	gboard = createBoard(args.w, args.h);

	spawnMines(&gboard, args.m);

	for(int y = 0; y < gboard.h; y++) {
		for(int x = 0; x < gboard.w; x++) {
			if(getCell(gboard, x, y).isMine)
				continue;
			int neighs = countNeighbourMines(gboard, x, y);
			getCell_ref(&gboard, x, y)->value = neighs;
		}
	}
	bool running = true;
	char inpBuff[16];

	while(running) {
		printBoard(gboard);
inp:;
    		printf("> ");
		fflush(stdout);
		if(fgets(inpBuff, 15, stdin) == NULL) {
			fprintf(stderr, "ERROR: Error while getting input\n");
			goto cleanUp;
		}
		enum CmdType type = parseInput(inpBuff);
		switch(type) {
			case C_QUIT:
				running = false;
				break;
			case C_SELECT:;
				Cell *curc = &gboard.cells[gboard.cursorPtr];
				if(curc->isOpen) {
					// TODO
				} else { // not open
					curc->isOpen = true;
					if(curc->isMine) {
						printf("GAME OVER\n");
						running = false;
					} else if(curc->value == 0) {
					puts("Here");
						printf("%d %d", gboard.cursorPtr%gboard.h, gboard.cursorPtr/gboard.h);
						setNeighbourZeros(&gboard, gboard.cursorPtr%gboard.h, gboard.cursorPtr/gboard.h);
					}
				}
				break;
			default:
				if(handleInput(type, &gboard))
					goto inp;
				break;
		}
	}
cleanUp:;
	if(isDebug)
		puts("DEBUG: Cleaning up");
	destroyBoard(&gboard);
	if(isDebug)
		puts("DEBUG: Exiting");
	return E_SUCC;
}

bool handleInput(enum CmdType type, Board *board) {
	switch(type) {
		case C_HELP:
			printf("%s", cmdHelp);
			break;
		case C_ERR:
			fprintf(stderr, "ERROR: error while getting input\n");
			break;
		case C_NONE:
			return true;
			break;
		case C_UNKNOWN:
			fprintf(stderr, "ERROR: Unknown command\n");
			printf("%s", cmdHelp);
			return true;
			break;

		case C_UP:
			board->cursorPtr -= board->w;
			break;
		case C_DOWN:
			board->cursorPtr += board->w;
			break;
		case C_LEFT:
			board->cursorPtr--;
			break;
		case C_RIGHT:
			board->cursorPtr++;
			break;

		case C_SAVE:
			saveBoard(board, "save");
			break;

		case C_SELECT:
		case C_QUIT:
			// Unreachable
			break;
	}
	return false;
}

enum CmdType parseInput(const char *input) {
	if(input == NULL)
		return C_ERR;
	else if(*input == '\n')
		return C_SELECT;
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
		}  else if (strncasecmp(buff, "save", 4) == 0) {
			return C_SAVE;
		} else {
			return C_UNKNOWN;
		}
	}
	return C_ERR;
}

void spawnMines(Board *board, size_t count) {
	if(count > (board->h*board->w))
		return;
	for(size_t i = 0; i < count; i++) {
up:;
		int rnd = rand() % (board->h * board->w);
		if(board->cells[rnd].isMine)
			goto up;
		board->cells[rnd].isMine = true;
		board->cells[rnd].isOpen = false;
	}
}

int setNeighbourZeros(Board *board, uint8_t x, uint8_t y) {
	if(!board ||
		x >= board->w || y >= board->h ||
		getCell(*board, x, y).value != 0 ||
		getCell(*board, x, y).isOpen)
		return -1;
	int count = 0;
	Cell *c = getCell_checked_ref(board, x+1, y);
	if(c && c->value == 0) {
		c->isOpen = true;
		count++;
		count += setNeighbourZeros(board, x+1, y);
	}
	c = getCell_checked_ref(board, x+1, y+1);
	if(c && c->value == 0) {
		c->isOpen = true;
		count++;
		count += setNeighbourZeros(board, x+1, y+1);
	}
	c = getCell_checked_ref(board, x+1, y-1);
	if(c && c->value == 0) {
		c->isOpen = true;
		count++;
		count += setNeighbourZeros(board, x+1, y-1);
	}
	c = getCell_checked_ref(board, x-1, y);
	if(c && c->value == 0) {
		c->isOpen = true;
		count++;
		count += setNeighbourZeros(board, x-1, y);
	}
	c = getCell_checked_ref(board, x-1, y+1);
	if(c && c->value == 0) {
		c->isOpen = true;
		count++;
		count += setNeighbourZeros(board, x+1, y+1);
	}
	c = getCell_checked_ref(board, x-1, y-1);
	if(c && c->value == 0) {
		c->isOpen = true;
		count++;
		count += setNeighbourZeros(board, x+1, y-1);
	}
	c = getCell_checked_ref(board, x, y+1);
	if(c && c->value == 0) {
		c->isOpen = true;
		count++;
		count += setNeighbourZeros(board, x, y+1);
	}
	c = getCell_checked_ref(board, x, y-1);
	if(c && c->value == 0) {
		c->isOpen = true;
		count++;
		count += setNeighbourZeros(board, x, y-1);
	}
	return count;
}

int countNeighbourMines(Board board, uint8_t x, uint8_t y) {
	int count = 0;
	if(getCell_checked(board, x+1, y).isMine) count++;
	if(getCell_checked(board, x+1, y+1).isMine) count++;
	if(getCell_checked(board, x+1, y-1).isMine) count++;
	if(getCell_checked(board, x-1, y).isMine) count++;
	if(getCell_checked(board, x-1, y+1).isMine) count++;
	if(getCell_checked(board, x-1, y-1).isMine) count++;
	if(getCell_checked(board, x, y+1).isMine) count++;
	if(getCell_checked(board, x, y-1).isMine) count++;
	return count;
}

Board createBoard(uint8_t w, uint8_t h) {
	Board ret = {
		w, h,
		0,
		(Cell *) calloc(sizeof(Cell), w * h)
	};
	if(!ret.cells) {
		fprintf(stderr, "ERROR: Couldn't allocate for board, '%s'\n",
				strerror(errno));
		exit(E_ALLOC);
	}
	//memset(ret.cells, (Cell){ 0, false, false }, w*h);
	return ret;
}

void destroyBoard(Board *board) {
	free(board->cells);
	board->cells = NULL;
}

Cell getCell_checked(Board board, uint8_t x, uint8_t y) {
	if(x >= board.w || y >= board.h)
		return (Cell){ 0,false,false };
	return board.cells[y * board.w + x];
}
Cell getCell(Board board, uint8_t x, uint8_t y) {
	return board.cells[y * board.w + x];
}
Cell *getCell_ref(const Board *board, uint8_t x, uint8_t y) {
	return &board->cells[y * board->w + x];
}
Cell *getCell_checked_ref(const Board *board, uint8_t x, uint8_t y) {
	if(x >= board->w || y >= board->h)
		return NULL;
	return &board->cells[y * board->w + x];
}
void setCell_checked(Board *board, uint8_t x, uint8_t y, Cell v) {
	if(x >= board->w || y >= board->h)
		return;
	board->cells[y * board->w + x] = v;
}
void setCell(Board *board, uint8_t x, uint8_t y, Cell v) {
	board->cells[y * board->w + x] = v;
}

void printBoard(Board board) {
	for(int i = 0; i < board.w + 2; i++)
		printf("-");
	puts("");
	for(int y = 0; y < board.h; y++) {
		printf("|");
		for(int x = 0; x < board.w; x++) {
			if((y*board.w + x) == board.cursorPtr) {
				printf("P");
				continue;
			}
			Cell c = getCell(board, x, y);
			if(!c.isOpen) {
				printf("%c", CLOSED);
			} else if(c.isMine) {
				printf("%c", MINE);
			} else if(c.value == 0) {
				printf(" ");
			} else {
				printf("%d", c.value);
			}
			//printf("%c", getCell(board, x, y).value);
		}
		puts("|");
	}
	for(int i = 0; i < board.w + 2; i++)
		printf("-");
	puts("");
}

void saveBoard(Board *board, const char *fname) {
	FILE *file = fopen(fname, "w");
	if(!file) {
		fprintf(stderr, "ERROR: Couldn't open file for board, '%s', '%s'\n", fname, strerror(errno));

		return;
	}
	fwrite(board, 1, sizeof(Board), file);
	fflush(file);
	fclose(file);
}
void signalHandler(int signal) {
	if(signal == SIGINT) {
		destroyBoard(&gboard);
	}
	static int depth = 0;
	if(depth > SIGH_MAXD)
		exit(E_SIGH);
	const char *fname = "err.log";
	fprintf(stderr, "FATAL ERROR, Dumping contents of board to %s\n", fname);
	saveBoard(&gboard, fname);
	if(isDebug)
		puts("DEBUG: Exiting");
}

