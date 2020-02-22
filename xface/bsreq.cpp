#include "bsreq.h"
#include "stringbuilder.h"

extern "C" int strcmp(const char* s1, const char* s2);
extern "C" int memcmp(const void* s1, const void* s2, unsigned size);

const bsreq bsmeta<int>::meta[] = {{"number"}, {}};
const bsreq bsmeta<const char*>::meta[] = {{"text"}, {}};
const bsreq bsmeta<bsreq>::meta[] = {
	BSREQ(id),
	BSREQ(offset),
	BSREQ(size),
	BSREQ(lenght),
	BSREQ(count),
	BSREQ(type),
	{}};

const bsreq* bsreq::find(const char* name) const {
	if(!this || !name || name[0] == 0)
		return 0;
	for(auto p = this; p->id; p++) {
		if(strcmp(p->id, name) == 0)
			return p;
	}
	return 0;
}

const bsreq* bsreq::find(const char* name, unsigned count) const {
	if(!this || !name || name[0] == 0)
		return 0;
	for(auto p = this; p->id; p++) {
		if(memcmp(p->id, name, count) == 0)
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
	if(!value || type != bsmeta<const char*>::meta)
		return false;
	for(int i = 0; name[i]; i++) {
		if(value[i] != name[i])
			return false;
	}
	return true;
}

const bsreq* bsreq::getname() const {
	auto p = find("name", bsmeta<const char*>::meta);
	if(!p)
		p = find("id", bsmeta<const char*>::meta);
	if(!p)
		p = find("text", bsmeta<const char*>::meta);
	return p;
}

const char* bsreq::gets(const void* p) const {
	if(is(KindText)) {
		auto v = (const char*)get(p);
		if(!v)
			return "";
		return v;
	} else if(is(KindReference)) {
		auto pf = type->getname();
		if(!pf)
			return "";
		auto v = (void*)get(p);
		if(!v)
			return "";
		return pf->gets(pf->ptr(v));
	} else if(is(KindEnum)) {
		auto pb = source;
		if(!pb)
			return "";
		auto pf = type->getname();
		if(!pf)
			return "";
		auto vi = get(p);
		auto v = (void*)pb->ptr(vi);
		return pf->gets(pf->ptr(v));
	}
	return "";
}

const char* bsreq::get(const void* p, char* result, const char* result_max) const {
	if(is(KindNumber)) {
		auto v = get(p);
		stringbuilder sb(result, result_max);
		sb.add("%1i", v);
		return result;
	}
	return gets(p);
}

bsval bsval::ptr(const char* url) const {
	bsval r(data, type);
	for(auto i = 0; url[i]; i++) {
		if(url[i] == '.') {
			r.type = type->find(url, i);
			r = r.dereference();
		}
	}
	r.type = r.type->find(url);
	if(!r.type)
		r.data = 0;
	return r;
}

bsval bsval::dereference() const {
	bsval r(data, type);
	while(true) {
		if(r.type->is(KindReference)) {
			r.data = *((void**)r.type->ptr(r.data));
			r.type = r.type->type;
		} else if(r.type->is(KindEnum)) {
			if(!r.type->source) {
				r.data = 0;
				r.type = 0;
				break;
			}
			r.data = r.type->source->ptr(r.get());
			r.type = r.type->type;
		} else
			break;
	}
	return r;
}

const char*	bsval::getname() const {
	auto pf = type->find("name");
	if(!pf)
		return "";
	auto pv = (const char*)pf->get(pf->ptr(data));
	if(!pv)
		return "";
	return pv;
}