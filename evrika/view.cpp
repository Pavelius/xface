#include "xface/draw_control.h"
#include "main.h"

static void test_database() {
	database db;
	db.size = sizeof(document);
	for(auto i = 0; i < 70; i++) {
		auto p = (document*)db.add();
		memset(p, 0, sizeof(document));
	}
}

int main() {
	test_database();
	draw::application("Evrika", true);
}

int __stdcall WinMain(void* ci, void* pi, char* cmd, int sw) {
	return main();
}