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
	if(false) {
		auto p = addtype("Rect");
		p->add("x1", addtype("i16"));
		p->add("y1", addtype("i16"));
		p->add("x2", addtype("i16"));
		p->add("y2", addtype("i16"));
		//p->add("Points", p1->reference());
		p->addm("Height", addtype("i32"));
		p->addm("Width", addtype("i32"));
		p->update();
		auto p1 = addtype("Point");
		p1->add("x", addtype("i16"));
		p1->add("y", addtype("i16"));
		p1->update();
		auto p2 = addtype("File");
		p2->add("Name", addtype("u8"))->setcount(260);
		p2->update();
		auto p4 = addtype("Item");
		p4->add("id", p4->type_text)->set(Public)->set(Dimension);
		p4->add("Name", p4->type_text);
		p4->update();
		auto p3 = addtype("Character");
		p3->add("Master", p3->reference());
		p3->add("Abilities", addtype("i8"))->setcount(6);
		p3->add("Wears", addtype("Item"))->setcount(8)->set(Public);
		p3->add("Elements", addtype("Character"))->setcount(256)->set(Static);
		p3->addm("New", p3)->set(Static)->set(Public);
		p3->update();
		p3->write("character.mtd");
	} else
		metadata::read("character.mtd");
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