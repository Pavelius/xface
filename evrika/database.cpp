#include "main.h"

//metadata databases[] = {{"Number", "Число"},
//{"Date", "Дата"},
//{"DateTime", "Дата и время"},
//{"Text", "Строка"},
//{"Reference", "Справочник"},
//{"Document", "Документ"},
//{"User", "Пользователь"},
//{"Header", "Раздел"},
//};

static void add(database& e, const char* id, const char* name, base_s base) {

}

static void add_name(base_s type) {
	auto& e = databases[type];
	add(e, "name", "Наименование", TextType);
	add(e, "text", "Комментарий", TextType);
}
static void add_stamp(base_s type) {
	auto& e = databases[type];
	add(e, "create_date", "Создано", DateTimeType);
	add(e, "identifier", "Идентификатор", NumberType);
}
static void add_change(base_s type) {
}
static void add_reference(base_s type) {
	add_stamp(type);
	add_change(type);
	add_name(type);
}
static void add_number(base_s type, const char* id, const char* name) {
	auto& e = databases[type];
	add(e, id, name, NumberType);
}
static void add_text(base_s type, const char* id, const char* name) {
	auto& e = databases[type];
	add(e, id, name, TextType);
}
static void add_fio(base_s type) {
	add_reference(type);
	add_text(type, "firstname", "Имя");
	add_text(type, "surname", "Фамилия");
	add_text(type, "lastname", "Отчество");
}

void initialize_metadata() {
	add_reference(ReferenceType);
	add_reference(UserType);
	add_fio(UserType);
}

void stamp::generate(base_s id) {
	base = id;
	counter = databases[id].getcount();
}

void stamp::generate() {

}

//static const char* findurl(const char* p) {
//	while(*p && p[0] != '.')
//		p++;
//	return p;
//}
//
//const requisit* requisit::find(const char* id, unsigned size) const {
//	for(auto p = this; *p; p++) {
//		if(memcmp(p->id, id, size) == 0)
//			return p;
//	}
//	return 0;
//}
//
//const requisit* requisita::find(const char* id, unsigned size) const {
//	for(auto& e : *this) {
//		if(memcmp(e.id, id, size) == 0)
//			return &e;
//	}
//	return 0;
//}
//
//void* requisita::ptr(void* object, const char* url, const requisit** result) const {
//	while(object) {
//		auto p1 = findurl(url);
//		auto pf = find(url, p1 - url);
//		if(!pf)
//			return 0;
//		object = pf->ptr(object);
//		if(p1[0] == 0) {
//			if(result)
//				*result = pf;
//			return object;
//		}
//		if(pf->isreference())
//			object = *((void**)object);
//		else
//			break;
//	}
//	return 0;
//}