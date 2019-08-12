#include "anyreq.h"

int anyreq::get(void* object) const {
	switch(size) {
	case 1: return *((char*)object);
	case 2: return *((short*)object);
	case 4: return *((int*)object);
	default: return 0;
	}
}

const char* anyreq::gets(void* object) const {
	if(size != sizeof(char*))
		return "";
	auto p = *((const char**)object);
	return p ? p : "";
}

void anyreq::set(void* object, int value) const {
	switch(size) {
	case 1: *((char*)object) = (char)value; break;
	case 2: *((short*)object) = (short)value; break;
	case 4: *((int*)object) = (int)value; break;
	default: break;
	}
}