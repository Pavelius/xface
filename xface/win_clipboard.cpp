#include "crt.h"
#include "win.h"

namespace clipboard {

void copy(const void* string, int lenght) {
	if(!lenght)
		return;
	if(!OpenClipboard(0))
		return;
	EmptyClipboard();
	auto size = (lenght + 1) * sizeof(short);
	void* hglbCopy = GlobalAlloc(GMEM_MOVEABLE, size);
	if(!hglbCopy) {
		CloseClipboard();
		return;
	}
	// Lock the handle and copy the text to the buffer.
	auto ps = (char*)GlobalLock(hglbCopy);
	szencode(ps, size, CPU16LE, (const char*)string, lenght, CP1251);
	GlobalUnlock(ps);
	SetClipboardData(CF_UNICODETEXT, hglbCopy);
	CloseClipboard();
}

char* paste() {
	auto format = CF_UNICODETEXT;
	if(!IsClipboardFormatAvailable(format)) {
		format = CF_TEXT;
		if(!IsClipboardFormatAvailable(format))
			return 0;
	}
	if(!OpenClipboard(0))
		return 0;
	void* hglb = GetClipboardData(format);
	char* buffer = 0;
	if(hglb) {
		auto result = GlobalSize(hglb);
		if(result) {
			buffer = new char[result + 1];
			char* p = (char*)GlobalLock(hglb);
			if(p) {
				memcpy(buffer, p, result);
				GlobalUnlock(hglb);
			}
			if(format == CF_UNICODETEXT) {
				szencode(buffer, result, CP1251, buffer, result, CPU16LE);
				result = result / 2;
				buffer[result] = 0;
			}
		}
	}
	CloseClipboard();
	return buffer;
}

}