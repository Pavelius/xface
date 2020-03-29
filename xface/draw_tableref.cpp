#include "draw_control.h"

using namespace draw::controls;

void* tableref::addref(void* object) {
	auto p = (element*)array::add();
	memset(p, 0, sizeof(*p));
	p->object = object;
	return p;
}

int	tableref::find(const void* value) const {
	auto count = getcount();
	for(unsigned i = 0; i < count; i++) {
		if(((element*)array::ptr(i))->object == value)
			return i;
	}
	return -1;
}

//const control::command* tree::getcommands() const {
//	static command add_elements[] = {{"change", "��������", 10, F2, &grid::change},
//	{"remove", "�������", 19, KeyDelete, &tree::remove},
//	{}};
//	static command move_elements[] = {{"moveup", "����������� �����", 21, 0, &tree::moveup},
//	{"movedown", "����������� �����", 22, 0, &tree::movedown},
//		//{"sortas", "����������� �� �����������", 11, 0, &grid::sortas},
//		//{"sortas", "����������� �� ��������", 12, 0, &grid::sortds},
//	{}};
//	static command elements[] = {{add_elements},
//	{move_elements},
//	{"setting", "���������", 16, 0, &grid::setting},
//	{}};
//	return elements;
//}