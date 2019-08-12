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
	e.add("create_date", "Дата создания", DateTimeType);
	e.add("counter", "Токен", NumberType);
}
static void add_change(base_s type) {
	auto& e = databases[type];
	e.add("create_user", "Автор", UserType);
	e.add("change_user", "Изменил", UserType);
	e.add("change_date", "Дата изменения", DateTimeType);
}
static void add_reference(base_s type) {
	add_stamp(type);
	add_name(type);
	add_change(type);
}
static void add_document(base_s type) {
	add_stamp(type);
	add_change(type);
	auto& e = databases[type];
	e.add("number", "Номер", NumberType);
}
static void add_fio(base_s type) {
	add_reference(type);
	auto& e = databases[type];
	e.add("firstname", "Имя", TextType);
	e.add("surname", "Фамилия", TextType);
	e.add("lastname", "Отчество", TextType);
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
		maximum = rmoptimal(maximum+1);
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

void* requisit::ptr(void* object, const char* url, const requisit** result) const {
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
		if(!pf->isreference())
			return 0;
		object = *((void**)object);
	}
	return 0;
}