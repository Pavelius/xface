#include "draw_control.h"

using namespace draw;
using namespace draw::controls;

#define TIGroup 1

int tree::getlevel(int index) const {
	return ((element*)array::ptr(index))->level;
}

int	tree::getimage(int index) const {
	return ((element*)array::ptr(index))->image;
}

int	tree::gettype(int index) const {
	return ((element*)array::ptr(index))->type;
}

bool tree::isgroup(int index) const {
	return (((element*)array::ptr(index))->flags&TIGroup) != 0;
}

tree::element* tree::insert(int& index, int level) {
	auto p = (element*)array::insert(index++, 0);
	p->level = level;
	return p;
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

void tree::expand(int index) {
	auto before_count = getmaximum();
	if(before_count == 0)
		expanding(0, 1);
	else
		expanding(index + 1, getlevel(index) + 1);
	auto after_count = getmaximum();
	if(before_count && before_count != after_count) {
		// Some rows where added
		auto p = (element*)array::ptr(index);
		p->setgroup(true);
	}
}

void tree::swap(int i1, int i2) {
	array::swap(i1, i2);
}

//bool tableref::keyinput(unsigned id) {
//	switch(id) {
//	case KeyLeft:
//		if(gettreecolumn() == current_column) {
//			if(isopen(current)) {
//				toggle(current);
//				break;
//			}
//			auto parent = getparent(current);
//			if(parent != current) {
//				select(parent, current_column);
//				break;
//			}
//		}
//		return table::keyinput(id);
//	case KeyRight:
//		if(gettreecolumn() != current_column || isopen(current))
//			return table::keyinput(id);
//		toggle(current);
//		break;
//	default: return table::keyinput(id);
//	}
//	return true;
//}

//void tableref::shift(int i1, int i2) {
//	auto c1 = getblockcount(i1);
//	auto c2 = getblockcount(i2);
//	array::shift(i1, i2, c1, c2);
//	if(i1 < i2)
//		select(i1, current_column);
//	else
//		select(i2 + c1, current_column);
//}