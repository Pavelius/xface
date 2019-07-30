#include "draw_control.h"

using namespace draw;

cmd::contexti cmd::ctx;

void cmd::apply_xor() {
	if(!hot.param)
		hot.param = 1;
	auto v = ctx.value.get() ^ hot.param;
	ctx.value.set(v);
}

void cmd::apply_set() {
	ctx.value.set(hot.param);
}

void cmd::apply_add() {
	ctx.value.set(ctx.value.get() + hot.param);
}