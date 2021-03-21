#include "crt.h"
#include "lexer.h"

static int tabulator_spaces = 4;

const lexer::word* lexer::find(const char* sym, unsigned size) const {
	auto pe = keywords + keywords_count;
	for(auto pb = keywords; pb < pe; pb++) {
		if(pb->size != size)
			continue;
		if(memcmp(pb->name, sym, size) == 0)
			return pb;
	}
	return 0;
}

const char* lexer::next(const char* p, pointl& pos) {
	switch(*p) {
	case 0:
		break;
	case 0x09:
		p++;
		pos.x = ((pos.x / tabulator_spaces) + 1) * tabulator_spaces;
		break;
	case 10:
		pos.x = 0; pos.y++;
		p++;
		if(*p == 13)
			p++;
		break;
	case 13:
		pos.x = 0; pos.y++;
		p++;
		if(*p == 10)
			p++;
		break;
	default:
		p++;
		pos.x++;
		break;
	}
	return p;
}

const char* lexer::next(const char* p, pointl& pos, group_s& type) const {
	type = IllegalSymbol;
	if(*p == 0)
		type = WhiteSpace;
	else if(*p == 0x20 || *p == 9 || *p == 10 || *p == 13) {
		type = WhiteSpace;
		while(*p == 0x20 || *p == 9 || *p == 10 || *p == 13)
			p = next(p, pos);
	} else if((*p == '-' && isnum(*p)) || isnum(*p)) {
		type = Number;
		if(*p == '-')
			p = next(p, pos);
		if(p[0] == '0' && p[1] == 'x') {
			p = next(p, pos);
			p = next(p, pos);
			while(isnum(*p) || (*p>='A' && *p<='F') || (*p >= 'a' && *p <= 'f'))
				p = next(p, pos);
		} else {
			while(isnum(*p))
				p = next(p, pos);
		}
	} else if(*p == '\"') {
		type = String;
		p = next(p, pos);
		while(*p && *p != '\"' && *p != 10 && *p != 13) {
			if(*p == '\\')
				p = next(p, pos);
			p = next(p, pos);
		}
		if(*p)
			p = next(p, pos);
	} else if(p[0] == '/' && p[1] == '/') {
		type = Comment;
		p = next(p, pos);
		p = next(p, pos);
		while(*p) {
			if(*p == 10 || *p == 13) {
				p = next(p, pos);
				break;
			} else
				p = next(p, pos);
		}
	} else if(ischa(*p) || *p == '_') {
		type = Identifier;
		auto pb = p;
		while(ischa(*p) || *p == '_' || isnum(*p))
			p = next(p, pos);
		auto pk = find(pb, p - pb);
		if(pk)
			type = pk->type;
	} else if(statement == *p || expression == *p || scope == *p) {
		type = Operator;
		pos.x++;
		return p + 1;
	} else {
		for(auto i = 0; operators2[i]; i += 2) {
			if(p[0] == operators2[i] && p[1] == operators2[i + 1]) {
				type = Operator;
				pos.x += 2;
				return p + 2;
			}
		}
		if(zchr(operators, *p)) {
			type = Operator;
			pos.x++;
			return p + 1;
		}
		return next(p, pos);
	}
	return p;
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
		if(*p == 10 || *p == 13 || *p == 0) {
			if(size.x < pos.x)
				size.x = pos.x;
			if(origin.y == pos.y && origin_index == -1)
				origin_index = i;
			if(*p == 0)
				break;
		}
		p = next(p, pos);
	}
	if(p1 == p2 || p2 == -1)
		pos2 = pos1;
	size.y = pos.y;
}