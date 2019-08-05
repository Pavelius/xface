#include "draw_control.h"

using namespace draw;

cmd cmd::ctx;

void cmd::calling() {
	((markup::commandproc)ctx.value.data)((void*)ctx.param);
}

void cmd::invert() {
	if(!ctx.param)
		ctx.param = 1;
	auto v = ctx.value.get() ^ ctx.param;
	ctx.value.set(v);
}

void cmd::assign() {
	ctx.value.set(ctx.param);
}

void cmd::add() {
	ctx.value.set(ctx.value.get() + ctx.param);
}