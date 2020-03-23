#include "crt.h"
#include "draw_control.h"

using namespace draw;
using namespace draw::controls;

#define TIGroup 1

void* tableref::builder::add(void* object, unsigned char image, unsigned char type, bool group) {
	element e;
	e.object = object;
	e.image = image;
	e.flags = group ? TIGroup : 0;
	e.type = type;
	e.level = level + 1;
	if(pc->getcount() == 0 || (unsigned)index > pc->getcount())
		return pc->array::add(&e);
	return pc->array::insert(++index, &e);
}

void tableref::addref(void* object) {
	auto p = (element*)array::add();
	memset(p, 0, sizeof(*p));
	p->object = object;
}

int	tableref::find(const void* value) const {
	auto count = getcount();
	for(unsigned i = 0; i < count; i++) {
		if(((element*)array::ptr(i))->object == value)
			return i;
	}
	return -1;
}

void* tableref::get(int index) const {
	return (void*)((element*)array::ptr(index))->object;
}

int tableref::getlevel(int index) const {
	return ((element*)array::ptr(index))->level;
}

int	tableref::getimage(int index) const {
	return ((element*)array::ptr(index))->image;
}

int	tableref::gettype(int index) const {
	return ((element*)array::ptr(index))->type;
}

int tableref::getroot(int index) const {
	while(true) {
		auto parent = getparent(index);
		if(parent == -1)
			return index;
		index = parent;
	}
}

//int tableref::getnumber(int line, int column) const {
//	if(columns[column].id) {
//		if(strcmp(columns[column].id, "image") == 0)
//			return getimage(line);
//		else if(strcmp(columns[column].id, "type") == 0)
//			return gettype(line);
//		else if(strcmp(columns[column].id, "level") == 0)
//			return getlevel(line);
//	}
//	return 0;
//}

int tableref::getparent(int index) const {
	int level = getlevel(index);
	while(index) {
		if(level > getlevel(index))
			return index;
		index--;
	}
	if(level > getlevel(index))
		return index;
	return -1;
}

int tableref::getblockcount(int index) const {
	auto start = index;
	auto level = getlevel(index++);
	auto index_last = getmaximum();
	while(index < index_last) {
		if(level >= getlevel(index))
			break;
		index++;
	}
	return index - start;
}

bool tableref::isgroup(int index) const {
	return (((element*)array::ptr(index))->flags&TIGroup) != 0;
}

void tableref::toggle(int index) {
	if(!isgroup(index))
		return;
	auto level = getlevel(index);
	auto mm = getmaximum();
	auto cc = current;
	if(isopen(index))
		collapse(index);
	else
		expand(index, level);
	if(cc > index) {
		if(mm < getmaximum())
			current += getmaximum() - mm;
	}
}

void tableref::collapse(int i) {
	// если выбранный элемент входит в сворачиваемю область
	// переместим его наверх
	auto w = current;
	while(w != -1) {
		if(w == i)
			break;
		w = getparent(w);
	}
	int m = getlevel(i);
	int c = getmaximum();
	while((int)i < c) {
		if(i + 1 >= c || getlevel(i + 1) <= m)
			break;
		array::remove(i + 1, 1);
		if(current >(int)i)
			current--;
		c--;
	}
}

void tableref::open(int max_level) {
	for(int level = 1; level <= max_level; level++) {
		bool need_test = true;
		while(need_test) {
			need_test = false;
			auto c = array::getcount();
			for(unsigned i = 0; i < c; i++) {
				if(level != getlevel(i))
					continue;
				if(i < c - 1) {
					if(getlevel(i + 1) > level)
						continue;
				}
				if(isgroup(i)) {
					auto i1 = array::getcount();
					expand(i, level);
					if(i1 < array::getcount()) {
						need_test = true;
						break;
					}
				}
			}
		}
	}
}

void tableref::expand(int index, int level) {
	builder e(this, index, level);
	expanding(e);
	if(e.index != index) {
		auto p = (element*)array::ptr(index);
		p->flags |= TIGroup;
	}
	// Remove unused rows
	unsigned i1 = e.index + 1;
	unsigned i2 = i1;
	while(i2 < array::getcount()) {
		auto p2 = (element*)array::ptr(i2);
		if(p2->level >= (level + 1))
			i2++;
		else
			break;
	}
	if(i1 != i2)
		array::remove(i1, i2 - i1);
	// Finally sort all rows
	if(sort_rows_by_name) {
		//	if(level == 0)
		//		amem::sort(0, getcount() - 1, group_sort_up ? compare_by_name_group_up : compare_by_name, this);
		//	else
		//		amem::sort(index + 1, this->index, group_sort_up ? compare_by_name_group_up : compare_by_name, this);
	}
}

bool tableref::treemarking(bool run) {
	toggle(hot.param);
	return true;
}

bool tableref::keyinput(unsigned id) {
	switch(id) {
	case KeyLeft:
		if(gettreecolumn() == current_column) {
			if(isopen(current)) {
				toggle(current);
				break;
			}
			auto parent = getparent(current);
			if(parent != current) {
				select(parent, current_column);
				break;
			}
		}
		return table::keyinput(id);
	case KeyRight:
		if(gettreecolumn() != current_column || isopen(current))
			return table::keyinput(id);
		toggle(current);
		break;
	default: return table::keyinput(id);
	}
	return true;
}

int	tableref::gettreecolumn() const {
	return getvalid(0, 1);
}

int	tableref::getnext(int index, int increment) const {
	auto i = index;
	auto level = getlevel(i);
	auto maximum = getmaximum();
	while(true) {
		auto n = i + increment;
		if((n < 0) || (n >= maximum))
			return index;
		auto m = getlevel(n);
		if(m == level)
			return n;
		if(m < level)
			return index;
		i = n;
	}
}

void tableref::shift(int i1, int i2) {
	auto c1 = getblockcount(i1);
	auto c2 = getblockcount(i2);
	array::shift(i1, i2, c1, c2);
	if(i1 < i2)
		select(i1, current_column);
	else
		select(i2 + c1, current_column);
}

bool tableref::moveup(bool run) {
	if(no_change_order)
		return false;
	if(current <= 0)
		return false;
	if(getmaximum() == 1)
		return false;
	if(run) {
		auto new_current = getnext(current, -1);
		if(new_current == current)
			return false;
		if(run)
			shift(new_current, current);
		//swap(current - 1, current);
		//select(current - 1, getcolumn());
	}
	return true;
}

bool tableref::movedown(bool run) {
	if(no_change_order)
		return false;
	if(current >= int(getmaximum() - 1))
		return false;
	if(run) {
		auto new_current = getnext(current, 1);
		if(new_current == current)
			return false;
		if(run)
			shift(new_current, current);
		//swap(current + 1, current);
		//select(current + 1, getcolumn());
	}
	return true;
}

bool tableref::remove(bool run) {
	if(read_only)
		return false;
	if(no_change_count)
		return false;
	if(!getcount())
		return false;
	if(run)
		array::remove(current, 1);
	return true;
}

bool tableref::sortas(bool run) {
	if(no_change_order)
		return false;
	if(getmaximum() <= 1)
		return false;
//	if(run)
//		sort(current_column, true);
	return true;
}

bool tableref::sortds(bool run) {
	if(no_change_order)
		return false;
	if(getmaximum() <= 1)
		return false;
//	if(run)
//		sort(current_column, false);
	return true;
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

void tableref::swap(int i1, int i2) {
	array::swap(i1, i2);
}