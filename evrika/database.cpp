#include "main.h"

static unsigned initialize_counter = 0;
static datetime initialize_date;

database databases[256];

static rfid add(const char* name) {
	return 0;
}

static reference* addrf(base_s id, rfid parent, const char* name, const char* comment) {
	auto p = (reference*)databases[id].add();
	p->base = id;
	p->session_id = 0;
	p->create_date = initialize_date;
	p->counter = initialize_counter++;
	p->parent = parent;
	p->name = add(name);
	p->comment = add(comment);
	return p;
}

static rfid add(base_s id, rfid parent, const char* name, const char* comment) {
	auto p = addrf(id, parent, name, comment);
	return gtr(id, databases[id].indexof(p));
}

static rfid addhd(base_s id, rfid parent, const char* name, const char* comment) {
	auto p = (header*)addrf(HeaderType, parent, name, comment);
	p->base = id;
	return gtr(id, databases[id].indexof(p));
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
	setdate({2020, 6, 25, 15, 45});
	auto parent = addhd(RequisitType, 0, "Реквизиты", "Список всех возможных реквизитов в системе");
	add(RequisitType, parent, "Идентификатор", "Уникальный идентификатор в пределах даты создания");
	add(RequisitType, parent, "Наименование", "Наименование используемое для представления в списках");
	add(RequisitType, parent, "Комментарий", "Комментарий или описание");
	add(RequisitType, parent, "Родитель", "Нахождение в иерархии");
	add(RequisitType, parent, "Тип", "Тип данных или база данных");
	add(RequisitType, parent, "Дата изменения", "Дата и время изменения объекта");
	add(RequisitType, parent, "Дата создания", "Дата и время создания объекта");
	add(RequisitType, parent, "Автор", "Пользователь, который создал объект");
	add(RequisitType, parent, "Изменил", "Пользователь, который последним менял объект");
	add(RequisitType, parent, "Ответственный", "Пользователь, который отвечает за объект");
}

static void add_documents() {
	setdate({2020, 6, 24, 13, 15});
	auto parent = addhd(ReferenceType, 0, "Документы", "События, которые влияют на деятельность предприятия");
}

void initialize_metadata() {
	create_base(RequisitType, sizeof(requisit));
	create_base(HeaderType, sizeof(header));
	create_base(UserType, sizeof(user));
	add_requisits();
	add_documents();
}

void stamp::generate() {

}

static void change_add(void* p1, void* p2, unsigned s1, unsigned s2, unsigned count, unsigned s3) {
	for(unsigned i = 0; i < count; i++) {
		memmove(p1, p2, s3);
		p1 = (char*)p1 - s1;
		p2 = (char*)p2 - s2;
	}
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