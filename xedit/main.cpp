#include "main.h"
#include "codeview.h"

using namespace draw;
using namespace draw::controls;

void logmsg(const char* format, ...);

static draw::controls::control* getwindow(const char* id) {
	auto p = control::plugin::find(id);
	if(!p)
		return 0;
	return &const_cast<control::plugin*>(p)->getcontrol();
}

static void setglob(const char* control_id, const char* id, int value) {
	auto p = getwindow(control_id);
	if(!p)
		return;
	p->setvalue(id, value);
}

int main() {
	codeview::instance();
	lexer e;
	e.add("fn", Keyword);
	e.add("mod", Keyword);
	e.add("mut", Keyword);
	e.add("for", Keyword);
	e.add("in", Keyword);
	e.add("if", Keyword);
	e.add("return", Keyword);
	e.add("while", Keyword);
	e.add("impl", Keyword);
	e.add("struct", Keyword);
	e.add("let", Keyword);
	e.add(":", Operator);
	e.add(";", Operator);
	e.add("..", Operator);
	e.add(",", Operator);
	e.add("->", Operator);
	e.add("=", Operator);
	e.add("(", OpenParam); e.add(")", CloseParam);
	e.add("{", OpenBlock); e.add("}", CloseBlock);
	e.add("[", OpenScope); e.add("]", CloseScope);
	e.sort();
	codepos cp = {};
	//setglob("codeview", "text", (int)"fn print(text : String) {\n  let a = 10;\n  a\n}");
	setglob("codeview", "open", (int)"code_sample.txt");
	setglob("codeview", "lex", (int)&e);
	setglob("codeview", "select", 100);
	setglob("codeview", "select_range", 200);
	application("X-Code editor", true, 0, 0, 0);
	return 0;
}

int __stdcall WinMain(void* ci, void* pi, char* cmd, int sw) {
	return main();
}