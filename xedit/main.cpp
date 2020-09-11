#include "main.h"
#include "code_view.h"

using namespace draw;

void logmsg(const char* format, ...);

int main() {
	lexer e;
	e.add("fn", Keyword);
	e.add("mod", Keyword);
	e.add("for", Keyword);
	e.add("while", Keyword);
	e.add(":", Operator);
	e.add("(", OpenParam); e.add(")", CloseParam);
	e.add("{", OpenBlock); e.add("}", CloseBlock);
	e.add("[", OpenScope); e.add("]", CloseScope);
	e.sort();
	codepos cp = {};
	codemodel cm;
	cm.set(e);
	cm.set("fn print(text : String) {}");
	while(true) {
		cm.getnext(cp);
		if(cp.from == cp.to)
			break;
		cp.from = cp.to;
	}
	application("X-Code editor", true, 0, 0, 0);
	return 0;
}

int __stdcall WinMain(void* ci, void* pi, char* cmd, int sw) {
	return main();
}