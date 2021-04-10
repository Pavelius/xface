#include "package.h"
#include "section.h"
#include "io.h"

using namespace code;

static package*			current;
struct parsestate {
	const char*			p;
	const char*			pb;
	pckh				parent;
	unsigned			level;
	constexpr unsigned	getindex() const { return p - pb; }
};
static pckh				type_sizeof = U32;
static parsestate		ps;
static vector<unsigned>	locals;

static void status(message id, ...) {
}

static bool skipcm() {
	if(ps.p[0] == '/' && ps.p[1] == '/') {
		// Line comment
		ps.p += 2;
		while(*ps.p && *ps.p != '\n' && *ps.p != '\r')
			ps.p++;
		return true;
	} else if(ps.p[0] == '/' && ps.p[1] == '*') {
		// Comment
		ps.p += 2;
		while(*ps.p && !(ps.p[0] == '*' && ps.p[1] == '/'))
			ps.p++;
		return true;
	}
	return false;
}

static void next(const char* p1) {
	ps.p = p1;
	while(true) {
		ps.p = skipspcr(ps.p);
		if(skipcm())
			continue;
		break;
	}
}

static void next() {
	next(ps.p + 1);
}

static void skip(char sym) {
	if(*ps.p == sym)
		next();
	else {
		char temp[] = {sym, 0};
		status(message::Expected1p, "symbol", temp);
	}
}

static bool ischab(char sym) {
	return (sym >= 'A' && sym <= 'Z')
		|| (sym >= 'a' && sym <= 'z')
		|| (sym == '_');
}

static bool ischa(char sym) {
	return (sym >= '0' && sym <= '9')
		|| (sym >= 'A' && sym <= 'Z')
		|| (sym >= 'a' && sym <= 'z')
		|| (sym == '_');
}

static bool match(char sym) {
	if(*ps.p == sym) {
		next();
		return true;
	}
	return false;
}

static bool match(const char* name) {
	auto p1 = ps.p;
	auto p2 = name;
	while(name[0] && *p1) {
		if(*name++ != *p1++)
			return false;
	}
	if(name[0] || ischa(*p1))
		return false;
	next(p1);
	status(message::Keyword, p2);
	return true;
}

static const char* identifier() {
	if(!ischab(*ps.p))
		return 0;
	static char temp[256];
	auto s = temp;
	while(ischa(*ps.p))
		*s++ = *ps.p++;
	*s++ = 0;
	next(ps.p);
	return temp;
}

static unsigned parse_pointer(unsigned declared) {
	while(*ps.p == '*') {
		next(ps.p + 1);
		declared = current->reference(declared);
	}
	return declared;
}

static void seturl(stringbuilder& sb, const char* id, const char* url, const char* ext) {
	char t1[256]; stringbuilder s1(t1);
	s1.add(id); s1.change('.', '/');
	sb.clear();
	sb.add(url);
	sb.add(t1);
	sb.add(".");
	sb.add(ext);
}

static bool istype(unsigned* declared, unsigned& flags) {
	unsigned e = I32;
	auto result = false;
	auto m_public = 0;
	auto m_static = 0;
	while(*ps.p) {
		if(match("static")) {
			result = true;
			m_static++;
			continue;
		} else if(match("public")) {
			result = true;
			m_public++;
			continue;
		} else if(!ischab(*ps.p)) {
			if(result)
				status(message::Expected1p, "type");
			return false;
		}
		const char* p1 = ps.p;
		auto pid = identifier();
		auto id = current->addstr(pid);
		bool change_back = true;
		e = current->findsym(id, Class, 0);
		if(e != None) {
			// Типы определены всегда. Если не определен, значит это не тип.
			if(*ps.p == '*' || ischa(*ps.p)) {
				change_back = false;
				result = true;
			}
		}
		if(change_back)
			ps.p = p1;
		if(result) {
			if(m_static > 1)
				status(message::DontUse2pTimes, "static", m_static);
			if(m_public > 1)
				status(message::DontUse2pTimes, "public", m_public);
			if(m_static)
				flags |= 1 << Static;
			if(m_public)
				flags |= 1 << Public;
			*declared = e;
		}
		break;
	}
	return result;
}

static pckh expression();
static pckh statement();
static pckh initialize(pckh result);

static pckh initialize() {
	return initialize(None);
}

static void add_statements(pckh parent) {
	auto push = ps.parent;
	ps.parent = parent;
	ps.level = 0;
	current->setast(parent, statement());
	ps.parent = push;
}

// Чтобы можно было в секции данных размещать ресурсы и подтягивать их под переменные.
static bool declaration(unsigned parent, unsigned flags, bool allow_functions = true, bool allow_variables = true, bool unique_test = false, pckh* return_result = 0, unsigned level = 0) {
	unsigned declared;
	const char* p1 = ps.p;
	if(!istype(&declared, flags))
		return false;
	while(*ps.p) {
		unsigned result = parse_pointer(declared);
		auto ix = ps.getindex();
		auto id = current->addstr(identifier());
		if(*ps.p == '(' && !allow_functions) {
			ps.p = p1;
			return false;
		} else if(*ps.p != '(' && !allow_variables) {
			ps.p = p1;
			return false;
		}
		if(*ps.p == '(')
			flags |= 1 << Method;
		auto m2 = current->addsym(id, parent, result, ix, flags, level);
		if(*ps.p == '(') {
			next();
			if(*ps.p == ')')
				next();
			else {
				while(*ps.p) {
					pckh result;
					unsigned pflags = 1 << Parameter;
					if(istype(&result, pflags)) {
						result = parse_pointer(result);
						auto ix = ps.getindex();
						auto id = current->addstr(identifier());
						result = current->addsym(id, m2, result, ix, pflags, 0);
						if(*ps.p == '=')
							current->setast(result, expression());
					}
					if(*ps.p == ')') {
						next();
						break;
					}
					skip(',');
				}
			}
			add_statements(m2);
			return true;
		} else if(*ps.p == '[') {
			next();
			if(*ps.p == ']')
				next();
			else {
				auto number = expression();
				skip(']');
			}
			if(*ps.p != '=')
				current->setast(m2, initialize());
		} else if(*ps.p == '=') {
			next();
			if(return_result)
				*return_result = current->add(operation::Assign, current->add(operation::Symbol, m2), expression());
			else
				current->setast(m2, expression());
		}
		if(*ps.p == ';') {
			next();
			break;
		}
		skip(',');
	}
	return true;
}

static char next_string_symbol() {
	while(*ps.p) {
		if(*ps.p != '\\')
			return *ps.p++;
		ps.p++;
		switch(*ps.p++) {
		case 0:
			return 0;
		case 'n':
			return '\n';
		case 't':
			return '\t';
		case 'r':
			return '\r';
		case '\\':
			return '\\';
		case '\'':
			return '\'';
		case '\"':
			return '\"';
		case '\n':
			if(ps.p[0] == '\r')
				ps.p++;
			break;
		case '\r':
			if(ps.p[0] == '\n')
				ps.p++;
			break;
		default:
		{
			char temp[2] = {ps.p[-1], 0};
			status(message::InvalidEscapeSequence, temp);
		}
		break;
		}
	}
	return 0;
}

static int next_char() {
	char result[5];
	char* d1 = result;
	char* d2 = result + sizeof(result) / sizeof(result[0]) - 1;
	memset(result, 0, sizeof(result));
	while(*ps.p) {
		if(*ps.p == '\'') {
			next();
			break;
		}
		if(d1 < d2)
			*d1++ = next_string_symbol();
	}
	*d1++ = 0;
	return *((int*)&result);
}

static unsigned next_string() {
	unsigned char temp[section::minimal_count];
	section result(temp);
	while(*ps.p) {
		if(*ps.p == '\"') {
			next();
			if(*ps.p == '\"') {
				// if two string in row
				next();
				continue;
			} else if(*ps.p == '+' && ps.p[1] != '+') {
				// checking '+' between two strings
				const char* p1 = ps.p;
				next(ps.p + 1);
				if(*ps.p == '\"') {
					next();
					continue;
				}
				ps.p = p1;
			}
			break;
		}
		result.add((unsigned char)next_string_symbol());
	}
	result.add(0);
	return current->addstr((const char*)result.begin());
}

static int next_number() {
	int num = 0;
	if(ps.p[0] == '0') {
		if(ps.p[1] == 'x') {
			ps.p += 2;
			while(true) {
				char s = *ps.p;
				if(s >= 'a' && s <= 'f')
					s = s - 'a' + 10;
				else if(s >= 'A' && s <= 'F')
					s = s - 'A' + 10;
				else if(s >= '0' && s <= '9')
					s = s - '0';
				else
					break;
				num = num * 16 + s;
				ps.p++;
			}
		} else {
			while(*ps.p >= '0' && *ps.p <= '7') {
				num = num * 8 + *ps.p - '0';
				ps.p++;
			}
		}
	} else {
		while(*ps.p >= '0' && *ps.p <= '9') {
			num = num * 10 + *ps.p - '0';
			ps.p++;
		}
	}
	if(*ps.p == 'f' || *ps.p == 'e')
		ps.p++;
	next(ps.p);
	return num;
}

static unsigned parameters() {
	pckh result = None;
	while(true) {
		if(result == None)
			result = expression();
		else
			result = current->add(Statement, result, expression());
		if(ps.p[0] == ',') {
			next();
			continue;
		} else
			break;
	}
	return result;
}

static void postfix(pckh& result) {
	while(*ps.p) {
		if(*ps.p == '(') {
			next();
			result = current->add(operation::Call, result, parameters());
			skip(')');
		} else if(*ps.p == '.') {
			next();
			auto sid = current->addstr(identifier());
			if(result==This) {
				auto n = current->findsym(sid, This, 0);
				if(n != None) {
					result = current->add(operation::Symbol, n);
					continue;
				} else
					status(message::NotFound1p2p, "identifier", sid);
			}
			result = current->add(operation::Point, result, sid);
		} else if(*ps.p == '[') {
			next();
			result = current->add(operation::Index, result, expression());
			skip(']');
		} else if(ps.p[0] == '-' && ps.p[1] == '-') {
			next(ps.p + 2);
		} else if(ps.p[0] == '+' && ps.p[1] == '+') {
			next(ps.p + 2);
		} else
			break;
	}
}

static pckh unary() {
	pckh result = None;
	switch(ps.p[0]) {
	case '-':
		ps.p++;
		if(ps.p[0] == '-') {
			next();
			result = unary();
		} else {
			next();
			result = unary();
		}
		break;
	case '+':
		ps.p++;
		if(ps.p[0] == '+') {
			next();
			result = unary();
		} else {
			next();
			result = unary();
		}
		break;
	case '!':
		next();
		result = current->add(operation::Not, unary());
		break;
	case '*':
		next();
		result = current->add(operation::Der, unary());
		break;
	case '&':
		next();
		result = current->add(operation::Ref, unary());
		break;
	case '(':
		next();
		result = expression();
		skip(')');
		break;
	case '\"':
		ps.p++;
		result = current->add(operation::Literal, next_string(), PI8);
		break;
	case '\'':
		ps.p++;
		result = current->add(operation::Literal, next_string(), PI8);
		break;
	case '0':case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8':case '9':
		result = current->add(operation::Number, next_number(), I32);
		break;
	default:
		if(match("sizeof")) {
			skip('(');
			result = current->add(operation::SizeOf, expression(), type_sizeof);
			skip(')');
		} else if(match("this"))
			result = current->add(operation::Symbol, This);
		else if(match("true"))
			result = current->add(operation::Number, 1, Bool);
		else if(match("false"))
			result = current->add(operation::Number, 0, Bool);
		else if(ischa(*ps.p)) {
			auto n = identifier();
			auto ni = current->addstr(n);
			pckh sym = None;
			for(int i = locals.count - 1; i >= 0; i--) {
				sym = current->findsym(ni, ps.parent, locals[i]);
				if(sym != None)
					break;
			}
			if(sym == None)
				sym = current->findsym(ni, ps.parent, 0);
			if(sym == None)
				sym = current->findsym(ni, This, 0);
			if(sym == None)
				sym = current->findsym(ni, Class, 0);
			if(sym == None) {
				sym = current->addsym(ni, ps.parent, I32, 0, 0, ps.level);
				status(message::NotFound1p2p, "symbol", n);
			}
			result = current->add(operation::Symbol, sym);
		}
		break;
	}
	postfix(result);
	return result;
}

static pckh multiplication() {
	auto result = unary();
	while((ps.p[0] == '*' || ps.p[0] == '/' || ps.p[0] == '%') && ps.p[1] != '=') {
		char s = ps.p[0];
		next();
		operation op;
		switch(s) {
		case '/': op = operation::Div; break;
		case '%': op = operation::DivPercent; break;
		default: op = operation::Mul; break;
		}
		result = current->add(op, result, unary());
	}
	return result;
}

static pckh addiction() {
	auto result = multiplication();
	while((ps.p[0] == '+' || ps.p[0] == '-') && ps.p[1] != '=') {
		char s = ps.p[0];
		next();
		operation op;
		switch(s) {
		case '+': op = operation::Add; break;
		case '-': op = operation::Sub; break;
		default: op = operation::Mul; break;
		}
		result = current->add(op, result, multiplication());
	}
	return result;
}

static pckh binary_cond() {
	auto result = addiction();
	while((ps.p[0] == '>' && ps.p[1] != '>')
		|| (ps.p[0] == '<' && ps.p[1] != '<')
		|| (ps.p[0] == '=' && ps.p[1] == '=')
		|| (ps.p[0] == '!' && ps.p[1] == '=')) {
		char t1 = *ps.p++;
		char t2 = 0;
		if(ps.p[0] == '=')
			t2 = *ps.p++;
		next(ps.p);
		operation op;
		switch(t1) {
		case '>':
			op = operation::Greater;
			if(t2 == '=')
				op = operation::GreaterEqual;
			break;
		case '<':
			op = operation::Lesser;
			if(t2 == '=')
				op = operation::LesserEqual;
			break;
		case '!': op = operation::NotEqual; break;
		default: op = operation::Equal; break;
		}
		result = current->add(op, result, addiction());
	}
	return result;
}

static pckh binary_and() {
	auto result = binary_cond();
	while(ps.p[0] == '&' && ps.p[1] != '&') {
		next(ps.p + 2);
		result = current->add(operation::And, result, binary_cond());
	}
	return result;
}

static pckh binary_xor() {
	auto result = binary_and();
	while(ps.p[0] == '^') {
		next(ps.p + 1);
		result = current->add(operation::Xor, result, binary_and());
	}
	return result;
}

static pckh binary_or() {
	auto result = binary_xor();
	while(ps.p[0] == '|' && ps.p[1] != '|') {
		next(ps.p + 1);
		result = current->add(operation::Or, result, binary_xor());
	}
	return result;
}

static pckh binary_shift() {
	auto result = binary_or();
	while((ps.p[0] == '>' && ps.p[1] == '>') || (ps.p[0] == '<' && ps.p[1] == '<')) {
		operation op;
		switch(ps.p[0]) {
		case '>': op = operation::ShiftRight; break;
		default: op = operation::ShiftLeft; break;
		}
		next(ps.p + 2);
		result = current->add(op, result, binary_or());
	}
	return result;
}

static pckh logical_and() {
	auto result = binary_shift();
	while(ps.p[0] == '&' && ps.p[1] == '&') {
		next(ps.p + 2);
		result = current->add(operation::LogicalAnd, result, binary_shift());
	}
	return result;
}

static pckh logical_or() {
	auto result = logical_and();
	while(ps.p[0] == '|' && ps.p[1] == '|') {
		next(ps.p + 2);
		result = current->add(operation::LogicalOr, result, logical_and());
	}
	return result;
}

static pckh expression() {
	auto result = logical_or();
	while(*ps.p == '?') {
		next(ps.p + 1);
		auto e1 = expression();
		skip(':');
		auto e2 = expression();
	}
	return result;
}

static pckh assigment() {
	auto result = expression();
	if((ps.p[0] == '=' && ps.p[1] != '=')
		|| ((ps.p[0] == '+' || ps.p[0] == '-' || ps.p[0] == '/' || ps.p[0] == '*') && ps.p[1] == '=')) {
		char t2 = *ps.p++;
		char t1 = '=';
		if(t2 == '=')
			t2 = 0;
		else
			t2 = *ps.p++;
		next(ps.p);
		result = current->add(operation::Assign, result, assigment());
	}
	return result;
}

static pckh initialize(pckh result) {
	if(*ps.p == '{') {
		next();
		while(true) {
			result = initialize(result);
			if(*ps.p == '}')
				break;
			skip(',');
			if(*ps.p == '}')
				break;
		}
		skip('}');
	} else {
		if(result)
			result = current->add(operation::Statement, result, expression());
		else
			result = expression();
	}
	return result;
}

static void join(pckh& result, pckh value) {
	if(result == None)
		result = value;
	else
		result = current->add(Statement, result, value);
}

static pckh statement() {
	pckh result = None;
	if(match(';')) {
		// Empthy statement
	} else if(*ps.p=='{') {
		next();
		ps.level++;
		auto push_count = locals.count;
		locals.add(ps.level);
		while(*ps.p) {
			if(match('}'))
				break;
			auto psp = ps.p;
			join(result, statement());
			if(psp == ps.p) {
				status(message::Expected1p, "keyword");
				break;
			}
		}
		locals.count = push_count;
	} else if(match("break")) {
		result = current->add(operation::Break, None);
		skip(';');
	} else if(match("continue")) {
		result = current->add(operation::Continue, None);
		skip(';');
	} else if(match("return")) {
		result = current->add(operation::Return, expression());
		skip(';');
	} else if(match("if")) {
		int e = 0;
		while(true) {
			skip('(');
			auto e = expression();
			skip(')');
			auto b = statement();
			if(!match("else")) {
				break;
			}
			if(match("if"))
				continue;
			// Else statement - if anything not match
			b = statement();
			break;
		}
	} else if(match("while")) {
		skip('(');
		auto e = expression();
		skip(')');
		auto b = statement();
		result = current->add(operation::While, e, b);
	} else if(match("do")) {
		auto b = statement();
		if(!match("while"))
			status(message::Expected1p, "while");
		skip('(');
		auto e = expression();
		skip(')');
		result = current->add(operation::Do, e, b);
	} else if(match("for")) {
		skip('(');
		// Инициализация цикла
		if(!declaration(ps.parent, 0, false, true, false, 0, ps.level)) {
			auto a = assigment();
		}
		if(*ps.p == ';')
			skip(';');
		auto e = expression();
		skip(';');
		auto t = statement();
		skip(')');
		// Генерируем блок тела цикла
		auto b = statement();
		join(b, t);
		current->add(operation::While, e, b);
	} else if(match("switch")) {
		skip('(');
		auto e = expression();
		skip(')');
		auto b = statement();
	} else if(match("case")) {
		auto e = expression();
		skip(':');
	} else if(match("default")) {
		skip(':');
	} else if(!declaration(ps.parent, 0, false, true, true, &result, ps.level)) {
		result = assigment();
		skip(';');
	}
	return result;
}

static void block_declaration() {
	while(declaration(This, 0, false, true));
}

static void block_function() {
	while(declaration(This, 0, true, false));
}

static void block_enums() {
	while(match("enum")) {
		auto type_name = identifier();
		skip('{');
		int num = 0;
		while(*ps.p) {
			unsigned sym = None;
			if(ischab(*ps.p)) {
				auto ix = ps.getindex();
				sym = current->addsym(current->addstr(identifier()), This, I32, ix, (1 << Const) | (1 << Static), 0);
				current->setast(sym, current->add(operation::Number, num));
			} else if(*ps.p == '=') {
				next();
				current->setast(sym, expression());
			} else if(*ps.p == ',') {
				num++;
				next();
			} else if(*ps.p == '}') {
				next();
				break;
			} else {
				status(message::ExpectedEnumeratorMember);
				return;
			}
		}
		skip(';');
	}
}

static void block_imports() {
	char temp[512]; stringbuilder sb(temp);
	auto count = 0;
	while(match("import")) {
		sb.clear();
		while(ps.p[0]) {
			auto pz = identifier();
			sb.add(pz);
			if(*ps.p == '.') {
				sb.add(".");
				next(ps.p + 1);
				continue;
			}
			auto idu = current->addstr(temp);
			if(match("as"))
				pz = identifier();
			auto id = current->addstr(pz);
			if(current->findsym(id, Class, 0) != None)
				status(message::AlreadyDefined, "module", pz);
			else
				current->addsym(id, Class, idu, 0, 0, 0);
			count++;
			skip(';');
			break;
		}
	}
}

void package::compile(const char* text) {
	parsestate push = ps;
	current = this;
	ps.pb = text;
	ps.p = text;
	ps.parent = This;
	block_imports();
	block_enums();
	block_declaration();
	block_function();
	ps = push;
}

bool package::compile(const char* url, const char* id) {
	char t1[512]; stringbuilder sb(t1);
	sb.clear(); addurl(sb, url, id, "c2");
	auto pf = loadt(t1);
	if(!pf)
		return false;
	package e;
	e.addsym(e.addstr("this"), Class, e.addstr(id), 0, 0, 0);
	e.compile(pf);
	delete pf;
	sb.clear(); addurl(sb, url, id, "ast");
	e.write(t1);
	return true;
}