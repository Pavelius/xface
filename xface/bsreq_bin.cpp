#include "crt.h"
#include "bsreq.h"
#include "io.h"

struct bsdata_bin {
	io::stream&	file;
	bool		writemode;
	const char* read_string() {
		unsigned lenght = 0;
		file.read(&lenght, sizeof(lenght));
		if(!lenght)
			return 0;
		char buffer[2048];
		auto p = buffer;
		if(lenght >= (sizeof(buffer) - 1))
			p = new char[lenght + 1];
		file.read(p, lenght);
		p[lenght] = 0;
		auto result = szdup(p);
		if(p != buffer)
			delete p;
		return result;
	}
	void write_string(const char* v) {
		unsigned lenght = 0;
		if(v)
			lenght = zlen(v);
		file.write(&lenght, sizeof(lenght));
		if(lenght != 0)
			file.write(v, lenght);
	}
	void serial(void* v, unsigned lenght) {
		if(writemode)
			file.write(v, lenght);
		else
			file.read(v, lenght);
	}
	bool serial(const void* object, const bsreq* records) {
		for(auto p = records; *p; p++) {
			switch(p->subtype) {
			case KindNumber:
			case KindCFlags:
			case KindEnum:
				// Сериализация с оптимизацией
				serial(p->ptr(object), p->lenght);
				break;
			case KindText:
				for(unsigned i = 0; i < p->count; i++) {
					auto ps = (const char**)p->ptr(object, i);
					if(writemode)
						write_string(*ps);
					else
						*ps = read_string();
				}
				break;
			case KindReference:
				for(unsigned i = 0; i < p->count; i++) {
					void* pv = 0;
					if(writemode) {
						pv = (void*)p->get(p->ptr(object, i));
						if(!serial_reference(pv, 0))
							return false;
					} else {
						bsdata* pb = 0;
						if(!serial_reference(pv, &pb))
							return false;
						if(!pb)
							return false;
						p->set(p->ptr(object, i), (int)pv);
					}
				}
				break;
			case KindScalar:
				for(unsigned i = 0; i < p->count; i++)
					serial(p->ptr(object, i), p->type); // Подчиненный объект, указанный прямо в теле
				break;
			}
		}
		return true;
	}
	bool serial_reference(void*& pv, bsdata** ppb) {
		char temp[256];
		// Вначале определим базу данных
		bsdata* pb = 0;
		if(writemode) {
			pb = bsdata::findbyptr(pv);
			if(!pb) {
				file.write(&pb, sizeof(pb));
				return true;
			}
			write_string(pb->id);
		} else {
			pv = 0;
			auto key = read_string();
			if(!key) // Пустая ссылка
				return true;
			pb = bsdata::find(key);
		}
		if(!pb)
			return false;
		if(ppb)
			*ppb = pb;
		// Теперь определим ключ ссылки
		// Ключем может быть только ОДНО первое поле
		// Оно может быть текстовое или скалярного типа (не ссылочный объект)
		auto pk = pb->meta;
		if(pk->type->is(KindReference))
			return false;
		if(pk->type->is(KindText) && (pk->type->count > 1))
			return false;
		if(pk->lenght > sizeof(temp))
			return false;
		// Запишем ключевое поле
		if(writemode) {
			if(pk->is(KindText)) {
				auto v = (const char*)pk->get(pk->ptr(pv));
				write_string(v);
			} else
				file.write(pk->ptr(pv), pk->lenght);
		} else {
			if(pk->is(KindText)) {
				*((const char**)temp) = read_string();
				pv = (void*)pb->find(pk, *((const char**)temp));
			} else {
				file.read(temp, pk->lenght);
				pv = (void*)pb->find(pk, temp, pk->lenght);
			}
			// Если ключ не найден попытаемся его создать
			if(!pv && pb->count < pb->maximum) {
				pv = pb->add();
				memcpy(pk->ptr(pv), temp, pk->lenght);
			}
		}
		return pv != 0;
	}
	bool read_object() {
		bsdata* pb = 0;
		void* pv = 0;
		if(!serial_reference(pv, &pb))
			return false;
		if(!pb)
			return false;
		return serial(pv, pb->meta + 1);
	}
	bool write_object(bsdata* pb, const void* pv) {
		auto cpv = (void*)pv;
		if(!serial_reference(cpv, 0))
			return false;
		return serial(pv, pb->meta + 1);
	}
	constexpr bsdata_bin(io::stream& file, bool writemode) : file(file), writemode(writemode) {}
};

int bsdata::read(const char* url) {
	io::file file(url, StreamRead);
	if(!file)
		return 0;
	bsdata_bin e(file, false);
	auto result = 0;
	while(true) {
		if(!e.read_object())
			break;
		result++;
	}
	return result;
}

int bsdata::write(const char* url) {
	char temp[261]; io::file::getdir(temp, sizeof(temp) - 1);
	io::file file(url, StreamWrite);
	if(!file)
		return 0;
	bsdata_bin e(file, true);
	auto result = 0;
	for(auto pb = bsdata::first; pb; pb = pb->next) {
		for(unsigned i = 0; i < pb->count; i++) {
			if(!e.write_object(pb, pb->get(i)))
				return result;
			result++;
		}
	}
	// Запишем признак конца файла - нулевя строка имя объекта
	file.stream::write((int)0);
	return result;
}