//Copyright 2013 by Pavel Chistyakov
//
//   Licensed under the Apache License, Version 2.0 (the "License");
//   you may not use this file except in compliance with the License.
//   You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
//   Unless required by applicable law or agreed to in writing, software
//   distributed under the License is distributed on an "AS IS" BASIS,
//   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//   See the License for the specific language governing permissions and
//   limitations under the License.

#include "crt.h"
#include "io_plugin.h"

static const char* psidn(const char* p, char* ps, const char* pe) {
	while(*p && (ischa(*p) || isnum(*p) || *p == '_')) {
		if(ps < pe)
			*ps++ = *p++;
		else
			break;
	}
	*ps = 0;
	return p;
}

static const char* read_name(const char* p, serializer::node& n) {
	char temp[256];
	if(*p == '\"' || *p == '\'')
		p = psstr(p + 1, temp, *p);
	else if(ischa(*p))
		p = psidn(p, temp, temp + sizeof(temp) - 1);
	else
		return p;
	n.name = szdup(temp);
	return p;
}

static const char* read_object(const char* p, serializer::node& n, serializer::reader& e);

static const char* read_array(const char* p, serializer::node& pn, serializer::reader& e) {
	int index = 0;
	while(*p) {
		serializer::node n(pn);
		n.name = "element";
		n.index = index;
		p = read_object(p, n, e);
		if(*p == ',') {
			p = skipspcr(p + 1);
			index++;
			continue;
		}
		if(*p == ']')
			return p + 1;
		else
			return 0;
	}
	return p;
}

static const char* read_dictionary(const char* p, serializer::node& pn, serializer::reader& e) {
	int index = 0;
	while(*p) {
		serializer::node n(pn);
		n.index = index;
		p = skipspcr(read_name(p, n));
		if(*p == ':')
			p = skipspcr(p + 1);
		p = read_object(p, n, e);
		if(!p)
			return 0;
		if(*p == ',') {
			p = skipspcr(p + 1);
			index++;
			continue;
		}
		if(*p == '}')
			return p + 1;
		else {
			// error occurs
			return 0;
		}
	}
	return p;
}

static const char* read_string(const char* p, char end_symbol, serializer::node& n, serializer::reader& e) {
	char temp[256 * 64];
	p = psstr(p, temp, end_symbol);
	n.type = serializer::Text;
	if(!n.skip)
		e.set(n, temp);
	return p;
}

static const char* read_number(const char* p, serializer::node& n, serializer::reader& e) {
	char temp[64];
	auto ps = temp;
	auto pe = temp + sizeof(temp) - 2;
	ps[0] = 0;
	if(*p == '-') {
		if(ps < pe)
			*ps++ = *p;
		p++;
	}
	while(isnum(*p)) {
		if(ps < pe)
			*ps++ = *p;
		p++;
	}
	*ps = 0;
	n.type = serializer::Number;
	if(!n.skip)
		e.set(n, temp);
	return p;
}

static const char* read_object(const char* p, serializer::node& n, serializer::reader& e) {
	if(!p)
		return 0;
	else if(*p == '\"' || *p == '\'')
		p = read_string(p + 1, *p, n, e);
	else if(*p == '-' || isnum(*p))
		p = read_number(p, n, e);
	else if(equal(p, "false")) {
		p += 5;
		n.type = serializer::Number;
		if(!n.skip)
			e.set(n, "false");
	} else if(equal(p, "true")) {
		p += 4;
		n.type = serializer::Number;
		if(!n.skip)
			e.set(n, "true");
	} else if(*p == '{') {
		n.type = serializer::Struct;
		if(!n.skip)
			e.open(n);
		p = skipspcr(p + 1);
		if(*p == '}') {
			if(!n.skip)
				e.close(n);
			return skipspcr(p + 1);
		}
		p = read_dictionary(p, n, e);
		if(!n.skip)
			e.close(n);
	} else if(*p == '[') {
		n.type = serializer::Array;
		if(!n.skip)
			e.open(n);
		p = skipspcr(p + 1);
		if(*p == ']') {
			if(!n.skip)
				e.close(n);
			return skipspcr(p + 1);
		}
		p = read_array(p, n, e);
		if(!n.skip)
			e.close(n);
	}
	return skipspcr(p);
}

struct json_writer : serializer {
	io::stream& e;
	int	level;
	int	types[128];
	int	count[128];
	void write_name(const char* name) {
		if(count[level]) {
			e << ",";
			write_cr();
		}
		if(level && name && name[0] && types[level] != serializer::Array)
			e << "\"" << name << "\":";
		count[level]++;
	}
	void write_cr() {
		if(level == -1)
			return;
		e << "\r\n";
		for(int i = 0; i < level; i++)
			e << "  ";
	}
	void open(const char* name, serializer::type_s type) override {
		write_name(name);
		if(type == serializer::Array)
			e << "[";
		else
			e << "{";
		types[++level] = type;
		count[level] = 0;
		write_cr();
	}
	void set(const char* name, int value, serializer::type_s type) override {
		write_name(name);
		e << value;
	}
	void set(const char* name, const char* value, serializer::type_s type) override {
		if(!value)
			return;
		write_name(name);
		e.write("\"", 1);
		while(true) {
			switch(*value) {
			case 0:
				e.write("\"", 1);
				return;
			case 10:
				e.write("\\n", 2);
				value++;
				break;
			case 13:
				e.write("\\r", 2);
				value++;
				break;
			case '\"':
			case '\\':
			case '/':
			case '\b':
			case '\t':
				e.write("\\", 1);
				e.write(value, 1);
				value++;
				break;
			default:
				if(false) {
					e.write(value, 1);
					value++;
				} else {
					char temp[16];
					char* s1 = temp;
					unsigned u = szget(&value);
					szput(&s1, u, codepages::UTF8);
					e.write(temp, s1 - temp);
				}
				break;
			}
		}
	}
	void close(const char* name, serializer::type_s type) override {
		level--;
		write_cr();
		if(type == serializer::Array)
			e << "]";
		else
			e << "}";
		if(!name)
			return;
	}
	json_writer(io::stream& e) : e(e), level(0) {
		memset(types, 0, sizeof(types));
		memset(count, 0, sizeof(count));
	}
};

static struct json_reader_parser : public io::plugin {
	const char* read(const char* p, serializer::reader& e) override {
		serializer::node root;
		root.type = serializer::Struct;
		return read_object(p, root, e);
	}
	serializer* write(io::stream& e) override {
		return new json_writer(e);
	}
	json_reader_parser() {
		name = "json";
		fullname = "JSON data object";
		filter = "*.json";
	}
} reader_parser_instance;