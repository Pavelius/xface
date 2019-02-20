#include "valuelist.h"

int valuelist::findvalue(int value) const {
	for(auto& e : *this) {
		if(e.value == value)
			return &e - data;
	}
	return -1;
}