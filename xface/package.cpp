#include "package.h"

using namespace code;

unsigned package::find(const char* v, unsigned len) {
	if(v && len) {
		auto pe = strings.end();
		auto s = *v;
		// Signature have first 4 bytes
		for(auto p = strings.begin() + 4; p < pe; p++) {
			if(*p != s)
				continue;
			unsigned n1 = pe - p - 1;
			if(n1 < len)
				return -1;
			if(memcmp(p + 1, v + 1, len) == 0)
				return p - strings.begin();
		}
	}
	return -1;
}

unsigned package::add(const char* v, unsigned len) {
	strings.reserve(len + 1);
	auto result = strings.count;
	memcpy(strings.ptr(strings.count), v, len + 1);
	strings.setcount(strings.count + len + 1);
	return result;
}

unsigned package::add(const char* v) {
	if(!v || v[0] == 0)
		return 0;
	auto c = zlen(v);
	auto i = find(v, c);
	if(i == 0xFFFFFFFF)
		i = add(v, c);
	return i;
}

unsigned package::find(opr type, unsigned left, unsigned right) const {
	for(auto& e : asts) {
		if(e.type == type && e.left == left && e.right == right)
			return &e - asts.begin();
	}
	return -1;
}

unsigned package::add(opr type, unsigned left, unsigned right) {
	auto i = find(type, left, right);
	if(i != 0xFFFFFFFF)
		return i;
	auto p = asts.add();
	p->type = type;
	p->left = left;
	p->right = right;
	return p - asts.begin();
}

void package::clear() {
	strings.clear();
	add("PKG", 3);
}