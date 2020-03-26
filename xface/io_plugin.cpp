#include "crt.h"
#include "io_plugin.h"

io::plugin* io::plugin::first;

static void add_filter(stringbuilder& sb, const char* name, const char* filter) {
	if(!filter)
		return;
	sb.add("%+1 (%2)", name, filter);
	sb.addsz();
	sb.add(filter);
	sb.addsz();
}

io::plugin::plugin() : name(0), fullname(0), filter(0) {
	seqlink(this);
}

io::plugin* io::plugin::find(const char* name) {
	for(auto p = first; p; p = p->next) {
		if(!p->name)
			continue;
		if(strcmp(p->name, name) == 0)
			return p;
	}
	return 0;
}

void io::plugin::getfilter(stringbuilder& sb) {
	for(auto p = first; p; p = p->next) {
		if(!p->name)
			continue;
		add_filter(sb, p->fullname, p->filter);
	}
}

io::strategy* io::strategy::first;

io::strategy::strategy(const char* id, const char* type) : id(id), type(type) {
	seqlink(this);
}

bool io::strategy::istrue(const char* value) {
	return value[0] == 't'
		&& value[1] == 'r'
		&& value[2] == 'u'
		&& value[3] == 'e'
		&& value[4] == 0;
}

io::strategy* io::strategy::find(const char* name) {
	for(auto n = first; n; n = n->next) {
		if(strcmp(n->id, name) == 0)
			return n;
	}
	return 0;
}

bool io::reader::node::operator==(const char* name) const {
	return strcmp(this->name, name) == 0;
}

int io::reader::node::get(int n) const {
	if(n >= sizeof(params) / sizeof(params[0]))
		return 0;
	auto p = this;
	while(p->parent) {
		if(p->params[n])
			return p->params[n];
		p = p->parent;
	}
	return 0;
}

void io::reader::node::set(int n, int v) {
	if(n >= sizeof(params) / sizeof(params[0]))
		return;
	params[n] = v;
}

int io::reader::node::getlevel() const {
	int result = 0;
	for(auto p = parent; p; p = p->parent)
		result++;
	return result;
}

bool io::read(const char* url, io::reader& e) {
	auto pp = io::plugin::find(szext(url));
	if(!pp)
		return false;
	const char* source = loadt(url);
	if(!source || source[0] == 0)
		return false;
	pp->read(source, e);
	delete source;
	return true;
}

bool io::read(const char* url, const char* root_name, void* param) {
	struct proxy : io::reader {
		const char*		root_name;
		void*			param;
		io::strategy*	st;
		void open(node& e) {
			switch(e.getlevel()) {
			case 0:
				if(e.name[0] != 0 && strcmp(e.name, root_name) != 0)
					e.skip = true;
				break;
			case 1:
				st = strategy::find(e.name);
				if(!st)
					e.skip = true;
				break;
			default:
				if(!st)
					e.skip = true;
				else
					st->open(e);
				break;
			}
		}

		void set(node& e, const char* value) {
			if(!st)
				e.skip = true;
			else
				st->set(e, value);
		}

		void close(node& e) {
			if(!st)
				e.skip = true;
			else
				st->close(e);
		}

		proxy() : root_name(0), param(0), st(0) {
		}

	};
	proxy reader_proxy;
	reader_proxy.root_name = root_name;
	reader_proxy.param = param;
	return read(url, reader_proxy);
}

bool io::write(const char* url, const char* root_name, void* param) {
	io::plugin* pp = plugin::find(szext(url));
	if(!pp)
		return false;
	io::file file(url, StreamWrite | StreamText);
	if(!file)
		return false;
	io::writer* pw = pp->write(file);
	if(!pw)
		return false;
	pw->open(root_name);
	for(auto ps = strategy::first; ps; ps = ps->next) {
		if(strcmp(ps->type, root_name) != 0)
			continue;
		pw->open(ps->id);
		ps->write(*pw, param);
		pw->close(ps->id);
	}
	pw->close(root_name);
	return true;
}

io::reader::node::node(io::type_s type) : parent(0), name(""), type(type), index(0), skip(false) {
	memset(params, 0, sizeof(params));
}

io::reader::node::node(io::reader::node& parent, const char* name, io::type_s type) : parent(&parent), name(name), type(type), index(0), skip(parent.skip) {
	memset(params, 0, sizeof(params));
}