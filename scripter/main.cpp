#include "main.h"

using namespace code;

void setcode(requisit* v);

int main() {
	initialize();
	auto p = addtype("rect");
	p->add("x1", addtype("short"));
	p->add("y1", addtype("short"));
	p->add("x2", addtype("short"));
	p->add("y2", addtype("short"));
	//p->add("Points", p1->reference());
	//p->add("Width", addtype("Short"));
	//p->add("Height", addtype("Short"));
	p->update();
	auto p1 = addtype("point");
	p1->add("x", addtype("short"));
	p1->add("y", addtype("short"));
	p1->update();
	auto p2 = addtype("filename");
	p2->add("name", addtype("char"))->setcount(260);
	p2->update();
	auto p4 = addtype("item");
	p4->add("id", addtype("text"));
	p4->add("name", addtype("text"));
	p4->update();
	auto p3 = addtype("character");
	p3->add("master", p3->reference());
	p3->add("ability", addtype("char"))->setcount(6);
	p3->add("wears", addtype("item"))->setcount(8);
	p3->update();
	p3->write("character.mtd");
	//p->write("rect.mtd");
	initialize();
	//p->read("rect.mtd");
	p->read("character.mtd");
	//auto r = p4->add("Add", addtype("Void"));
	//r->set(new expression(Add, new expression(10), new expression(12)));
	logmsg("Размер метаданных %1i", sizeof(metadata));
	logmsg("Размер реквизита %1i", sizeof(requisit));
	setcode(0);
	run_main();
}

int __stdcall WinMain(void* ci, void* pi, char* cmd, int sw) {
	return main();
}