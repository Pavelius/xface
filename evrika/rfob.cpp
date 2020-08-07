#include "main.h"

void* rfob::ptr(rfob id) const {
	auto& d = databases[bytes[3]];
	auto p = d.ptr(dword & 0xFFFFFF);
	unsigned offset = 0;
	// ≈сли найдем нужный реквизит вернем его сразу
	for(auto& e : d.requisits) {
		if(e == id)
			return (char*)p + offset;
		offset += sizeof(e);
	}
	for(auto& e : bsdata<header>()) {
		if(e.parent == id.dword) {

		}
	}
	return 0;
}

int rfob::get(rfob id) const {
	auto p = ptr(id);
	if(!p)
		return 0;
	return *((int*)p);
}

void* rfob::ptr() const {
	return databases[bytes[3]].ptr(dword & 0xFFFFFF);
}