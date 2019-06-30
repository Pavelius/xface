#include "anyval.h"

anyval::operator int() const {
	if(data) {
		if(bit_size) {

		} else {
			switch(size) {
			case sizeof(int) : return *((int*)data);
			case sizeof(short) : return *((short*)data);
			case sizeof(char) : return *((char*)data);
			}
		}
	}
	return 0;
}

void anyval::operator=(const int v) const {
	if(!data)
		return;
	if(bit_size) {

	} else {
		switch(size) {
		case sizeof(int) : *((int*)data) = v; break;
		case sizeof(short) : *((short*)data) = v; break;
		case sizeof(char) : *((char*)data) = v; break;
		}
	}
}