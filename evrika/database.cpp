#include "main.h"

database* database::last() {
	for(auto p = this; p; p = p->next) {
		if(!p->next)
			return p;
	}
	return 0;
}

void* database::add() {
	for(auto p = this; p && p->count < p->count_maximum; p = p->next)
		return (char*)p->data + (p->count++)*size;
	auto p = last();
}