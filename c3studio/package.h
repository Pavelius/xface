#include "crt.h"
#include "stringbuilder.h"

#pragma once

namespace code {
enum operation {
	Number, Literal, Symbol,
	Statement, Assign, If, While, Do,
	Add, Sub, Mul, Div, DivPercent, Index,
	Greater, GreaterEqual, Lesser, LesserEqual, Equal, NotEqual,
	LogicalAnd, LogicalOr,
	And, Or, Xor, ShiftLeft, ShiftRight,
	Not, Ref, Der, Negate,
	SizeOf, Return, Call, Point, Break, Continue
};
enum flag_s : unsigned char {
	Public, Static, Method, Const, Parameter
};
enum symbol_s : unsigned {
	This,
	I32 = 0xFFFFFF00, I16, I8, U32, U16, U8, Void, Bool,
	PI8, PVoid,
	True, False,
	Pointer, Platform, Class,
	SecStr, SecData, SecCode, SecBSS, SecLoc,
	None = 0xFFFFFFFF,
	FirstDef = I32, LastDef = SecLoc
};
enum class message {
	UnexpectedSymbols, Expected1p,
	InvalidEscapeSequence, InvalidOperationWithType, InvalidPointerOperation,
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
	DivisionByZero,
	Options, Compilator, Linker,
	StartParse, EndParse, Link1p, Keyword, Declare,
};
typedef cflags<flag_s>	flagf;
typedef unsigned		pckh;
class stringv : public array {
	unsigned			find(const char* v, unsigned len) const;
	unsigned			add(const char* v, unsigned len);
public:
	stringv() : array(1) {}
	unsigned			add(const char* v);
	unsigned			find(const char* v) const;
	const char*			get(unsigned v) const { return (v < count) ? (const char*)ptr(v) : ""; }
};
struct ast {
	operation			type;
	unsigned			right;
	unsigned			left;
	constexpr bool isliteral() const { // left if literal string
		return type == operation::Literal;
	}
	constexpr bool issymbol() const { // left if symbol
		return type == operation::Symbol;
	}
	constexpr bool isnumber() const {
		return type == operation::Number;
	}
};
struct symbol {
	pckh				id = None; // String identificator
	pckh				parent = None; // Parent symbol
	pckh				result = None; // Result symbol type of expression
	unsigned			index = 0; // Position in source file
	unsigned			flags = 0;
	unsigned			local;
	pckh				ast = None; // Abstract syntaxis tree index
	constexpr bool		is(flag_s v) const { return (flags & (1 << v)) != 0; }
	constexpr void		set(flag_s v) { flags |= 1 << v; }
};
class package {
	stringv				strings;
	unsigned			find(operation type, unsigned left, unsigned right) const;
	symbol*				getsymbol(pckh v) const;
	bool				serial(const char* url, bool write_mode);
public:
	vector<symbol>		symbols;
	vector<ast>			asts;
	explicit operator bool() const { return strings.getcount()!=0; }
	pckh				addstr(const char* v);
	unsigned			add(operation type, unsigned left, unsigned right);
	unsigned			add(operation type, unsigned left) { return add(type, left, 0); }
	unsigned			addsym(pckh id, pckh parent, pckh result, unsigned index, unsigned flags, unsigned level);
	static void			addurl(stringbuilder& sb, const char* url, const char* id, const char* ext);
	static void			build(const char* module, const char* start);
	void				clear();
	void				compile(const char* text);
	static bool			compile(const char* url, const char* id);
	static const char*	def_strings[];
	static symbol		def_symbols[];
	pckh				findstr(const char* v);
	pckh				findsym(pckh id, pckh parent, unsigned level) const;
	static bool			findurl(stringbuilder sb, const char* id, const char* ext);
	void				getast(stringbuilder& sb, unsigned v) const;
	const ast*			getast(pckh v) const;
	unsigned			getresult(pckh sym) const;
	unsigned			getindex(pckh v) const;
	pckh				getleft(pckh v) const;
	pckh				getparent(pckh v) const;
	const char*			getstr(pckh v) const;
	void				getsym(stringbuilder& sb, pckh v) const;
	pckh				getsymast(pckh sym) const;
	const char*			getsymstr(pckh sym) const;
	const char*			getsymurl(pckh sym) const;
	bool				ismethod(pckh sym) const;
	bool				ispointer(pckh sym) const;
	bool				issymbol(pckh ast) const;
	void				print(stringbuilder& sb);
	void				read(const char* url) { serial(url, false); }
	pckh				reference(pckh v);
	void				setast(pckh sym, pckh ast);
	void				write(const char* url) { serial(url, true); }
};
namespace urls {
extern const char*		library;
extern const char*		project;
}
}