#include "variable.h"

int variable::get() const {
	switch(size) {
	case sizeof(char) : return *((char*)data);
	case sizeof(short) : return *((short*)data);
	case sizeof(int) : return *((int*)data);
	default: return 0;
	}
}

void variable::set(int value) const {
	switch(size) {
	case sizeof(char) : *((char*)data) = value; break;
	case sizeof(short) : *((short*)data) = value; break;
	case sizeof(int) : *((int*)data) = value; break;
	default: break;
	}
}
