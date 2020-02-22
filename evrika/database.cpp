#include "main.h"

metadata databases[] = {{"Number", "Число"},
{"Date", "Дата"},
{"DateTime", "Дата и время"},
{"Text", "Строка"},
{"Reference", "Справочник"},
{"Document", "Документ"},
{"User", "Пользователь"},
{"Header", "Раздел"},
};

static void add_name(base_s type) {
	auto& e = databases[type];
	e.add("name", "Наименование", TextType);
	e.add("text", "Комментарий", TextType);
}
static void add_stamp(base_s type) {
	auto& e = databases[type];
	e.add("create_date", "Создано", DateTimeType);
	e.add("identifier", "Идентификатор", NumberType);
}
static void add_change(base_s type) {
	auto& e = databases[type];
	e.add("create_user", "Создал", UserType);
	e.add("change_user", "Изменил", UserType);
	e.add("change_date", "Изменено", DateTimeType);
}
static void add_reference(base_s type) {
	add_stamp(type);
	add_change(type);
	add_name(type);
}
static void add_number(base_s type, const char* id, const char* name) {
	auto& e = databases[type];
	e.add(id, name, NumberType);
}
static void add_text(base_s type, const char* id, const char* name) {
	auto& e = databases[type];
	e.add(id, name, TextType);
}
static void add_document(base_s type) {
	add_stamp(type);
	add_change(type);
	add_number(type, "number", "Номер");
}
static void add_fio(base_s type) {
	add_reference(type);
	add_text(type, "firstname", "Имя");
	add_text(type, "surname", "Фамилия");
	add_text(type, "lastname", "Отчество");
}

void initialize_metadata() {
	add_reference(ReferenceType);
	//
	add_document(DocumentType);
	//
	add_reference(UserType);
	add_fio(UserType);
}

requisit* metadata::add(const char* id, const char* name, base_s type) {
	if(count)
		return 0;
	auto p = (requisit*)requisits.add();
	memset(p, 0, sizeof(requisit));
	p->id = szdup(id);
	p->name = szdup(name);
	p->type = type;
	p->offset = size;
	size += p->getsize();
	return p;
}

void metadata::add(const requisit* source) {
	for(auto p = source; *p; p++)
		add(p->id, p->name, p->type);
}

metadata* metadata::find(const void* object) {
	return 0;
}

void* datalist::add() {
	if(count >= maximum) {
		if(!size)
			return 0;
		maximum = rmoptimal(maximum + 1);
		auto new_size = maximum * size;
		elements = rmreserve(elements, new_size);
	}
	return (char*)elements + (count++)*size;
}

metadata::~metadata() {
	if(elements)
		delete elements;
}

void* metadata::find(unsigned offset, const void* object, unsigned object_size) const {
	auto pe = end();
	for(auto pp = begin() + offset; pp < pe; pp += size) {
		if(memcmp(pp, object, object_size) == 0)
			return pp - offset;
	}
	return 0;
}

void metadata::select(valuelist& result, bool standart, bool object) {
}

static const char* findurl(const char* p) {
	while(*p && p[0] != '.')
		p++;
	return p;
}

const requisit* requisit::find(const char* id, unsigned size) const {
	for(auto p = this; *p; p++) {
		if(memcmp(p->id, id, size) == 0)
			return p;
	}
	return 0;
}

const requisit* requisita::find(const char* id, unsigned size) const {
	for(auto& e : *this) {
		if(memcmp(e.id, id, size) == 0)
			return &e;
	}
	return 0;
}

void* requisita::ptr(void* object, const char* url, const requisit** result) const {
	while(object) {
		auto p1 = findurl(url);
		auto pf = find(url, p1 - url);
		if(!pf)
			return 0;
		object = pf->ptr(object);
		if(p1[0] == 0) {
			if(result)
				*result = pf;
			return object;
		}
		if(pf->isreference())
			object = *((void**)object);
		else
			break;
	}
	return 0;
}

void requisita::set(void* object, const char* id, int v) const {
	const requisit* type;
	object = ptr(object, id, &type);
	if(!object)
		return;
	type->set(object, v);
}

void requisita::set(void* object, const char* id, const char* v) const {
	const requisit* type;
	object = ptr(object, id, &type);
	if(!object)
		return;
	type->set(object, (int)szdup(v));
}

const char* requisita::gets(const void* p, const char* url) const {
	const requisit* type;
	p = ptr((void*)p, url, &type);
	if(!p)
		return "";
	auto pv = *((const char**)p);
	if(!pv)
		pv = "";
	return pv;
}

void* datalist::find(const void* object, unsigned size) const {
	return 0;
}

void* datastore::ptr(unsigned index) const {
	for(auto p = first; p; p = p->next) {
		if(index > p->count)
			index -= p->count;
		else
			return (unsigned char*)p + size*index;
	}
	return 0;
}

void* datastore::add() {
	if(!first)
		first = (element*)new char[sizeof(element) + size * 32];
	auto p = first;
	while(true) {
		if(p->count < p->count_maximum)
			return p->ptr(size, p->count++);
		if(!p->next)
			break;
		p = p->next;
	}
	p->next = (element*)new char[sizeof(element) + size * (p->count_maximum * 2)];
	return p->ptr(size, p->count++);
}