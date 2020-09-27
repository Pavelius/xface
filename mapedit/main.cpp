#include "main.h"

void run_main();

int main() {
	run_main();
}

int __stdcall WinMain(void* ci, void* pi, char* cmd, int sw) {
	return main();
}