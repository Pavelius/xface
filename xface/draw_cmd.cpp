#include "draw_control.h"

using namespace draw;

cmd cmd::ctx;

void cmd::apply_xor() {
	if(!ctx.param)
		ctx.param = 1;
	auto v = ctx.get() ^ ctx.param;
	ctx.set(v);
}

void cmd::apply_set() {
	ctx.set(ctx.param);
}

void cmd::apply_add() {
	ctx.set(ctx.get() + ctx.param);
}