#include "main.h"

arrayseq* arrayseq::last() {
	for(auto p = this; p; p = p->next) {
		if(!p->next)
			return p;
	}
	return 0;
}

unsigned arrayseq::getcount() const {
	if(!this)
		return 0;
	auto result = count;
	for(auto p = next; p; p = p->next)
		result += p->count;
	return result;
}