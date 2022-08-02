#ifndef _NTR_BOARD_H_
#define _NTR_BOARD_H_ 1
#include <stdint.h>
#include <stdbool.h>

typedef struct {
	uint8_t value;
	bool isMine;
	bool isOpen;
	bool isLooped;
} Cell;

typedef struct {
	uint8_t w, h;
	int cursorPtr;
	Cell *cells;
	//char *cells;
} Board;

Board boardCreate(uint8_t w, uint8_t h);
void boardDestroy(Board *board);

// Board manipulation
Cell cellGet(Board board, uint8_t x, uint8_t y);
Cell cellGet_checked(Board board, uint8_t x, uint8_t y);
Cell *cellGet_ref(const Board *board, uint8_t x, uint8_t y);
Cell *cellGet_checked_ref(const Board *board, uint8_t x, uint8_t y);
void cellSet(Board *board, uint8_t x, uint8_t y, Cell v);
void cellSet_checked(Board *board, uint8_t x, uint8_t y, Cell v);

void boardPrint(Board board);
void spawnMines(Board *board, uint8_t count);
int countNeighbourMines(Board board, uint8_t x, uint8_t y);
int setNeighbourZeros(Board realB, Board *board, int8_t x, int8_t y, bool isFirstCall);
void boardClear(Board *board);
void boardCopy(Board *dest, Board b);
// saves board to file
void boardSave(const Board *board, const char *fname);
// loads board from file
void boardLoad(Board *board, const char *fname);

#endif /* ifndef _NTR_BOARD_H_ */

