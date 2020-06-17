#include "main.h"

// 1) Нет ссылок, только индексы

static void change_add(void* p1, void* p2, unsigned s1, unsigned s2, unsigned count, unsigned s3) {
	for(unsigned i = 0; i < count; i++) {
		memmove(p1, p2, s3);
		p1 = (char*)p1 - s1;
		p2 = (char*)p2 - s2;
	}
}