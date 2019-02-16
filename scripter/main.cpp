#include "main.h"

int main() {
	auto p = addtype("rect");
	auto p1 = addtype("point");
	p->add("x1", int_meta);
	p->add("y1", int_meta);
	p->add("x2", int_meta);
	p->add("y2", int_meta);
	p->add("points", p1->reference());
}

int __stdcall WinMain(void* ci, void* pi, char* cmd, int sw) {
	return main();
}