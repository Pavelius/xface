#include "mutex.h"

extern "C" {
int __stdcall CloseHandle(void* hObject);
int __stdcall ReleaseMutex(void* hMutex);
int __stdcall WaitForSingleObject(void* hHandle, int dwMilliseconds);
void* __stdcall CreateMutexA(void* lpMutexAttributes, int bInitialOwner, const char* lpName);
}

io::mutex::mutex() {
	s = (int)CreateMutexA(0, 0, 0);
}

io::mutex::~mutex() {
	CloseHandle((void*)s);
}

void io::mutex::lock() {
	WaitForSingleObject((void*)s, 0xFFFFFFFF);
}

void io::mutex::unlock() {
	ReleaseMutex((void*)s);
}