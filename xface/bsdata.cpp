#include "crt.h"
#include "bsdata.h"

bsdata* bsdata::first;

void bsdata::globalize() {
	if(!find(fields))
		seqlink(this);
}

void* bsdata::add() {
	if(getcount() >= getmaxcount())
		return get(0);
	return get(count++);
}

bsdata* bsdata::find(const char* id) {
	if(!id || !id[0])
		return 0;
	for(auto p = first; p; p = p->next) {
		if(strcmp(p->id, id) == 0)
			return p;
	}
	return 0;
}

bsdata* bsdata::find(const bsreq* type) {
	if(!type)
		return 0;
	for(auto p = first; p; p = p->next) {
		if(p->fields == type)
			return p;
	}
	return 0;
}

int bsdata::indexof(const void* object) const {
	if(object >= data && object <= (char*)data + maximum_count*size)
		return ((char*)object - (char*)data) / size;
	return -1;
}

void bsdata::remove(int index, int elements_count) {
	if(((unsigned)index) >= getcount())
		return;
	if((unsigned)index < getcount() - 1)
		memcpy((char*)data + index*size,
		(char*)data + (index + elements_count)*size,
			(getcount() - (index + elements_count))*size);
	count -= elements_count;
}

bsdata* bsdata::findbyptr(const void* object) {
	if(!object)
		return 0;
	for(auto p = first; p; p = p->next)
		if(p->indexof(object) != -1)
			return p;
	return 0;
}

void* bsdata::find(const bsreq* id, const char* value) {
	if(!id || id->type != text_type)
		return 0;
	auto ps = (char*)id->ptr(data);
	auto pe = ps + size*getcount();
	for(; ps < pe; ps += size) {
		auto ps_value = (const char*)id->get(ps);
		if(!ps_value)
			continue;
		if(strcmp(ps_value, value) == 0)
			return get(indexof(ps));
	}
	return 0;
}

const char* bsval::getname() const {
	auto d = data;
	if(type->isenum) {
		auto b = bsdata::find(type->type);
		if(!b)
			return "No base";
		auto i = type->get(type->ptr(d));
		d = b->get(i);
	}
	auto t = type->type;
	auto f = t->find("name", text_type);
	if(!f)
		return "";
	auto p = (const char*)f->get(f->ptr(d));
	if(!p)
		return "";
	return p;
}