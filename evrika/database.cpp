#include "main.h"

static metadata number_type[] = {{"Number", "�����"}};
static metadata date_type[] = {{"Date", "����"}};
static metadata datetime_type[] = {{"DateTime", "���� � �����"}};
static metadata text_type[] = {{"Text", "������"}};
static metadata state_type[] = {{"State", "���������"}};

static metadata object_type[] = {{"Object", "������", 0}};
static metadata reference_type[] = {{"Reference", "����������", object_type}};
static metadata document_type[] = {{"Document", "��������", object_type}};
static metadata users[] = {{"User", "������������", reference_type}};
static metadata headers[] = {{"Header", "������", reference_type}};

static metadata* object_types[] = {headers, reference_type, users, document_type, users};
static metadata* simple_types[] = {number_type, date_type, datetime_type, text_type, state_type};

static void add_stamp(metadata& e) {
	e.add("create_user", "�����", users);
	e.add("create_date", "���� ��������", datetime_type);
	e.add("change_user", "�������", users);
	e.add("change_date", "���� ���������", datetime_type);
}

void metadata::initialize() {
	object_type->add("id", "�������������", text_type);
	object_type->add("name", "������������", text_type);
	object_type->add("text", "�����������", text_type);
	object_type->add("parent", "��������", object_type);
	object_type->add("state", "���������", state_type);
	add_stamp(object_type[0]);
	users->add("firstname", "���", text_type);
	users->add("surname", "�������", text_type);
	users->add("lastname", "��������", text_type);
	document_type->add("date", "����", datetime_type);
	add_stamp(document_type[0]);
}

requisit* metadata::add(const char* id, const char* name, const metadata* type) {
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
	if(!object)
		return 0;
	for(auto p : object_types) {
		auto pb = p->elements;
		if(!pb)
			continue;
		auto pe = (char*)pb + p->size*p->count;
		if(object < pb || object > pe)
			continue;
		return p;
	}
	return 0;
}

void* datalist::add() {
	if(!elements && size && !count)
		elements = new char[maximum*size];
	if(count < maximum)
		return (char*)elements + (count++)*size;
	return 0;
}

metadata::~metadata() {
	if(maximum && elements)
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

void select(valuelist& result, bool standart, bool object) {
	if(standart) {
		for(auto p : simple_types)
			result.add(p->name, (int)p, 0, 4);
	}
	if(object) {
		for(auto p : object_types)
			result.add(p->name, (int)p, 0, 4);
	}
}