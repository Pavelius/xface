#include "main.h"

using namespace code;

void setcode(requisit* v);

int main() {
	auto p = addtype("Rect");
	auto p1 = addtype("Point");
	auto p2 = addtype("FileName");
	auto p3 = addtype("Character");
	auto p4 = addtype("Item");
	auto r1 = p->add("x1", int_meta);
	p->add("y1", int_meta);
	auto r2 = p->add("x2", int_meta);
	p->add("y2", int_meta);
	p->add("Points", p1->reference());
	p->add("Width", int_meta)->set(new expression(Return, new expression(Sub, new expression(r1), new expression(r2))));
	p->add("Height", int_meta);
	p1->add("x", sint_meta);
	p1->add("y", sint_meta);
	p2->add("Name", char_meta)->setcount(260);
	p3->add("Ability", char_meta)->setcount(6);
	p3->add("Wears", p4)->setcount(8);
	p4->add("Id", text_meta);
	p4->add("Name", text_meta);
	auto r = p4->add("Add", void_meta);
	r->set(new expression(Add, new expression(10), new expression(12)));
	//logmsg("Размер метаданных %1i", sizeof(metadata));
	//logmsg("Размер реквизита %1i", sizeof(requisit));
	setcode(p->find("Width"));
	run_main();
}

int __stdcall WinMain(void* ci, void* pi, char* cmd, int sw) {
	return main();
}