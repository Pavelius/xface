#include "xface/draw_control.h"
#include "main.h"

static bool test_write() {
	return metadata::writefile("test.edb");
}

int main() {
	metadata::initialize();
	draw::application("Evrika", true);
}

int __stdcall WinMain(void* ci, void* pi, char* cmd, int sw) {
	return main();
}