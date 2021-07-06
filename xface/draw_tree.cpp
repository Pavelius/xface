#include "draw_control.h"

using namespace draw;
using namespace draw::controls;

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

void tree::update(void* p) {
	auto i = find(0, -1, p);
	if(i == -1)
		return;
	expand(i);
}

void tree::expand(int index) {
	auto before_count = getmaximum();
	if(before_count == 0)
		index = -1;
	else {
		// Пометим все элементы
		auto level = getlevel(index) + 1;
		for(auto i = index + 1; i < before_count; i++) {
			auto p = (element*)ptr(i);
			if(p->level > level)
				continue;
			if(p->level < level)
				break;
			p->set(element::Marked);
		}
	}
	expanding(index);
	auto after_count = getmaximum();
	if(before_count) {
		// Уберем признак группы, если не добавились элементы
		auto p = (element*)array::ptr(index);
		if(before_count != after_count)
			p->set(element::Group);
		else
			p->remove(element::Group);
	}
	if(before_count) {
		// Удалим все элементы, которые были и не обновились
		auto level = getlevel(index) + 1;
		for(auto i = index + 1; i < after_count; i++) {
			auto p = (element*)ptr(i);
			if(p->level > level)
				continue;
			if(p->level < level)
				break;
			if(p->is(element::Marked)) {
				collapse(i);
				after_count = getmaximum();
				i--;
			}
		}
	}
}

tree::element* tree::addnode(int parent, unsigned char type, unsigned char image, void* object, bool isgroup) {
	auto m = getmaximum();
	auto level = (parent == -1) ? 1 : getlevel(parent) + 1;
	auto ni = parent + 1;
	for(; ni < m; ni++) {
		auto p = (element*)ptr(ni);
		if(p->level > level)
			continue;
		if(p->level < level)
			break;
		if(p->object == object) {
			p->type = type;
			p->image = image;
			p->remove(element::Marked);
			return p;
		}
	}
	auto p = (element*)array::insert(ni, 0);
	p->level = level;
	p->object = object;
	p->type = type;
	p->image = image;
	if(isgroup)
		p->flags = 1 << element::Group;
	else
		p->flags = 0;
	return p;
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

void tree::view(const rect& rc) {
	if(auto_expand && getmaximum() == 0) {
		auto_expand = false;
		expand(-1);
	}
	table::view(rc);
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