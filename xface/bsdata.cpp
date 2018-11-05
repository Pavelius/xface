#include "crt.h"
#include "bsdata.h"

bsdata* bsdata::first;

void bsdata::globalize() {
	if(!find(fields))
		seqlink(this);
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
	auto ps = (char*)id->ptr(begin());
	auto pe = ps + getsize()*getcount();
	for(; ps < pe; ps += getsize()) {
		auto ps_value = (const char*)id->get(ps);
		if(!ps_value)
			continue;
		if(strcmp(ps_value, value) == 0)
			return get(indexof(ps));
	}
	return 0;
}

const char* bsval::getname() const {
	if(!data)
		return "";
	auto f = type->find("name", text_type);
	if(!f)
		return "";
	auto p = (const char*)f->get(f->ptr(data));
	if(!p)
		return "";
	return p;
}

const char* bsval::findpart(const char* id) {
	for(auto f = type; *f; f++) {
		for(auto i = 0; true; i++) {
			if(id[i]==0 || id[i] == '.') {
				if(f->id[i] == 0) {
					type = f;
					if(id[i] == '.')
						return id + i + 1;
					return 0;
				}
			} else if(id[i] != f->id[i])
				break;
		}
	}
	data = 0;
	return 0;
}

bsval& bsval::get(const char* id) {
	while((id=findpart(id))!=0) {
		if(!dereference())
			data = 0;
	}
	return *this;
}

bsval& bsval::dereference() {
	if(data) {
		if(type->isenum) {
			auto b = bsdata::find(type->type);
			if(!b)
				data = 0;
			else {
				data = b->get(get());
				type = b->fields;
			}
		}
	}
	return *this;
}