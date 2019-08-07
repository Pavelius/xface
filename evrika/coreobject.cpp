#include "main.h"

objecti* objecti::write() {
	stampi::write();
	change.date = datetime::now();
	change.user = current_user;
	auto p = (objecti*)getreference();
	if(!p) {
		auto& db = getbase();
		p = (objecti*)db.add();
		memcpy(p, this, db.getsize());
	}
	return p;
}