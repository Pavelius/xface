#include "main.h"

using namespace code;

void setcode(requisit* v);

static bool test_metadata() {
	auto u = sizeof(meta_size<decltype(metadata::type)>::value);
	auto u1 = sizeof(meta_size<metadata*>::value);
	return u == sizeof(unsigned);
}

int main() {
	if(!test_metadata())
		return -1;
	initialize();
	auto p = addtype("rect");
	p->add("x1", addtype("i16"));
	p->add("y1", addtype("i16"));
	p->add("x2", addtype("i16"));
	p->add("y2", addtype("i16"));
	//p->add("Points", p1->reference());
	p->addm("height", addtype("i32"));
	p->addm("width", addtype("i32"));
	p->update();
	auto p1 = addtype("point");
	p1->add("x", addtype("i16"));
	p1->add("y", addtype("i16"));
	p1->update();
	auto p2 = addtype("file");
	p2->add("name", addtype("u8"))->setcount(260);
	p2->update();
	auto p4 = addtype("item");
	p4->add("id", addtype("text"))->set(Public)->set(Dimension);
	p4->add("Name", addtype("text"));
	p4->update();
	auto p3 = addtype("character");
	p3->add("master", p3->reference());
	p3->add("ability", addtype("i8"))->setcount(6);
	p3->add("wears", addtype("item"))->setcount(8)->set(Public);
	p3->addm("new", p3)->set(Static)->set(Public);
	p3->update();
	//p3->write("character.mtd");
	//p->write("rect.mtd");
	//initialize();
	//p->read("rect.mtd");
	//p->read("character.mtd");
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