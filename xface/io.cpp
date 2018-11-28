#include "crt.h"
#include "io.h"

io::stream& io::stream::operator<<(const int n) {
	char temp[32];
	sznum(temp, n);
	return *this << temp;
}

io::stream&	io::stream::operator<<(const char* t) {
	if(!t)
		return *this;
	// ѕриведем формат строки из стандартной кодировки
	while(*t) {
		char temp[8];
		char* s1 = temp;
		unsigned u = szget(&t);
		szput(&s1, u, CPUTF8);
		write(temp, s1 - temp);
	}
	return *this;
}

unsigned char io::stream::get() {
	unsigned char r = 0;
	read(&r, 1);
	return r;
}

unsigned short io::stream::getLE16() {
	unsigned char u2 = get();
	unsigned char u1 = get();
	return (u2 << 8) | u1;
}

unsigned io::stream::getLE32() {
	unsigned char u4 = get();
	unsigned char u3 = get();
	unsigned char u2 = get();
	unsigned char u1 = get();
	return (u4 << 24) | (u3 << 16) | (u2 << 8) | u1;
}

void* loadb(const char* url, int* size, int additional) {
	void* p = 0;
	if(size)
		*size = 0;
	if(!url || url[0] == 0)
		return 0;
	io::file file(url, StreamRead);
	if(!file)
		return 0;
	int s = file.seek(0, SeekEnd) + additional;
	file.seek(0, SeekSet);
	p = new char[s];
	memset(p, 0, s);
	file.read(p, s);
	if(size)
		*size = s;
	return p;
}

char* loadt(const char* url, int* size) {
	int s1;
	if(size)
		*size = 0;
	unsigned char* p = (unsigned char*)loadb(url, &s1, 1);
	if(!p)
		return 0;
	if(p[0] == 0xEF && p[1] == 0xBB && p[2] == 0xBF) {
		// UTF8
		// ѕерекодируем блок через декодировщик. ћожет быть только меньше,
		// так как системна€ кодировка ANSI
		szencode((char*)p, s1, metrics::code, (char*)p + 3, s1, CPUTF8);
	}
	if(size)
		*size = s1;
	return (char*)p;
}

char* szurlc(char* result) {
	char* p = result;
	while(*p) {
		if(*p == '\\')
			*p = '/';
		p++;
	}
	return zskipspcr(result);
}

char* szurl(char* p, const char* path, const char* name, const char* ext, const char* suffix) {
	if(!p)
		return 0;
	*p = 0;
	if(path) {
		zcpy(p, path);
		zcat(p, "/");
	}
	if(name)
		zcat(p, name);
	if(suffix)
		zcat(p, suffix);
	if(ext && szext(p) == 0) {
		zcat(p, ".");
		zcat(p, ext);
	}
	return szurlc(p);
}

const char* szext(const char* path) {
	for(const char* r = zend((char*)path); r > path; r--) {
		if(*r == '.')
			return r + 1;
		else if(*r == '\\' || *r == '/')
			return 0;
	}
	return 0;
}

const char* szfname(const char* path) {
	for(const char* r = zend((char*)path); r > path; r--) {
		if(*r == '\\' || *r == '/')
			return r + 1;
	}
	return path;
}

char* szfnamewe(char* result, const char* name) {
	zcpy(result, szfname(name));
	char* p = (char*)szext(result);
	if(p && p != result)
		p[-1] = 0;
	return result;
}