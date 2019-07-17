#include "main.h"

using namespace code;

void setcode(requisit* v);

int main() {
	metadata::initialize();
	auto p = addtype("Rect");
	auto p1 = addtype("Point");
	auto p2 = addtype("FileName");
	auto p3 = addtype("Character");
	auto p4 = addtype("Item");
	auto r1 = p->add("x1", "Short");
	p->add("y1", "Short");
	auto r2 = p->add("x2", "Short");
	p->add("y2", "Short");
	p->add("Points", p1->reference());
	p->add("Width", "Short");
	p->add("Height", "Short");
	p1->add("x", "Short");
	p1->add("y", "Short");
	p2->add("Name", "Char")->setcount(260);
	p3->add("Ability", "Char")->setcount(6);
	p3->add("Wears", p4)->setcount(8);
	p4->add("Id", "Text");
	p4->add("Name", "Text");
	auto r = p4->add("Add", "Void");
	p3->write("character.mtp");
	r->set(new expression(Add, new expression(10), new expression(12)));
	//logmsg("Размер метаданных %1i", sizeof(metadata));
	//logmsg("Размер реквизита %1i", sizeof(requisit));
	auto c1 = new expression(Return, new expression(Sub, new expression(r1), new expression(r2)));
	auto c2 = new expression(Return, new expression(szdup("Test string")));
	c1->add(c2);
	auto rt = p->find("Width");
	rt->set(c1);
	setcode(rt);
	run_main();
}

int __stdcall WinMain(void* ci, void* pi, char* cmd, int sw) {
	return main();
}