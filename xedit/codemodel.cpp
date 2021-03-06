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
	qsort(words.begin(), words.getcount(), sizeof(words.begin()[0]), compare_keywords);
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
	if(!value)
		return;
	auto len = zlen(value);
	reserve(len + 1);
	memcpy(data, value, len + 1);
	count = len + 1;
}

bool codemodel::iswhitespace(char sym) const {
	return sym == 0x20 || sym == '\t';
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

bool codemodel::isnextline(const char* ps, const char** pv) const {
	if(*ps == 10) {
		ps++;
		if(*ps == 13)
			*ps++;
		if(pv)
			*pv = ps;
		return true;
	} else if(*ps == 13) {
		ps++;
		if(*ps == 10)
			*ps++;
		if(pv)
			*pv = ps;
		return true;
	}
	return false;
}

bool codemodel::istype(const char* source, const typei** pv) const {
	if(!parser)
		return false;
	auto kw = parser->find(source);
	if(!kw)
		return false;
	if(pv)
		*pv = kw;
	return true;
}

bool codemodel::isliteral(const char* ps, const char** pv) const {
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

bool codemodel::isidentifier(char sym) const {
	return ischa(sym) || sym == '_' || (sym >= '0' && sym <= '9');
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

int codemodel::getnext(const char* ps, point& pos, group_s& type) const {
	const typei* pt;
	const lexer::word* kw;
	type = IllegalSymbol;
	const char* p1 = ps;
	if(*ps == 0)
		type = WhiteSpace;
	else if(iswhitespace(ps, &ps))
		type = WhiteSpace;
	else if(isnextline(ps, &ps)) {
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
		if(istype(p1, &pt))
			type = Type;
		else
			type = Identifier;
	} else
		ps++;
	pos.x += ps - p1;
	return ps - p1;
}

int codemodel::getlenght() const {
	return count;
}

int	codemodel::lineb(int index) const {
	auto p = ptr(index);
	while(p > data) {
		if(p[-1] == 10 || p[-1] == 13)
			break;
		p--;
	}
	return p - begin();
}

int	codemodel::linee(int index) const {
	auto p = ptr(index);
	while(p > data) {
		if(p[0] == 10 || p[0] == 13)
			break;
		p++;
	}
	return p - begin();
}

int	codemodel::getindex(point pt) const {
	point pt1, pt2, size;
	int p1 = -1, p2 = -1, r3;
	getstate(p1, pt1, p2, pt2, size, pt, r3);
	return r3;
}

void codemodel::getstate(int p1, point& pos1, int p2, point& pos2, point& size, const point origin, int& origin_index) const {
	const char* pb = begin();
	const char* pe = end();
	point pos = {0, 0};
	pos1 = {-1, -1};
	pos2 = {-1, -1};
	size = {0, 0};
	origin_index = -1;
	while(true) {
		auto i = pb - begin();
		if(i == p1)
			pos1 = pos;
		if(i == p2)
			pos2 = pos;
		if(origin == pos)
			origin_index = i;
		if(pb >= pe || *pb == 0 || isnextline(pb, &pb)) {
			if(size.x < pos.x)
				size.x = pos.x;
			if(origin.y == pos.y && origin_index == -1)
				origin_index = i;
			pos.x = 0;
			pos.y++;
			if(pb >= pe || *pb == 0)
				break;
		} else {
			pos.x++;
			pb++;
		}
	}
	if(p1 == p2 || p2 == -1)
		pos2 = pos1;
	size.y = pos.y;
}

int	codemodel::skipsp(int index) const {
	auto p = ptr(index);
	auto pe = end();
	while(p < pe && iswhitespace(*p))
		p++;
	return p - begin();
}

int	codemodel::skipnsp(int index) const {
	auto p = ptr(index);
	auto pe = end();
	while(p < pe && !iswhitespace(*p))
		p++;
	return p - begin();
}

void parseri::addtype(const char* id) {
	auto p = types.add();
	p->name = szdup(id);
	p->size = zlen(id);
}

const typei* parseri::find(const char* sym) const {
	for(auto& e : types) {
		auto n = e.size;
		if(memcmp(sym, e.name, e.size) == 0)
			return &e;
	}
	return 0;
}