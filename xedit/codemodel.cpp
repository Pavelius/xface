#include "codemodel.h"

static int compare_keywords(const void* v1, const void* v2) {
	auto p1 = (lexer::word*)v1;
	auto p2 = (lexer::word*)v2;
	//auto d = p1->size - p2->size;
	//if(d != 0)
	//	return d;
	return strcmp(p1->name, p2->name);
}

void lexer::sort() {
	qsort(words.data, words.count, sizeof(words.data[0]), compare_keywords);
}

void lexer::add(const char* keyword_name, group_s type) {
	word e = {};
	e.name = szdup(keyword_name);
	e.size = zlen(e.name);
	e.type = type;
	words.add(e);
}

const lexer::word* lexer::find(const char* sym) const {
	for(auto& e : words) {
		auto n = e.size;
		if(memcmp(sym, e.name, e.size) == 0)
			return &e;
	}
	return 0;
}

void codemodel::set(const char* value) {
	auto len = zlen(value);
	reserve(len + 1);
	memcpy(data, value, len + 1);
	changing();
}

bool codemodel::iswhitespace(char sym) const {
	if(lex && lex->whitespaces)
		return zchr(lex->whitespaces, sym) != 0;
	return zchr(" \t", sym) != 0;
}

bool codemodel::iswhitespace(const char* sym, const char** v) const {
	if(!iswhitespace(*sym))
		return false;
	auto pb = sym;
	while(iswhitespace(*sym))
		sym++;
	if(v)
		*v = sym;
	return true;
}

bool codemodel::iskeyword(const char* source, const lexer::word** pv) const {
	if(!lex)
		return false;
	auto kw = lex->find(source);
	if(!kw)
		return false;
	if(pv)
		*pv = kw;
	return true;
}

bool codemodel::isidentifier(const char* ps, const char** v) const {
	if(ischa(*ps) || *ps == '_') {
		while(ischa(*ps) || isnum(*ps) || *ps == '_')
			ps++;
		if(v)
			*v = ps;
		return true;
	}
	return false;
}

void codemodel::getnext(codepos& e) const {
	if(!data)
		return;
	const lexer::word* kw;
	e.type = IllegalSymbol;
	e.to = e.from;
	const char* ps = data + e.from;
	if(*ps == 0)
		e.type = WhiteSpace;
	else if(iswhitespace(ps, &ps))
		e.type = WhiteSpace;
	else if((ps[0] == '-' && (ps[1] >= '0' && ps[1] <= '9')) || (ps[1] >= '0' && ps[1] <= '9')) {
		if(ps[0] == '-')
			ps++;
		while(*ps && *ps >= '0' && *ps <= '9')
			ps++;
		e.type = Number;
	} else if(ps[0] == '/' && ps[1] == '/') {
		ps += 2;
		e.type = Comment;
		while(*ps && *ps != 13 && *ps != 10)
			ps++;
	} else if(iskeyword(ps, &kw)) {
		ps += kw->size;
		e.type = kw->type;
	} else if(isidentifier(ps, &ps))
		e.type = Identifier;
	e.to = ps - data;
}

int	codemodel::getbegin() const {
	if(p2 == -1)
		return p1;
	return imin(p1, p2);
}

int	codemodel::getend() const {
	if(p2 == -1)
		return p1;
	return imax(p1, p2);
}

void codemodel::clear() {
	if(p2 != -1 && p1 != p2 && data) {
		auto s1 = data + getbegin();
		auto s2 = data + getend();
		while(*s2)
			*s1++ = *s2++;
		*s1 = 0;
		changing();
		if(p1 > p2)
			p1 = p2;
	}
	p2 = -1;
}

void codemodel::paste(const char* input) {
	clear();
	auto i2 = zlen(input);
	auto count = getlenght();
	if(p1 + i2 > count)
		reserve(p1 + i2 + 1);
	memmove(data + p1 + i2, data + p1, (count - p1 + 1) * sizeof(char));
	memcpy(data + p1, input, i2); count += i2;
	changing();
	set(p1 + i2, false);
}

int codemodel::getlenght() const {
	return count;
}

void codemodel::set(int index, bool shift) {
	if(index < 0)
		index = 0;
	else if(index > getlenght())
		index = count;
	if(shift) {
		if(p2 == -1)
			p2 = p1;
	} else
		p2 = -1;
	p1 = index;
}

void codemodel::correct() {
	auto lenght = getlenght();
	if(p2 != -1 && p2 > lenght)
		p2 = lenght;
	if(p1 > lenght)
		p1 = lenght;
	if(p1 < 0)
		p1 = 0;
}

void codemodel::left(bool shift, bool ctrl) {
	auto n = p1;
	if(!ctrl)
		n -= 1;
	else {
//		for(; n > 0 && isspace(string[n - 1]); n--);
//		for(; n > 0 && !isspace(string[n - 1]); n--);
	}
	set(n, shift);
}

void codemodel::right(bool shift, bool ctrl) {
	auto n = p1;
	if(!ctrl)
		n += 1;
	else {
//		for(; string[n] && !isspace(string[n]); n++);
//		for(; string[n] && isspace(string[n]); n++);
	}
	set(n, shift);
}