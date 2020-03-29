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

void tree::toggle(int index) {
	if(!isgroup(index))
		return;
	auto mm = getmaximum();
	auto cc = current;
	if(isopen(index))
		collapse(index);
	else
		expand(index);
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
			auto c = getmaximum();
			for(auto i = 0; i < c; i++) {
				if(level != getlevel(i))
					continue;
				if(i < c - 1) {
					if(getlevel(i + 1) > level)
						continue;
				}
				if(isgroup(i)) {
					auto i1 = getmaximum();
					expand(i);
					if(i1 < getmaximum()) {
						need_test = true;
						break;
					}
				}
			}
		}
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