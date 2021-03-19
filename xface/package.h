#include "crt.h"

#pragma once

namespace code {
enum class base : unsigned char {
	String, Symbol, AST,
};
enum class opr : unsigned char {
	Nop,
	Constant, Symbol,
	Statement, Assign,
	Add, Sub, Mul, Div
};
enum class flag : unsigned char {
	Public, Static,
};
enum class msg : unsigned char {
	UnexpectedSymbols, Expected1p,
	InvalidEscapeSequence,
	Invalid1p2pIn3p,
	CantFind1pWithName2p,
	Operation1pNotUsedWithOperands2pAnd3p,
	NeedSymbol, NeedConstantExpression, NeedIndentifier, NeedFunctionMember, NeedPointerOrArray, NeedLValue,
	LongNotUseWithThisType, ShortNotUseWithThisType, UnsignedNotUseWithThisType,
	DontUse2pTimes,
	AlreadyDefined,
	ArrayOverflow, ImportAlreadyHavePseudonim, ModuleAlreadyImported,
	UnknownInstance, VoidReturnValue,
	Keyword1pUsedWithout2p,
	AssigmentWithoutEnumeratorMember, ExpectedEnumeratorMember,
	SectionNumber,
	WrongParamNumber,
	NotImplement1p2p,
	NotFound1p2p,
	CastType1pTo2p,
	Options, Compilator, Linker,
	StartParse, EndParse, Link1p, Keyword, Declare,
};
typedef cflags<flag>	flagf;
class package {
	struct ast {
		opr				type;
		unsigned		right;
		unsigned		left;
		bool			isdeterminal() const { return type == opr::Constant || type == opr::Symbol; }
	};
	struct symbol {
		unsigned		id;
		unsigned		offset;
		unsigned		parent;
		flagf			flags;
	};
	vector<char>		strings;
	vector<ast>			asts;
	vector<symbol>		symbols;
	unsigned			find(const char* v, unsigned len);
	unsigned			find(opr type, unsigned left, unsigned right) const;
public:
	void				clear();
	unsigned			add(const char* v, unsigned len);
	unsigned			add(const char* v);
	unsigned			add(opr type, unsigned left, unsigned right);
	unsigned			add(opr type, unsigned right) { return add(type, 0, right); }
	ast*				getast(unsigned v) const { return asts.ptr(v); }
	const char*			getstr(unsigned v) const { return v ? strings.ptr(v) : ""; }
	base				getbase(unsigned v) const { return base(v >> 30); }
};
}
