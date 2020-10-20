#include "crt.h"
#include "io_plugin.h"

io::plugin* io::plugin::first;

void io::plugin::addfilter(stringbuilder& sb, const char* name, const char* filter) {
	if(!filter || !name || filter[0]==0 || name[0]==0)
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
		addfilter(sb, p->fullname, p->filter);
	}
	sb.addsz();
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

bool io::read(const char* url, serializer::reader& e) {
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
	struct proxy : serializer::reader {
		const char*		root_name;
		void*			param;
		io::strategy*	st;
		void open(serializer::node& e) {
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
		void set(serializer::node& e, const char* value) override {
			if(!st)
				e.skip = true;
			else
				st->set(e, value);
		}
		void close(serializer::node& e)  override {
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
	auto pw = pp->write(file);
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