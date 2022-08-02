#ifndef _NTR_ACT_H_
#define _NTR_ACT_H_ 1
#include "board.h"

enum ActType {
	C_QUIT,
	C_HELP,
	C_NONE,
	C_ERR,
	C_UNKNOWN,

	C_UP,
	C_DOWN,
	C_LEFT,
	C_RIGHT,
	C_OPEN,
	C_MINE,

	C_SAVE,
	C_LOAD,

	C_DPRINT
};

bool quit(Board realB, Board *virt, void *arg);
bool help(Board realB, Board *virt, void *arg);
bool none(Board realB, Board *virt, void *arg);
bool err(Board realB, Board *virt, void *arg);
bool unknown(Board realB, Board *virt, void *arg);
bool up(Board realB, Board *virt, void *arg);
bool down(Board realB, Board *virt, void *arg);
bool left(Board realB, Board *virt, void *arg);
bool right(Board realB, Board *virt, void *arg);
bool open(Board realB, Board *virt, void *arg);
bool mine(Board realB, Board *virt, void *arg);
bool save(Board realB, Board *virt, void *arg);
bool load(Board realB, Board *virt, void *arg);
bool dprint(Board realB, Board *virt, void *arg);

typedef bool (*ActFn)(Board realB, Board *virt, void *arg);
extern const ActFn ActLUT[14];

#endif//_NTR_ACT_H_

