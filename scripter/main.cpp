#include "main.h"

using namespace code;

void setcode(requisit* v);

int main() {
	initialize();
	auto p = addtype("Rect");
	auto p1 = addtype("Point");
	auto p2 = addtype("FileName");
	auto p3 = addtype("Character");
	auto p4 = addtype("Item");
	auto r1 = p->add("x1", addtype("Short"));
	p->add("y1", addtype("Short"));
	p->add("x2", addtype("Short"));
	p->add("y2", addtype("Short"));
	//p->add("Points", p1->reference());
	//p->add("Width", addtype("Short"));
	//p->add("Height", addtype("Short"));
	p->update();
	p1->add("x", addtype("Short"));
	p1->add("y", addtype("Short"));
	p1->update();
	p2->add("Name", addtype("Char"))->setcount(260);
	p2->update();
	p3->add("Master", p3->reference());
	p3->add("Ability", addtype("Char"))->setcount(6);
	p3->add("Wears", addtype("Item"))->setcount(8);
	p3->update();
	p4->add("Id", addtype("Text"));
	p4->add("Name", addtype("Text"));
	p4->update();
	//p3->write("character.mtd");
	auto r = p4->add("Add", addtype("Void"));
	r->set(new expression(Add, new expression(10), new expression(12)));
	logmsg("Размер метаданных %1i", sizeof(metadata));
	logmsg("Размер реквизита %1i", sizeof(requisit));
	setcode(0);
	run_main();
}

int __stdcall WinMain(void* ci, void* pi, char* cmd, int sw) {
	return main();
}