#include "draw_control.h"

using namespace draw;
using namespace draw::controls;

tree::growable::growable(tree* source, int index) : source(source), index(index), i1(index + 1), i2(index) {
	level = (index == -1) ? 0 : source->getlevel(index);
	auto m = source->getmaximum();
	for(auto i = i1; i < m; i++) {
		auto p = (tree::element*)source->array::ptr(i);
		if(p->level > level + 1)
			continue;
		if(p->level <= level)
			break;
		i2 = i;
		p->set(tree::element::Marked);
	}
}

tree::growable::~growable() {
	auto m = source->getmaximum();
	for(auto i = i1; i < m; i++) {
		auto p = (tree::element*)source->array::ptr(i);
		if(p->level > level + 1)
			continue;
		if(p->level <= level)
			break;
		if(p->is(element::Marked)) {
			source->collapse(i);
			source->remove(i);
			m = source->getmaximum();
		}
	}
}

void tree::growable::add(unsigned char type, unsigned char image, void* object) {
	element* p;
	auto i = source->find(i1, i2, object);
	if(i == -1) {
		p = (element*)source->array::insert(i1, 0);
		p->level = level + 1;
		p->flags = 0;
		p->set(element::Group);
	} else
		p = (element*)source->array::ptr(i);
	p->type = type;
	p->image = image;
	p->object = object;
	p->remove(tree::element::Marked);
}

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
	return ((element*)array::ptr(index))->is(element::Group);
}

tree::element* tree::insert(int& index, int level) {
	auto p = (element*)array::insert(index++, 0);
	p->level = level;
	return p;
}

int	tree::find(int i1, int i2, void* object) {
	return array::find(i1, i2, object, FO(element, object), sizeof(element::object));
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
	if(before_count == 0) {
		growable source(this, -1);
		expanding(source);
	} else {
		growable source(this, index);
		expanding(index, source);
	}
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

int tree::gettreecolumn() const {
	auto i = -1;
	for(auto& e : columns) {
		i++;
		if(e.size == SizeInner)
			continue;
		break;
	}
	return i;
}

bool tree::keyinput(unsigned id) {
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

//void tableref::shift(int i1, int i2) {
//	auto c1 = getblockcount(i1);
//	auto c2 = getblockcount(i2);
//	array::shift(i1, i2, c1, c2);
//	if(i1 < i2)
//		select(i1, current_column);
//	else
//		select(i2 + c1, current_column);
//}