#include "bsreq.h"

extern "C" int strcmp(const char* s1, const char* s2);

bsreq number_type[2] = {{"number"}};
bsreq text_type[2] = {{"text"}};
bsreq bsreq_type[] = {
	BSREQ(bsreq, id, text_type),
	BSREQ(bsreq, offset, number_type),
	BSREQ(bsreq, size, number_type),
	BSREQ(bsreq, lenght, number_type),
	BSREQ(bsreq, count, number_type),
	BSREQ(bsreq, reference, number_type),
	BSREQ(bsreq, type, bsreq_type),
{0}
};

const bsreq* bsreq::getkey() const {
	auto f = find("id", text_type);
	if(!f)
		f = find("name", text_type);
	if(!f)
		f = find("text", text_type);
	return f;
}

const bsreq* bsreq::find(const char* name) const {
	if(!this)
		return 0;
	for(auto p = this; p->id; p++) {
		if(strcmp(p->id, name) == 0)
			return p;
	}
	return 0;
}

const bsreq* bsreq::find(const char* name, const bsreq* type) const {
	if(!this)
		return 0;
	for(auto p = this; p->id; p++) {
		if(p->type != type)
			continue;
		if(strcmp(p->id, name) == 0)
			return p;
	}
	return 0;
}

const bsreq* bsreq::find(const char* name, unsigned count) const {
	if(!this)
		return 0;
	for(auto p = this; p->id; p++) {
		if(p->type != type)
			continue;
		auto found = true;
		for(unsigned i = 0; i < count; i++) {
			if(p->id[i] != name[i]) {
				found = false;
				break;
			}
		}
		if(found)
			return p;
	}
	return 0;
}

int bsreq::get(const void* p) const {
	switch(size) {
	case sizeof(char) : return *((char*)p);
	case sizeof(short) : return *((short*)p);
	default: return *((int*)p);
	}
}

void bsreq::set(const void* p, int value) const {
	switch(size) {
	case sizeof(char) : *((char*)p) = value; break;
	case sizeof(short) : *((short*)p) = value; break;
	default: *((int*)p) = value; break;
	}
}

bool bsreq::match(const void* p, const char* name) const {
	auto value = (const char*)get(p);
	if(!value || type != text_type)
		return false;
	for(int i = 0; name[i]; i++) {
		if(value[i] != name[i])
			return false;
	}
	return true;
}