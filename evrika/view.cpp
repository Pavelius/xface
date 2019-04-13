#include "xface/draw_control.h"
#include "main.h"

static bool test_database() {
	const unsigned real_db_count = 72;
	unsigned db_count;
	database db;
	db_count = db.getcount();
	db.size = sizeof(document);
	for(auto i = 0; i < real_db_count; i++) {
		auto p = (document*)db.add();
		memset(p, 0, sizeof(document));
	}
	db_count = db.getcount();
	return db_count == real_db_count;
}

int main() {
	if(!test_database())
		return 0;
	draw::application("Evrika", true);
}

int __stdcall WinMain(void* ci, void* pi, char* cmd, int sw) {
	return main();
}