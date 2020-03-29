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
//	static command add_elements[] = {{"change", "Изменить", 10, F2, &grid::change},
//	{"remove", "Удалить", 19, KeyDelete, &tree::remove},
//	{}};
//	static command move_elements[] = {{"moveup", "Переместить вверх", 21, 0, &tree::moveup},
//	{"movedown", "Переместить вверх", 22, 0, &tree::movedown},
//		//{"sortas", "Сортировать по возрастанию", 11, 0, &grid::sortas},
//		//{"sortas", "Сортировать по убыванию", 12, 0, &grid::sortds},
//	{}};
//	static command elements[] = {{add_elements},
//	{move_elements},
//	{"setting", "Настройки", 16, 0, &grid::setting},
//	{}};
//	return elements;
//}