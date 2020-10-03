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
		auto p1 = addtype("Point");
		p1->add("x", metadata::type_i16)->set(Public);
		p1->add("y", metadata::type_i16)->set(Public);
		p1->update();
		auto p = addtype("Rect");
		p->add("x1", metadata::type_i16)->set(Public);
		p->add("y1", metadata::type_i16)->set(Public);
		p->add("x2", metadata::type_i16)->set(Public);
		p->add("y2", metadata::type_i16)->set(Public);
		p->addm("Height", metadata::type_i32)->set(Public);
		p->addm("Width", metadata::type_i32)->set(Public);
		p->update();
		auto p2 = addtype("File");
		p2->add("Name", metadata::type_i8)->setcount(260);
		p2->update();
		auto p4 = addtype("Item");
		p4->add("id", metadata::type_text)->set(Public)->set(Dimension);
		p4->add("Name", metadata::type_text)->set(Public);
		p4->update();
		auto p3 = addtype("Character");
		p3->add("Name", metadata::type_text)->set(Dimension);
		p3->add("Master", p3->reference());
		p3->add("Enemy", p3->reference());
		p3->add("Abilities", metadata::type_i8)->setcount(6);
		p3->add("Wears", addtype("Item"))->setcount(8);
		p3->add("Elements", addtype("Character"))->setcount(256)->set(Static);
		p3->addm("New", p3)->set(Static)->set(Public);
		p3->update();
		p->write("rect.mtd");
		p3->write("character.mtd");
		p2->write("file.mtd");
		p1->write("point.mtd");
		p4->write("item.mtd");
	} /*else
		metadata::read("character.mtd");*/
	logmsg("Размер метаданных %1i", sizeof(metadata));
	logmsg("Размер реквизита %1i", sizeof(requisit));
	setcode(0);
	run_main();
}

int __stdcall WinMain(void* ci, void* pi, char* cmd, int sw) {
	return main();
}