#include "draw_control.h"

using namespace draw;

anyval cmd::current;

void cmd::apply_xor() {
	if(!hot.param)
		hot.param = 1;
	auto v = current.get() ^ hot.param;
	current.set(v);
}

void cmd::apply_set() {
	current.set(hot.param);
}

void cmd::apply_add() {
	current.set(current.get() + hot.param);
}