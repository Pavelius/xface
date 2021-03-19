#include "thread.h"

extern "C" {
	int __stdcall CloseHandle(void* hObject);
	void* __stdcall CreateThread(void* lpThreadAttributes, int dwStackSize, io::thread::fnroutine lpStartAddress,
		void* lpParameter, int dwCreationFlags, unsigned* lpThreadId);
	int __stdcall ReleaseMutex(void* hMutex);
	int __stdcall WaitForSingleObject(void* hHandle, int dwMilliseconds);
	void* __stdcall CreateMutexA(void* lpMutexAttributes, int bInitialOwner, const char* lpName);
}

io::thread::thread(fnroutine proc, void* v) {
	unsigned ThreadID;
	s = (int)CreateThread(
		0,			// default security attributes
		0,          // default stack size
		proc,
		v,          // thread function arguments
		0,          // default creation flags
		&ThreadID); // receive thread identifier
}

io::thread::~thread() {
	CloseHandle((void*)s);
}

void io::thread::join() {
	WaitForSingleObject((void*)s, 0xFFFFFFFF);
}