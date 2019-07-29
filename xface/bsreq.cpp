#include "crt.h"
#include "bsreq.h"

extern "C" int strcmp(const char* s1, const char* s2);
extern "C" int memcmp(const void* s1, const void* s2, unsigned size);

bsdata*	bsdata::first;
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
	if(!this || !name || name[0]==0)
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
	if(!value || type != bsmeta<const char*>::meta)
		return false;
	for(int i = 0; name[i]; i++) {
		if(value[i] != name[i])
			return false;
	}
	return true;
}

bsdata::bsdata(const char* id, const bsreq* meta,
	void* data, unsigned size, unsigned count, unsigned maximum,
	bstype_s subtype) :
	id(id), meta(meta), next(0),
	data(data), count(count), maximum(maximum), size(size),
	subtype(subtype) {
	auto pf = &first;
	while(*pf)
		pf = &((*pf)->next);
	*pf = this;
}

void* bsdata::add() {
	if(count < maximum)
		return (char*)data + (count++)*size;
	return data;
}

bsdata* bsdata::find(const char* v) {
	if(!v || !v[0])
		return 0;
	for(auto p = first; p; p = p->next) {
		if(strcmp(p->id, v) == 0)
			return p;
	}
	return 0;
}

bsdata* bsdata::find(const bsreq* v) {
	if(!v)
		return 0;
	for(auto p = first; p; p = p->next) {
		if(p->meta == v)
			return p;
	}
	return 0;
}

bsdata* bsdata::findbyptr(const void* object) {
	if(!object)
		return 0;
	for(auto p = first; p; p = p->next)
		if(p->has(object))
			return p;
	return 0;
}

int	bsdata::indexof(const void* object) const {
	if(!has(object))
		return -1;
	return ((char*)object - (char*)data) / size;
}

const void* bsdata::find(const bsreq* id, const char* value) const {
	if(!id || id->type != bsmeta<const char*>::meta)
		return 0;
	if(!value)
		return find(id, &value, sizeof(value));
	auto ps = (char*)id->ptr(data);
	auto pe = ps + size*count;
	for(; ps < pe; ps += size) {
		auto ps_value = (const char*)id->get(ps);
		if(!ps_value)
			continue;
		if(strcmp(ps_value, value) == 0) {
			auto i = indexof(ps);
			if(i == -1)
				return 0;
			return get(i);
		}
	}
	return 0;
}

const void* bsdata::find(const bsreq* id, const void* value, unsigned size) const {
	if(!id)
		return 0;
	auto ps = (char*)id->ptr(data);
	auto pe = ps + size * count;
	for(; ps < pe; ps += size) {
		if(memcmp(ps, value, size) == 0) {
			auto i = indexof(ps);
			if(i == -1)
				return 0;
			return get(i);
		}
	}
	return 0;
}

const char*	bsdata::getstring(const void* object, const bsreq* type, const char* id) {
	auto pf = type->find(id);
	if(!pf)
		return "";
	auto ps = (const char*)pf->get(pf->ptr(object));
	if(!ps)
		ps = "";
	return ps;
}

const bsreq* bsreq::getname() const {
	auto p = find("name", bsmeta<const char*>::meta);
	if(!p)
		p = find("id", bsmeta<const char*>::meta);
	if(!p)
		p = find("text", bsmeta<const char*>::meta);
	return p;
}

const char* bsreq::get(const void* p, char* result, const char* result_max) const {
	if(is(KindNumber)) {
		auto v = get(p);
		szprint(result, result_max, "%1i", v);
	} else if(is(KindText)) {
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
		return pf->get(pf->ptr(v), result, result_max);
	} else if(is(KindEnum)) {
		auto pb = bsdata::find(type);
		if(!pb)
			return "";
		auto pf = pb->meta->getname();
		if(!pf)
			return "";
		auto vi = get(p);
		auto v = (void*)pb->get(vi);
		return pf->get(pf->ptr(v), result, result_max);
	}
	return result;
}

bsval bsval::ptr(const char* url) const {
	bsval r(data, type->find(url));
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
			auto p = bsdata::find(r.type->type);
			r.data = (void*)p->get(r.get());
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