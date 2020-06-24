#include "main.h"

static unsigned initialize_counter = 0;
static datetime initialize_date;

database databases[256];

static void add(base_s id, rfid parent, const char* name, const char* comment) {
	auto p = (reference*)databases[id].add();
	p->base = id;
	p->session_id = 0;
	p->create_date = initialize_date;
	p->counter = initialize_counter++;
	p->parent = parent;
	p->name = szdup(name);
	p->comment = szdup(comment);
}

//static void add_name(base_s type) {
//	auto& e = databases[type];
//	add(e, "Наименование", TextType);
//	add(e, "Комментарий", TextType);
//}
//static void add_stamp(base_s type) {
//	auto& e = databases[type];
//	add(e, "create_date", "Создано", DateTimeType);
//	add(e, "identifier", "Идентификатор", NumberType);
//}
//static void add_change(base_s type) {
//}
//static void add_reference(base_s type) {
//	add_stamp(type);
//	add_change(type);
//	add_name(type);
//}
//static void add_number(base_s type, const char* id, const char* name) {
//	auto& e = databases[type];
//	add(e, id, name, NumberType);
//}
//static void add_text(base_s type, const char* id, const char* name) {
//	auto& e = databases[type];
//	add(e, id, name, TextType);
//}
//static void add_fio(base_s type) {
//	add_reference(type);
//	add_text(type, "firstname", "Имя");
//	add_text(type, "surname", "Фамилия");
//	add_text(type, "lastname", "Отчество");
//}

static void setdate(datetime v) {
	initialize_date = v;
	initialize_counter = 0;
}

static void create_base(base_s id, unsigned size) {
	databases[id].setup(size);
}

static void add_requisits() {
	create_base(HeaderType, sizeof(header));
	create_base(UserType, sizeof(user));
	create_base(RequisitType, sizeof(requisit));
	setdate({2020, 6, 24, 13, 15});
	add(RequisitType, 0, "Идентификатор", "Уникальный идентификатор в пределах даты создания");
	add(RequisitType, 0, "Наименование", "Наименование используемое для представления в списках");
	add(RequisitType, 0, "Комментарий", "Комментарий или описание");
	add(RequisitType, 0, "Родитель", "Нахождение в иерархии");
	add(RequisitType, 0, "Тип", "Тип данных или база данных");
	add(RequisitType, 0, "Дата изменения", "Дата и время изменения объекта");
	add(RequisitType, 0, "Дата создания", "Дата и время создания объекта");
	add(RequisitType, 0, "Автор", "Пользователь, который создал объект");
	add(RequisitType, 0, "Изменил", "Пользователь, который последним менял объект");
	add(RequisitType, 0, "Ответственный", "Пользователь, который отвечает за объект");
}

void initialize_metadata() {
	add_requisits();
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