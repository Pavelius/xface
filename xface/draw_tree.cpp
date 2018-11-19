#include "crt.h"
#include "draw_grid.h"

using namespace draw::controls;

#define TIGroup 1

int	tree::find(const void* value) const {
	auto count = getcount();
	for(unsigned i = 0; i < count; i++) {
		if(((element*)array::get(i))->object==value)
			return i;
	}
	return -1;
}

void* tree::get(int index) const {
	return (void*)((element*)array::get(index))->object;
}

int tree::getlevel(int index) const {
	return ((element*)array::get(index))->level;
}

int	tree::getimage(int index) const {
	return ((element*)array::get(index))->image;
}

int	tree::gettype(int index) const {
	return ((element*)array::get(index))->type;
}

int tree::getroot(int index) const {
	while(true) {
		auto parent = getparent(index);
		if(parent == -1)
			return index;
		index = parent;
	}
}

int	tree::getnumber(int line, int column) const {
	if(columns[column].id) {
		if(strcmp(columns[column].id, "image") == 0)
			return getimage(line);
		else if(strcmp(columns[column].id, "type") == 0)
			return gettype(line);
		else if(strcmp(columns[column].id, "level") == 0)
			return getlevel(line);
	}
	return grid::getnumber(line, column);
}

int tree::getparent(int index) const {
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

int tree::getlastchild(int index) const {
	int level = getlevel(index++) + 1;
	int i2 = getmaximum();
	while(index < i2) {
		if(level != getlevel(index))
			break;
		index++;
	}
	return index;
}

bool tree::isgroup(int index) const {
	return (((element*)array::get(index))->flags&TIGroup) != 0;
}

void tree::toggle(int index) {
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

void tree::collapse(int i) {
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
		if(current > (int)i)
			current--;
		c--;
	}
}

void tree::open(int max_level) {
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

//void tree::expand(int index, int level) {
//	this->index = index;
//	this->level = level;
//	expanding();
//	if(this->index != index) {
//		auto p = (element*)rows.get(index);
//		p->flags |= TIGroup;
//	}
//	// Remove unused rows
//	unsigned i1 = this->index + 1;
//	unsigned i2 = i1;
//	while(i2<rows.getcount()) {
//		auto p2 = (element*)rows.get(i2);
//		if(p2->level >= (level+1))
//			i2++;
//		else
//			break;
//	}
//	if(i1 != i2)
//		rows.remove(i1, i2 - i1);
//	// Finally sort all rows
//	if(sort_rows_by_name) {
//		if(level == 0)
//			amem::sort(0, getcount() - 1, group_sort_up ? compare_by_name_group_up : compare_by_name, this);
//		else
//			amem::sort(index + 1, this->index, group_sort_up ? compare_by_name_group_up : compare_by_name, this);
//	}
//}
//
//void tree::addrow(tree::element& e) {
//	e.level = level + 1;
//	if((unsigned)(index + 1) < rows.getcount()) {
//		auto p = (element*)rows.get(index + 1);
//		if(p->level >= e.level) {
//			memcpy(p, &e, getsize());
//			index++;
//			return;
//		}
//	}
//	if(level == 0)
//		collection::add(&e);
//	else
//		amem::insert(++index, &e);
//}

void tree::add(void* object, unsigned char level, unsigned char image, unsigned char type, unsigned char flags) {
	element e;
	e.object = object;
	e.image = image;
	e.flags = flags;
	e.level = level;
	e.type = type;
	array::add(&e);
}

//void tree::addrow(void* object) {
//	if(getsize() != sizeof(element))
//		return;
//	element e = {0};
//	e.param = (int)object;
//	rows.add(&e);
//}