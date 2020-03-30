#include "xface/crt.h"
#include "xface/io.h"
#include "main.h"

using namespace code;

namespace {
class context {
	io::stream&		file;
	bool			write_mode;
	void serial(void* object, unsigned size) {
		if(write_mode)
			file.write(object, size);
		else
			file.read(object, size);
	}
	template<class T> void serial(T& e) {
		serial(&e, sizeof(e));
	}
	void serial(const char*& ps) {
		unsigned len = 0;
		char temp[256 * 32];
		char* ppt;
		if(write_mode) {
			if(ps)
				len = zlen(ps);
			file.write(&len, sizeof(len));
			if(len)
				file.write(ps, len);
		} else {
			file.read(&len, sizeof(len));
			ps = 0;
			ppt = temp;
			if(len > 0) {
				if(len > sizeof(temp) - 1)
					ppt = new char[len + 1];
				file.read(ppt, len);
			}
			ppt[len] = 0;
			ps = szdup(ppt);
			if(ppt != temp)
				delete ppt;
		}
	}
	void serial(void* object, const requisit& e) {
		if(e.type->isreference()) {
			auto type = e.type->type;
			if(type->isnumber() || type->istext() || type->isreference())
				return;
		} else if(e.type->isarray()) {

		} else if(e.type->istext())
			serial(*((const char**)object));
		else if(e.type->isnumber())
			serial(object, e.type->size);
		else
			serial(object, e.type);
	}
	void serial(void* object, const metadata* pm) {
		if(!pm)
			return;
		for(auto& e : bsdata<requisit>()) {
			if(e.parent != pm)
				continue;
			if(e.count <= 1)
				serial(e.ptr(object), e);
			else {
				for(unsigned i = 0; i < e.count; i++)
					serial(e.ptr(object, i), e);
			}
		}
	}
public:
	context(io::stream& file, bool write_mode) : file(file), write_mode(write_mode) {}
};
}

void metadata::write(const char* url) const {
	io::file file(url, StreamWrite);
	if(!file)
		return;
	context e(file, true);
	auto p = addtype("*Character");
	//e.serial(e.header);
}