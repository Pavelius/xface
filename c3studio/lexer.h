#include "pointl.h"

#pragma once

enum group_s : unsigned char {
	IllegalSymbol, Comment,
	Operator, Keyword,
	Number, String, Identifier, Type, Member, Constant,
	WhiteSpace,
};
struct lexer {
	struct word {
		const char*			name;
		group_s				type;
		unsigned			size;
		constexpr word() : name(0), type(IllegalSymbol), size(0) {}
		constexpr word(const char* name, group_s type) : name(name), type(type), size(len(name)) {}
		static constexpr const char* end(const char* p) { while(*p) p++; return p; }
		static constexpr unsigned len(const char* p) { return end(p) - p; }
	};
	struct block {
		char				open;
		char				close;
		constexpr bool operator==(const char sym) const { return open == sym || close == sym; }
	};
	const char*				name;
	const char*				extensions;
	const char*				operators;
	const char*				operators2;
	const word*				keywords;
	unsigned				keywords_count;
	block					statement, expression, scope;
	const char*				increase;
	const word*				find(const char* sym, unsigned size) const;
	static void				get(const char* pb, int p1, pointl& pos1, int p2, pointl& pos2, pointl& size, const pointl origin, int& origin_index);
	static const char*		next(const char* p, pointl& pos);
	const char*				next(const char* p, pointl& pos, group_s& type) const;
};