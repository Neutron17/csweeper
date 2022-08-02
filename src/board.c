#include "board.h"
#include "common.h"
#include "errors.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

void spawnMines(Board *board, uint8_t count) {
	printf("Count: %d\n", count);
	if(count > (board->h*board->w))
		return;
	for(int i = 0; i < count; i++) {
up:;
		int rnd = rand() % (board->h * board->w);
		if(board->cells[rnd].isMine)
			goto up;
		board->cells[rnd].isMine = true;
		board->cells[rnd].isOpen = false;
	}
}

const int xs[8] = {
	+1, +1, +1,
	-1, -1, -1,
	0, 0
};

const int ys[8] = {
	0, +1, -1,
	0, +1, -1,
	+1, -1
};

int setNeighbourZeros(Board realB, Board *board, int8_t x, int8_t y, bool isFirstCall) {
	static int depth = 0;
	if(isFirstCall)
		depth = 0;
	depth++;
	if(depth > 128)
		return -2;
	if(!board ||
		x >= board->w || y >= board->h ||
		x < 0 || y < 0 ||
		//cellGet(*board, x, y).value != 0 ||
		cellGet(realB, x, y).isMine ||
		cellGet(*board, x, y).isOpen
		){
		return -1;
	}
	int count = 0;
	Cell *c = NULL;
	for(int i = 0; i < 8; i++) {
		c = cellGet_checked_ref(board, x+xs[i], y+ys[i]);
		if(c) {
			if(c->isOpen)
				return count;
			count++;
			if(c->value == 0)
				count += setNeighbourZeros(realB, board, x+xs[i], y+ys[i], false);
			c->isOpen = true;
		}
	}
	cellGet_ref(board, x, y)->isOpen = true;
	/*cellGet_ref(board, x, y)->isOpen = true;
	Cell *c = cellGet_checked_ref(board, x+1, y);
	if(c && c->value == 0) {
		c->isOpen = true;
		count++;
		count += setNeighbourZeros(board, x+1, y);
	}
	c = cellGet_checked_ref(board, x+1, y+1);
	if(c && c->value == 0) {
		c->isOpen = true;
		count++;
		count += setNeighbourZeros(board, x+1, y+1);
	}
	c = cellGet_checked_ref(board, x+1, y-1);
	if(c && c->value == 0) {
		c->isOpen = true;
		count++;
		count += setNeighbourZeros(board, x+1, y-1);
	}
	c = cellGet_checked_ref(board, x-1, y);
	if(c && c->value == 0) {
		c->isOpen = true;
		count++;
		count += setNeighbourZeros(board, x-1, y);
	}
	c = cellGet_checked_ref(board, x-1, y+1);
	if(c && c->value == 0) {
		c->isOpen = true;
		count++;
		count += setNeighbourZeros(board, x+1, y+1);
	}
	c = cellGet_checked_ref(board, x-1, y-1);
	if(c && c->value == 0) {
		c->isOpen = true;
		count++;
		count += setNeighbourZeros(board, x+1, y-1);
	}
	c = cellGet_checked_ref(board, x, y+1);
	if(c && c->value == 0) {
		c->isOpen = true;
		count++;
		count += setNeighbourZeros(board, x, y+1);
	}
	c = cellGet_checked_ref(board, x, y-1);
	if(c && c->value == 0) {
		c->isOpen = true;
		count++;
		count += setNeighbourZeros(board, x, y-1);
	}*/
	return count;
}

int countNeighbourMines(Board board, uint8_t x, uint8_t y) {
	int count = 0;
	if(cellGet_checked(board, x+1, y).isMine) count++;
	if(cellGet_checked(board, x+1, y+1).isMine) count++;
	if(cellGet_checked(board, x+1, y-1).isMine) count++;
	if(cellGet_checked(board, x-1, y).isMine) count++;
	if(cellGet_checked(board, x-1, y+1).isMine) count++;
	if(cellGet_checked(board, x-1, y-1).isMine) count++;
	if(cellGet_checked(board, x, y+1).isMine) count++;
	if(cellGet_checked(board, x, y-1).isMine) count++;
	return count;
}

Board boardCreate(uint8_t w, uint8_t h) {
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

void boardDestroy(Board *board) {
	free(board->cells);
	board->cells = NULL;
}

Cell cellGet_checked(Board board, uint8_t x, uint8_t y) {
	if(x >= board.w || y >= board.h)
		return (Cell){ 0,false,false };
	return board.cells[y * board.w + x];
}
Cell cellGet(Board board, uint8_t x, uint8_t y) {
	return board.cells[y * board.w + x];
}
Cell *cellGet_ref(const Board *board, uint8_t x, uint8_t y) {
	return &board->cells[y * board->w + x];
}
Cell *cellGet_checked_ref(const Board *board, uint8_t x, uint8_t y) {
	if(x >= board->w || y >= board->h)
		return NULL;
	return &board->cells[y * board->w + x];
}
void cellSet_checked(Board *board, uint8_t x, uint8_t y, Cell v) {
	if(x >= board->w || y >= board->h)
		return;
	board->cells[y * board->w + x] = v;
}
void cellSet(Board *board, uint8_t x, uint8_t y, Cell v) {
	board->cells[y * board->w + x] = v;
}

void boardPrint(Board board) {
	for(int i = 0; i < board.w + 2; i++)
		printf("-");
	puts("");
	for(int y = 0; y < board.h; y++) {
		printf("|");
		for(int x = 0; x < board.w; x++) {
			if((y*board.w + x) == board.cursorPtr) {
				printf("\033[1;41m\033[5m");
			}
			Cell c = cellGet(board, x, y);
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

void boardSave(const Board *board, const char *fname) {
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
			fwrite(cellGet_ref(board, x, y), 1, sizeof(Cell), file);
	}
	//fwrite(board, 1, sizeof(Board), file);
	fflush(file);
	fclose(file);
}

void boardLoad(Board *board, const char *fname) {
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
			fread(cellGet_ref(board, x, y), 1, sizeof(Cell), file);
	}
	fflush(file);
	fclose(file);
}

void boardCopy(Board *dest, Board b) {
	if(dest->h != b.h || dest->w != b.w)
		return;
	for(int i = 0; i < b.h * b.w; i++) {
		dest->cells[i] = b.cells[i];
	}
	//memcpy(dest->cells, b.cells, dest->h*dest->w);
}

