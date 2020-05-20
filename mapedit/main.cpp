#include "main.h"

tileset*			current_tileset;

void run_main();

int main() {
	tileset::add("test");
	run_main();
}

int __stdcall WinMain(void* ci, void* pi, char* cmd, int sw) {
	return main();
}