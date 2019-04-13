#include "xface/draw_control.h"
#include "main.h"

static void initialize_databases() {
	databases[Reference].size = sizeof(reference);
	databases[Document].size = sizeof(document);
}

static bool test_database() {
	const unsigned real_db_count = 72;
	for(auto i = 0; i < real_db_count; i++) {
		document e;
		e.date = datetime::now().daybegin();
		e.text = "Произвольный комментарий";
		e.write();
		auto pd = e.getreference();
		if(!pd)
			return false;
	}
	auto db_count = databases[Document].getcount();
	return db_count == real_db_count;
}

static bool test_write() {
	return database::writefile("test.edb");
}

int main() {
	initialize_databases();
	if(!test_database())
		return 0;
	test_write();
	draw::application("Evrika", true);
}

int __stdcall WinMain(void* ci, void* pi, char* cmd, int sw) {
	return main();
}