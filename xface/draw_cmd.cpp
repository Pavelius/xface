#include "draw_control.h"

using namespace draw;

cmd cmd::ctx;

void cmd::calling() {
	((markup::command_type)ctx.data)((void*)ctx.param);
}

void cmd::invert() {
	if(!ctx.param)
		ctx.param = 1;
	auto v = ctx.get() ^ ctx.param;
	ctx.set(v);
}

void cmd::assign() {
	ctx.set(ctx.param);
}

void cmd::add() {
	ctx.set(ctx.get() + ctx.param);
}