#include "crt.h"
#include "lexer.h"

static int compare(const void* v1, const void* v2) {
	auto p1 = (lexer::word*)v1;
	auto p2 = (lexer::word*)v2;
	if(p1->size != p2->size)
		return p1->size - p2->size;
	return strcmp(p1->name, p2->name);
}

void lexer::set(lexer::word* p, unsigned count) {
	source_begin = p;
	source_end = p + count;
	auto pe = p + count;
	for(auto pb = p; pb < pe; pb++)
		pb->size = zlen(pb->name);
}

const lexer::word* lexer::find(const char* sym) const {
	for(auto& e : *this) {
		auto n = e.size;
		if(memcmp(sym, e.name, e.size) == 0)
			return &e;
	}
	return 0;
}

static bool isnextline(const char* ps, const char** pv) {
	if(*ps == 10) {
		ps++;
		if(*ps == 13)
			*ps++;
		*pv = ps;
		return true;
	} else if(*ps == 13) {
		ps++;
		if(*ps == 10)
			*ps++;
		*pv = ps;
		return true;
	}
	return false;
}

static bool isliteral(const char* ps, const char** pv) {
	if(*ps != '\"')
		return false;
	ps++;
	while(*ps && *ps != '\"') {
		if(*ps == '\\')
			ps++;
		ps++;
	}
	if(*ps)
		ps++;
	if(pv)
		*pv = ps;
	return true;
}

static bool isidentifier(char sym) {
	return ischa(sym) || sym == '_' || (sym >= '0' && sym <= '9');
}

static bool isidentifier(const char* ps, const char** v) {
	if(ischa(*ps) || *ps == '_') {
		while(ischa(*ps) || isnum(*ps) || *ps == '_')
			ps++;
		if(v)
			*v = ps;
		return true;
	}
	return false;
}

bool lexer::iskeyword(const char* source, const lexer::word** pv) const {
	auto kw = find(source);
	if(!kw)
		return false;
	if(pv)
		*pv = kw;
	return true;
}

static int tabulator_spaces = 4;

int lexer::getnext(const char* ps, pointl& pos, group_s& type) const {
	const lexer::word* kw;
	type = IllegalSymbol;
	const char* p1 = ps;
	if(*ps == 0)
		type = WhiteSpace;
	else if(*ps == '\t') {
		type = WhiteSpace;
		while(*ps == '\t') {
			pos.x = ((pos.x / tabulator_spaces) + 1) * tabulator_spaces;
			ps++;
		}
		return ps - p1;
	} else if(*ps == 0x20) {
		type = WhiteSpace;
		while(*ps == 0x20)
			ps++;
	} else if(isnextline(ps, &ps)) {
		type = WhiteSpace;
		pos.y++;
		pos.x = 0;
		return ps - p1;
	} else if((ps[0] == '-' && (ps[1] >= '0' && ps[1] <= '9')) || (ps[0] >= '0' && ps[0] <= '9')) {
		type = Number;
		if(ps[0] == '-')
			ps++;
		while(*ps && *ps >= '0' && *ps <= '9')
			ps++;
	} else if(isliteral(ps, &ps))
		type = String;
	else if(ps[0] == '/' && ps[1] == '/') {
		type = Comment;
		ps += 2;
		while(*ps) {
			if(isnextline(ps, &ps)) {
				pos.y++;
				pos.x = 0;
				return ps - p1;
			} else
				ps++;
		}
	} else if(iskeyword(ps, &kw)) {
		ps += kw->size;
		type = kw->type;
	} else if(isidentifier(ps, &ps)) {
		type = Identifier;
	} else
		ps++;
	pos.x += ps - p1;
	return ps - p1;
}

void lexer::get(const char* pb, int p1, pointl& pos1, int p2, pointl& pos2, pointl& size, const pointl origin, int& origin_index) {
	auto p = pb;
	pointl pos = {0, 0};
	pos1 = {-1, -1};
	pos2 = {-1, -1};
	size = {0, 0};
	origin_index = -1;
	while(true) {
		auto i = p - pb;
		if(i == p1)
			pos1 = pos;
		if(i == p2)
			pos2 = pos;
		if(origin == pos)
			origin_index = i;
		if(*p == 0 || isnextline(p, &p)) {
			if(size.x < pos.x)
				size.x = pos.x;
			if(origin.y == pos.y && origin_index == -1)
				origin_index = i;
			pos.x = 0;
			pos.y++;
			if(*p == 0)
				break;
		} else {
			pos.x++;
			p++;
		}
	}
	if(p1 == p2 || p2 == -1)
		pos2 = pos1;
	size.y = pos.y;
}