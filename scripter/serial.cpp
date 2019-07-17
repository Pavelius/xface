#include "xface/archive.h"
#include "xface/strlib.h"
#include "main.h"

using namespace code;

class type_info;

struct pointers : aref<void*> {
	arem<type_info*>	types;
	template<class T> unsigned add(T* object) {
		return add(object, typeid(T));
	}
	template<class T> unsigned add(const T* object) {
		return add((void*)object, typeid(T));
	}
	unsigned add(void* object, const type_info& id) {
		return 0;
	}
	unsigned add(void* object) {
		if(!object)
			return 0;
		auto i = indexof(object);
		if(i == -1) {
			i = count;
			add(object);
		}
		return i;
	}
};

struct serial_metadata {
	strlib			strings;
};

void metadata::write(const char* url) const {
	serial_metadata context;
}