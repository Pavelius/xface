#include "xface/io.h"
#include "main.h"

struct database_serial_bin {
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
					if(writemode) {
						auto pv = (reference*)p->get(p->ptr(object, i));
						write_reference(pv);
					} else {
						auto pv = read_reference();
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
	void write_reference(objecti* p) {
	}
	void* read_reference() {
		return 0;
	}
	//bool read_object() {
	//	auto pv = read_reference();
	//	if(!pv)
	//		return false;
	//	auto pm = pv->getmeta();
	//	return serial(pv, pv->getmeta() + 1);
	//}
	//bool write_object(bsdata* pb, const void* pv) {
	//	auto cpv = (void*)pv;
	//	if(!serial_reference(cpv, 0))
	//		return false;
	//	return serial(pv, pb->meta + 1);
	//}
	constexpr database_serial_bin(io::stream& file, bool writemode) : file(file), writemode(writemode) {}
};

bool metadata::readfile(const char* url) {
	io::file file(url, StreamRead);
	if(!file)
		return 0;
	return 0;
}

bool metadata::writefile(const char* url) {
	io::file file(url, StreamWrite);
	if(!file)
		return 0;
	return 0;
}