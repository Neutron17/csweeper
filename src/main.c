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

	C_SAVE,
	C_LOAD
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
// saves board to file
void saveBoard(const Board *board, const char *fname);
// loads board from file
void loadBoard(Board *board, const char *fname);

void signalHandler(int signal);
// static variable magic for signalHandler
Board *sigBoard(Board *board);

const char *cmdHelp =
	"Minesweeper - command help\n"
	"\tH,?: help\n"
	"\tq: quit\n";
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

	Board board = createBoard(args.w, args.h);
	sigBoard(&board);

	spawnMines(&board, args.m);

	for(int y = 0; y < board.h; y++) {
		for(int x = 0; x < board.w; x++) {
			if(getCell(board, x, y).isMine)
				continue;
			int neighs = countNeighbourMines(board, x, y);
			getCell_ref(&board, x, y)->value = neighs;
		}
	}
	bool running = true;
	char inpBuff[16];

	while(running) {
		printBoard(board);
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
				Cell *curc = &board.cells[board.cursorPtr];
				if(curc->isOpen) {
					// TODO
				} else { // not open
					curc->isOpen = true;
					if(curc->isMine) {
						printf("GAME OVER\n");
						running = false;
					} else if(curc->value == 0) {
						setNeighbourZeros(&board,
								board.cursorPtr % board.h,
								board.cursorPtr / board.h);
					}
				}
				break;
			default:
				if(handleInput(type, &board))
					goto inp;
				break;
		}
	}
cleanUp:;
	if(isDebug)
		puts("DEBUG: Cleaning up");
	destroyBoard(&board);
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
		case C_LOAD:
			loadBoard(board, "save");
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
		} else if (strncasecmp(buff, "save", 4) == 0) {
			return C_SAVE;
		} else if (strncasecmp(buff, "load", 4) == 0) {
			return C_LOAD;
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
				printf("\033[1;41m\033[5m");
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
			printf("\033[m\033[39;49m");
		}
		puts("|");
	}
	for(int i = 0; i < board.w + 2; i++)
		printf("-");
	puts("");
}

void saveBoard(const Board *board, const char *fname) {
	FILE *file = fopen(fname, "wb");
	if(!file) {
		fprintf(stderr, "ERROR: Couldn't open file for board, '%s', '%s'\n", fname, strerror(errno));
		return;
	}
	fwrite(&board->cursorPtr, 1, sizeof(uint8_t), file);
	fwrite(&board->w, 1, sizeof(uint8_t), file);
	fwrite(&board->h, 1, sizeof(uint8_t), file);
	for(int y = 0; y < board->h; y++) {
		for(int x = 0; x < board->w; x++)
			fwrite(getCell_ref(board, x, y), 1, sizeof(Cell), file);
	}
	//fwrite(board, 1, sizeof(Board), file);
	fflush(file);
	fclose(file);
}

void loadBoard(Board *board, const char *fname) {
	FILE *file = fopen(fname, "rb");
	if(!file) {
		fprintf(stderr, "ERROR: Couldn't open file for board, '%s', '%s'\n", fname, strerror(errno));
		return;
	}
	fread(&board->cursorPtr, 1, sizeof(uint8_t), file);
	fread(&board->w, 1, sizeof(uint8_t), file);
	fread(&board->h, 1, sizeof(uint8_t), file);
	for(int y = 0; y < board->h; y++) {
		for(int x = 0; x < board->w; x++)
			fread(getCell_ref(board, x, y), 1, sizeof(Cell), file);
	}
	fflush(file);
	fclose(file);
}

Board *sigBoard(Board *board) {
	static Board *b;
	if(!board)
		b = board;
	return b;
}

void signalHandler(int signal) {
	if(signal == SIGINT) {
		destroyBoard(sigBoard(NULL));
	}
	static int depth = 0;
	if(depth > SIGH_MAXD)
		exit(E_SIGH);
	const char *fname = "err.log";
	fprintf(stderr, "FATAL ERROR, Dumping contents of board to %s\n", fname);
	saveBoard(sigBoard(NULL), fname);
	if(isDebug)
		puts("DEBUG: Exiting");
}

