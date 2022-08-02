#include "act.h"
#include "board.h"
#include "common.h"
#include <signal.h>
#include <stdio.h>

const ActFn ActLUT[14] = {
	quit, help, none, err, unknown,
	up, down, left, right, open, mine,
	save, load,
	dprint
};

// TODO expand
const char *cmdHelp =
	"Minesweeper - command help\n"
	"\tH,?: help\n"
	"\tq: quit\n";

bool quit(Board realB, Board *virt, void *arg) {
	raise(SIGINT);
	return false;
}
bool help(Board realB, Board *virt, void *arg) {
	printf("%s", cmdHelp);
	return false;;
}
bool none(Board realB, Board *virt, void *arg) {
	return true;
}
bool err(Board realB, Board *virt, void *arg) {
	fprintf(stderr, "ERROR: error while getting input\n");
	return true;
}
bool unknown(Board realB, Board *virt, void *arg) {
	fprintf(stderr, "ERROR: Unknown command\n");
	printf("%s", cmdHelp);
	return true;
}

bool up(Board realB, Board *virt, void *arg) {
	virt->cursorPtr -= virt->w;
	return false;
}
bool down(Board realB, Board *virt, void *arg) {
	virt->cursorPtr += virt->w;
	return false;
}
bool left(Board realB, Board *virt, void *arg) {
	virt->cursorPtr--;
	return false;
}
bool right(Board realB, Board *virt, void *arg) {
	virt->cursorPtr++;
	return false;
}
bool open(Board realB, Board *virt, void *arg) {
	Cell *curc = &virt->cells[virt->cursorPtr];
	if(curc->isOpen) {
		// TODO
	} else { // not open
		if(curc->isMine) {
			printf("GAME OVER\n");
			quit(realB, virt, arg);
		} else if(curc->value == 0) {
			setNeighbourZeros(realB, virt,
				virt->cursorPtr % virt->h,
				virt->cursorPtr / virt->h, true);
		} else {
			curc->isOpen = true;
		}
	}
	return false;
}
bool mine(Board realB, Board *virt, void *arg) {
	//if(virt->cells[virt->cursorPtr].isOpen)
		// TODO
	return false;
}

bool save(Board realB, Board *virt, void *arg) {
	boardSave((Board *)arg, "save");
	return false;
}
bool load(Board realB, Board *virt, void *arg) {
	boardLoad((Board *)arg, "save");
	return false;
}

bool dprint(Board realB, Board *virt, void *arg) {
	for(int y = 0; y < virt->h; y++) {
		for(int x = 0; x < virt->w; x++) {
			Cell c = cellGet(*virt, x, y);
			if(c.isMine) {
				printf("%c", MINE);
			} else {
				printf("%d", cellGet(*virt, x, y).value);
			}
		}
		puts("");
	}
	Cell c = virt->cells[virt->cursorPtr];
	printf("At cursPtr: loop:%d mine:%d open:%d val:%d\n", c.isLooped,c.isMine, c.isOpen, c.value);
	return false;
}

