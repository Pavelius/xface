#include "pointl.h"

#pragma once

enum group_s : unsigned char {
	IllegalSymbol, WhiteSpace, Comment,
	Operator, Keyword,
	Number, String, Identifier, Type,
};
struct lexer {
	struct word {
		const char*			name;
		group_s				type;
		unsigned			size;
	};
	const char*				name;
	const char*				extensions;
	const word*				source_begin;
	const word*				source_end;
	constexpr const word*	begin() const { return source_begin; }
	constexpr const word*	end() const { return source_end; }
	const word*				find(const char* sym) const;
	static void				get(const char* pb, int p1, pointl& pos1, int p2, pointl& pos2, pointl& size, const pointl origin, int& origin_index);
	int						getnext(const char* ps, pointl& pos, group_s& type) const;
	bool					iskeyword(const char* source, const lexer::word** pv) const;
	void					set(lexer::word* v, unsigned count);
};