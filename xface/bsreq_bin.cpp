#include "crt.h"
#include "bsreq.h"
#include "io.h"

namespace {
struct parser {
	io::stream&		file;
	bool			writemode;
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
						if(!serial_reference(pv, p->type, p->source))
							return false;
					} else {
						if(!serial_reference(pv, p->type, p->source))
							return false;
						p->set(p->ptr(object, i), (int)pv);
					}
				}
				break;
			case KindScalar:
				for(unsigned i = 0; i < p->count; i++)
					serial(p->ptr(object, i), p->type); // Подчиненный объект, указанный прямо в теле
				break;
            default:
                break;
			}
		}
		return true;
	}
	bool serial_reference(void*& pv, const bsreq* pk, array* pb) {
		char temp[256];
		if(!pk)
			return false;
		// Теперь определим ключ ссылки
		// Ключем может быть только ОДНО первое поле
		// Оно может быть текстовое или скалярного типа (не ссылочный объект)
		if(pk->is(KindReference))
			return false;
		if(pk->is(KindText) && (pk->type->count > 1))
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
			int index = -1;
			pv = 0;
			if(pk->is(KindText)) {
				*((const char**)temp) = read_string();
				if(pb)
					index = pb->find(*((const char**)temp), 0);
			} else {
				file.read(temp, pk->lenght);
				if(pb)
					index = pb->find(temp, 0, pk->lenght);
			}
			// Если ключ не найден попытаемся его создать
			if(pb) {
				if(index == -1) {
					if(pb->isgrowable())
						pv = pb->add();
				} else
					pv = pb->ptr(index);
			}
			if(pv)
				memcpy(pk->ptr(pv), temp, pk->lenght);
		}
		return true;
	}
	bool read_object(array* pb, const bsreq* meta) {
		void* pv = 0;
		if(!serial_reference(pv, meta, pb))
			return false;
		return serial(pv, meta + 1);
	}
	bool write_object(const void* pv, const bsreq* meta) {
		auto cpv = (void*)pv;
		if(!serial_reference(cpv, meta, 0))
			return false;
		return serial(pv, meta + 1);
	}
	constexpr parser(io::stream& file, bool writemode) : file(file), writemode(writemode) {}
};
}

bool bsreq::write(const char* url, void* object) const {
	io::file file(url, StreamWrite);
	if(!file)
		return false;
	parser e(file, true);
	return e.write_object(object, this);
}

bool bsreq::read(const char* url, const void* object) const {
	io::file file(url, StreamRead);
	if(!file)
		return false;
	parser e(file, false);
	return e.serial(object, this);
}