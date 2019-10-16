#include "draw_control.h"

using namespace draw;

//struct cmd {
//	eventproc				proc;
//	int						param;
//	anyval					value;
//	static cmd				ctx;
//	constexpr cmd() : value(), proc(0), param(0) {}
//	constexpr cmd(eventproc proc) : value(proc, 0), proc(proc), param(0) {}
//	constexpr cmd(eventproc proc, int param) : value(proc, 0), proc(proc), param(param) {}
//	constexpr cmd(eventproc proc, int param, void* data, unsigned size) : value(data, size), proc(proc), param(param) {}
//	constexpr cmd(eventproc proc, int param, const anyval& value) : value(value), proc(proc), param(param) {}
//	constexpr cmd(commandproc proc, void* source) : value(proc, 0), proc(calling), param((int)source) {}
//	static void				add();
//	static void				assign();
//	static void				calling();
//	static void				invert();
//	bool					ischecked() const { return ((value.get()&param) != 0) ? true : false; }
//	void					execute() const { ctx = *this; draw::execute(proc, param); }
//	int						getid() const { return (int)value.data; }
//};

cmd cmd::ctx;

void cmd::calling() {
	((commandproc)ctx.value.data)((void*)ctx.param);
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