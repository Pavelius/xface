#include "main.h"

using namespace code;

void setcode(requisit* v);

int main() {
	metadata::initialize();
	auto p = config.types.add("Rect");
	auto p1 = config.types.add("Point");
	auto p2 = config.types.add("FileName");
	auto p3 = config.types.add("Character");
	auto p4 = config.types.add("Item");
	auto r1 = p->add("x1", config.types.find("Short"));
	p->add("y1", config.types.find("Short"));
	auto r2 = p->add("x2", config.types.find("Short"));
	p->add("y2", config.types.find("Short"));
	p->add("Points", config.types.reference(p1));
	p->add("Width", config.types.find("Short"));
	p->add("Height", config.types.find("Short"));
	p->update();
	p1->add("x", config.types.find("Short"));
	p1->add("y", config.types.find("Short"));
	p1->update();
	p2->add("Name", config.types.find("Char"))->setcount(260);
	p2->update();
	p3->add("Master", config.types.reference(p3));
	p3->add("Ability", config.types.find("Char"))->setcount(6);
	p3->add("Wears", config.types.find("Item"))->setcount(8);
	p3->update();
	p4->add("Id", config.types.find("Text"));
	p4->add("Name", config.types.find("Text"));
	p4->update();
	auto r = p4->add("Add", config.types.find("Void"));
	//p3->write("character.mtp");
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