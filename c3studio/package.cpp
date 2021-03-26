#include "package.h"
#include "io.h"

using namespace code;

symbol package::def_symbols[] = {
	{I32, Class, Class},
	{I16, Class, Class},
	{I8, Class, Class},
	{U32, Class, Class},
	{U16, Class, Class},
	{U8, Class, Class},
	{Void, Class, Class},
	{Bool, Class, Class},
	{None, Pointer, I8},
	{None, Pointer, Void},
	{True, This, Bool}, {False, This, Bool},
	{Pointer}, {Platform}, {Class},
	{SecStr}, {SecData}, {SecCode}, {SecBSS}, {SecLoc},
};
static_assert(sizeof(package::def_symbols) / sizeof(package::def_symbols[0]) == ((LastDef - FirstDef) + 1), "Invalid default symbols count");

const char* package::def_strings[] = {
	"int", "short", "char",
	"unsigned", "ushort", "uchar",
	"void", "bool",
	"char", "void",
	"true", "false",
	"*", "platform", "classes",
	"strings", "data", "code", "bss", "locale"
};
static_assert(sizeof(package::def_strings) / sizeof(package::def_strings[0]) == ((LastDef - FirstDef) + 1), "Invalid symbol strings count");

static const char* opr_strings[] = {
	"Number", "Literal", "Symbol",
	",", "=", "if", "while", "do",
	"+", "-", "*", "/", "%", "[]",
	">", ">=", "<", "<=", "==", "!=",
	"&&", "||",
	"&", "|", "^", "<<", ">>",
	"!", "&", "*", "-",
	"sizeof", "return",
};
static_assert(sizeof(opr_strings) / sizeof(opr_strings[0]) == Return + 1, "Invalid operator strings count");

const char*	urls::project;
const char* urls::library;

bool isnostrictorder(operation id) {
	switch(id) {
	case Add: case Mul:
		return true;
	default:
		return false;
	}
}

unsigned stringv::find(const char* v, unsigned len) const {
	if(v && len) {
		for(auto& e : package::def_strings) {
			if(strcmp(e, v) == 0)
				return (&e - package::def_strings) + FirstDef;
		}
		auto pe = end();
		auto s = *v;
		// Signature have first 4 bytes
		for(auto p = begin(); p < pe; p++) {
			if(*p != s)
				continue;
			unsigned n1 = pe - p - 1;
			if(n1 < len)
				return -1;
			if(memcmp(p + 1, v + 1, len) == 0)
				return p - begin();
		}
	}
	return -1;
}

unsigned stringv::find(const char* v) const {
	return find(v, zlen(v));
}

unsigned stringv::add(const char* v, unsigned len) {
	auto result = count;
	reserve(result + len + 1);
	memcpy(ptr(result), v, len + 1);
	setcount(result + len + 1);
	return result;
}

unsigned stringv::add(const char* v) {
	if(!v || v[0] == 0)
		return 0;
	auto c = zlen(v);
	auto i = find(v, c);
	if(i != 0xFFFFFFFF)
		return i;
	return add(v, c);
}

void package::getast(stringbuilder& sb, unsigned v) const {
	if(v == None)
		return;
	auto p = getast(v);
	if(!p)
		return;
	switch(p->type) {
	case SizeOf:
		sb.add(opr_strings[p->type]);
		sb.add("(");
		getast(sb, p->left);
		sb.add(")");
		break;
	case Return:
		sb.add(opr_strings[p->type]);
		sb.add(" ");
		getast(sb, p->left);
		break;
	case Add: case Sub:
	case Mul: case Div: case DivPercent:
	case And: case Or: case Xor:
	case ShiftLeft: case ShiftRight:
		getast(sb, p->left);
		sb.add(opr_strings[p->type]);
		getast(sb, p->right);
		break;
	case Greater: case GreaterEqual:
	case Lesser: case LesserEqual:
	case Equal:	case NotEqual:
	case LogicalOr: case LogicalAnd:
		getast(sb, p->left);
		sb.add(" ");
		sb.add(opr_strings[p->type]);
		sb.add(" ");
		getast(sb, p->right);
		break;
	case Symbol:
		sb.add(getsymstr(p->left));
		break;
	case Number:
		sb.add("%1i", p->left);
		break;
	case Literal:
		sb.add("\"");
		sb.add(getstr(p->left));
		sb.add("\"");
		break;
	case Call:
		getast(sb, p->left);
		sb.add("()");
		break;
	case Point:
		getast(sb, p->left);
		sb.add(".");
		sb.add(getstr(p->right));
		break;
	case Assign:
		getast(sb, p->left);
		sb.add("=");
		getast(sb, p->right);
		break;
	case Statement:
		getast(sb, p->left);
		sb.add("\n ");
		getast(sb, p->right);
		break;
	default: break;
	}
}

void package::getsym(stringbuilder& sb, pckh v) const {
	if(ispointer(v)) {
		getsym(sb, getresult(v));
		sb.add(def_strings[Pointer- FirstDef]);
	} else {
		auto p = getsymbol(v);
		if(!p)
			return;
		if(p->parent == Class) {
			//if(v < This) {
			//	sb.add(getstr(p->result));
			//	sb.add(" as ");
			//}
			sb.add(getstr(p->id));
		}
		else {
			getsym(sb, getresult(v));
			sb.adds(getstr(p->id));
			if(ismethod(v)) {
				sb.add("(");
				auto id = p - symbols.begin();
				auto pe = symbols.end();
				auto count = 0;
				for(auto p1 = p; p1 < pe; p1++) {
					if(p1->parent != id)
						continue;
					if(count > 0)
						sb.add(", ");
					getsym(sb, p1 - symbols.begin());
					count++;
				}
				sb.add(")");
				if(p->ast != None) {
					sb.add("\n ");
					getast(sb, p->ast);
				}
			} else {
				if(p->ast != None) {
					sb.add(" = ");
					getast(sb, p->ast);
				}
			}
		}
	}
}

void package::print(stringbuilder& sb) {
	for(auto& e : symbols) {
		if(e.parent == Pointer)
			continue;
		if(e.is(Parameter))
			continue;
		if(e.local)
			continue;
		getsym(sb, &e - symbols.begin());
		sb.add("\n");
	}
}

pckh package::findstr(const char* v) {
	return strings.find(v);
}

unsigned package::addstr(const char* v) {
	return strings.add(v);
}

unsigned package::find(operation type, unsigned left, unsigned right) const {
	for(auto& e : asts) {
		if(e.type == type && e.left == left && e.right == right)
			return &e - asts.begin();
	}
	return None;
}

unsigned package::add(operation type, unsigned left, unsigned right) {
	auto i = find(type, left, right);
	if(i != None)
		return i;
	if(isnostrictorder(type)) {
		i = find(type, right, left);
		if(i != None)
			return i;
	}
	auto p = asts.add();
	p->type = type;
	p->left = left;
	p->right = right;
	return p - asts.begin();
}

void package::clear() {
	strings.clear();
	symbols.clear();
	asts.clear();
}

static bool version(io::stream& file, const char* signature, int major, int minor, bool write_mode) {
	char temp[4];
	if(write_mode) {
		temp[0] = signature[0];
		temp[1] = signature[1];
		temp[2] = signature[2];
		temp[3] = 0;
		file.write(temp, sizeof(temp));
		temp[0] = '0' + major;
		temp[1] = '.';
		temp[2] = '0' + minor;
		file.write(temp, sizeof(temp));
	} else {
		file.read(temp, sizeof(temp));
		for(auto i = 0; i < 4; i++) {
			if(temp[i] != signature[i])
				return false;
		}
		file.read(temp, sizeof(temp));
		if(temp[0] - '0' != major)
			return false;
		if(temp[2] - '0' > minor)
			return false;
	}
	return true;
}

static void serialx(io::stream& file, array& e, bool write_mode) {
	if(write_mode) {
		file.write(&e.count, sizeof(e.count));
		if(e.count)
			file.write(e.begin(), e.count * e.getsize());
	} else {
		e.clear();
		unsigned v = 0;
		file.read(&v, sizeof(v));
		if(v) {
			e.reserve(v);
			e.setcount(v);
			file.read(e.begin(), e.count * e.getsize());
		}
	}
}

bool package::serial(const char* url, bool write_mode) {
	io::file file(url, write_mode ? StreamWrite : StreamRead);
	if(!file)
		return false;
	version(file, "PKG", 0, 1, write_mode);
	serialx(file, strings, write_mode);
	serialx(file, symbols, write_mode);
	serialx(file, asts, write_mode);
	return true;
}

pckh package::findsym(pckh id, pckh parent, unsigned level) const {
	for(auto& e : def_symbols) {
		if(e.id == id && e.parent == parent)
			return &e - def_symbols + FirstDef;
	}
	for(auto& e : symbols) {
		if(e.id == id && e.parent == parent)
			return &e - symbols.begin();
	}
	return None;
}

pckh package::addsym(pckh id, pckh parent, pckh result, unsigned index, unsigned flags, unsigned level) {
	auto v = findsym(id, parent, level);
	if(v == None) {
		auto p = symbols.add();
		p->id = id;
		p->parent = parent;
		p->result = result;
		p->index = index;
		p->flags = flags;
		p->ast = None;
		p->local = level;
		v = p - symbols.begin();
	}
	return v;
}

void package::setast(pckh sym, pckh ast) {
	auto p = getsymbol(sym);
	if(p)
		p->ast = ast;
}

unsigned package::getresult(pckh sym) const {
	auto p = getsymbol(sym);
	if(!p)
		return None;
	if(p->parent == Class)
		return Class;
	return p->result;
}

const char* package::getstr(pckh v) const {
	if(v >= FirstDef) {
		v -= FirstDef;
		if(v < sizeof(def_strings) / sizeof(def_strings[0]))
			return def_strings[v];
		return "";
	} else if(v < strings.getcount())
		return (const char*)strings.ptr(v);
	return "";
}

pckh package::getparent(pckh v) const {
	auto p = getsymbol(v);
	if(!p)
		return None;
	return p->parent;
}

const char* package::getsymstr(pckh sym) const {
	auto p = getsymbol(sym);
	if(!p)
		return "";
	return getstr(p->id);
}

const char* package::getsymurl(pckh sym) const {
	auto p = getsymbol(sym);
	if(!p || p->parent!=Class)
		return "";
	return getstr(p->result);
}

pckh package::getsymast(pckh sym) const {
	auto p = getsymbol(sym);
	if(!p)
		return None;
	return p->ast;
}

unsigned package::reference(pckh sym) {
	return addsym(None, Pointer, sym, 0, 0, 0);
}

symbol* package::getsymbol(pckh v) const {
	if(v < symbols.getcount())
		return const_cast<symbol*>(symbols.ptr(v));
	else if(v >= FirstDef && v <= (sizeof(def_strings) / sizeof(def_strings[0]) + FirstDef))
		return &def_symbols[v - FirstDef];
	return 0;
}

const ast* package::getast(pckh v) const {
	if(v >= asts.getcount())
		return 0;
	return asts.ptr(v);
}

unsigned package::getindex(pckh v) const {
	auto p = getsymbol(v);
	if(!p)
		return 0;
	return p->index;
}

pckh package::getleft(pckh v) const {
	auto p = getast(v);
	if(!p)
		return None;
	return p->left;
}

bool package::issymbol(pckh ast) const {
	auto p = getast(ast);
	if(!p)
		return false;
	return p->type == Symbol;
}

bool package::ispointer(pckh sym) const {
	auto p = getsymbol(sym);
	if(!p)
		return false;
	return p->parent == Pointer;
}

bool package::ismethod(pckh sym) const {
	auto p = getsymbol(sym);
	if(!p)
		return false;
	return p->is(Method);
}

void package::addurl(stringbuilder& sb, const char* url, const char* id, const char* ext) {
	char temp[512]; stringbuilder s1(temp);
	s1.add(id); s1.change((char)'.', (char)'//');
	sb.add(url);
	sb.add(s1);
	sb.add(".");
	sb.add(ext);
}

bool package::findurl(stringbuilder sb, const char* id, const char* ext) {
	sb.clear(); addurl(sb, urls::project, id, ext);
	if(!io::file::exist(sb)) {
		sb.clear(); addurl(sb, urls::library, id, ext);
		if(!io::file::exist(sb))
			return false;
	}
	return true;
}