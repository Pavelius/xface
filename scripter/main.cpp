#include "main.h"

int main() {
	auto p = addtype("Rect");
	auto p1 = addtype("Point");
	p->add("x1", int_meta);
	p->add("y1", int_meta);
	p->add("x2", int_meta);
	p->add("y2", int_meta);
	p->add("points", p1->reference());
	p->add("width", int_meta);
	p->add("height", int_meta);
	p1->add("x", int_meta);
	p1->add("y", int_meta);
	logmsg("Размер метаданных %1i", sizeof(metadata));
	logmsg("Размер реквизита %1i", sizeof(requisit));
	run_main();
}

int __stdcall WinMain(void* ci, void* pi, char* cmd, int sw) {
	return main();
}