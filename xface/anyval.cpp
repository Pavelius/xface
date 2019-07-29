#include "anyval.h"

int anyval::get() const {
	if(!data)
		return 0;
	switch(size) {
	case sizeof(int) : return *((int*)data);
	case sizeof(short) : return *((short*)data);
	case sizeof(char) : return *((char*)data);
	}
	return 0;
}

void anyval::set(const int v) const {
	if(!data)
		return;
	switch(size) {
	case sizeof(int) : *((int*)data) = v; break;
	case sizeof(short) : *((short*)data) = v; break;
	case sizeof(char) : *((char*)data) = v; break;
	}
}