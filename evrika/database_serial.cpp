#include "io.h"
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
	bool serial(void* object, const requisit* records, const requisit* pe) {
		for(auto p = records; p<pe; p++) {
			switch(p->type) {
			case NumberType:
			case DateTimeType:
			case DateType:
				// Сериализация с оптимизацией
				serial(p->ptr(object), p->getsize());
				break;
			case TextType:
				if(true) {
					auto ps = (const char**)p->ptr(object);
					if(writemode)
						write_string(*ps);
					else
						*ps = read_string();
				}
				break;
			default:
				if(writemode) {
					auto pv = (stamp*)p->get(p->ptr(object));
					write_reference(pv);
				} else {
					auto pv = read_reference();
					p->set(p->ptr(object), (int)pv);
				}
				break;
			}
		}
		return true;
	}
	void write_reference(stamp* p) {
		file.write(p, sizeof(*p));
	}
	stamp* read_reference() {
		stamp e = {}; file.read(&e, sizeof(e));
		if(!e.create_date)
			return 0;
		stamp* p = (stamp*)databases[e.type].find(0, &e, sizeof(e));
		if(p)
			return p;
		p = (stamp*)databases->add();
		memcpy(p, &e, sizeof(e));
		return p;
	}
	bool read_object() {
		auto pv = read_reference();
		if(!pv)
			return false;
		auto& bs = databases[pv->type];
		return serial(pv,
			((requisit*)bs.requisits.begin()) + 2,
			(requisit*)bs.requisits.end());
	}
	bool write_object(stamp* pv) {
		write_reference(pv);
		auto& bs = databases[pv->type];
		return serial(pv,
			((requisit*)bs.requisits.begin()) + 2,
			(requisit*)bs.requisits.end());
	}
	constexpr database_serial_bin(io::stream& file, bool writemode) : file(file), writemode(writemode) {}
};

bool metadata_readfile(const char* url) {
	io::file file(url, StreamRead);
	if(!file)
		return 0;
	return 0;
}

bool metadata::writefile(const char* url) {
	io::file file(url, StreamWrite);
	if(!file)
		return 0;
	database_serial_bin sb(file, true);
	for(auto& e : databases) {
		if(!e)
			continue;
		for(unsigned i = 0; i < e.count; i++)
			sb.write_object((stamp*)e.get(i));
	}
	stamp e = {};
	sb.serial(&e, sizeof(e));
	return 0;
}