#include "main.h"

coreobject* coreobject::write() {
	timestamp::write();
	change_date = datetime::now();
	change_user = current_user;
	auto p = (coreobject*)getreference();
	if(!p) {
		auto& db = databases[type];
		p = (coreobject*)db.add();
		memcpy(p, this, db.size);
	}
	return p;
}