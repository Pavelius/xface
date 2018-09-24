#include "draw.h"

#pragma once

namespace res {
struct element {
	const char*		name;
	const char*		folder;
	sprite*			data;
	bool			notfound;
};
void				cleanup();
extern element		elements[];
const char*			getname(int rid);
}
sprite*				gres(int rid);
