#include "crt.h"
#include "bsdata.h"

bsdata* bsdata::first;

void bsdata::globalize(bool make_global) {
	if(!make_global)
		return;
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

//void bsdata::remove(int index, int elements_count) {
//	if(((unsigned)index) >= getcount())
//		return;
//	if((unsigned)index < getcount() - 1)
//		memcpy((char*)data + index*size,
//		(char*)data + (index + elements_count)*size,
//			(getcount() - (index + elements_count))*size);
//	count -= elements_count;
//}

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

const char* get_last(const char* id) {
	while(*id && *id != '.')
		id++;
	return id;
}

bsval bsval::ptr(const char* id) {
	auto result = *this;
	auto p = get_last(id);
	while(true) {
		result.type = result.type->find(id, p - id);
		if(!result.type)
			return result;
		if(p[0] == '.') {
			id = p + 1;
			if(result.type->isenum) {
				auto b = bsdata::find(result.type->type);
				if(!b)
					return bsval();
				result.data = b->get(result.get());
				result.type = b->fields;
			}
			continue;
		}
		break;
	}
	return bsval{type, data};
}