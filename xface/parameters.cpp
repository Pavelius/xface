#include "crt.h"
#include "parameters.h"

parameters::element* parameters::add(const char* id) {
	auto p = get(id);
	if(p)
		return const_cast<element*>(p);
	if(count < sizeof(elements) / sizeof(elements[0]))
		return elements + (count++);
	return 0;
}

const parameters::element* parameters::get(const char* id) const {
	for(auto& e : *this) {
		if(strcmp(e.id, id) == 0)
			return &e;
	}
	return 0;
}

void parameters::add(const char* id, long value) {
	auto p = add(id);
	if(!p)
		return;
	p->type = kind::Number;
	p->value = value;
}

void parameters::add(const char* id, const char* value) {
	auto p = add(id);
	if(!p)
		return;
	p->type = kind::Text;
	p->value = (int)addstr(value);
}

const char* parameters::addstr(const char* id) {
	if(!id || id[0] == 0)
		return "";
	auto c = id[0];
	auto textc = zlen(id) + 1;
	auto i = 0;
	while(i < buffer_count) {
		if(c == buffer[i]) {
			auto j = 1;
			auto p = &buffer[i];
			for(; j < textc; j++)
				if(p[j] != id[j])
					break;
			if(j == textc)
				return p;
		}
		i++;
	}
	int m = sizeof(buffer) - i;
	if(m > textc)
		return "";
	auto p = buffer + i;
	memcpy(p, id, textc);
	count += textc;
	return p;
}

int	parameters::getnum(const char* id) const {
	auto p = get(id);
	if(!p)
		return 0;
	return p->value;
}
const char*	parameters::getstr(const char* id) const {
	auto p = get(id);
	if(!p)
		return "";
	return (const char*)p->value;
}