#include "bsreq.h"
#include "crt.h"
#include "stringbuilder.h"

const int maximum_strings = 32;

static const char* read_string(const char* p, char* ps, const char* pe) {
	while(p[0] && p[0] != '\n' && p[0] != '\r') {
		if(ps < pe)
			*ps++ = *p;
		p++;
	}
	ps[0] = 0;
	return skipspcr(p);
}

static void apply_value(const char* id, const char* id_value, const char** requisits, const char** strings, const bsinf* sources) {
	for(auto pp = sources; *pp; pp++) {
		auto pfid = pp->type->find(id);
		if(!pfid)
			continue;
		for(unsigned i = 0; requisits[i]; i++) {
			if(!requisits[i])
				continue;
			auto pf = pp->type->find(requisits[i]);
			if(!pf)
				continue;
			auto string_value = szdup(strings[i]);
			for(unsigned j = 0; j < pp->source->getcount(); j++) {
				auto pv = pp->source->ptr(j);
				auto j_id = (const char*)pfid->get(pfid->ptr(pv));
				if(!j_id)
					continue;
				if(strcmp(j_id, id_value) != 0)
					continue;
				pf->set(pf->ptr(pv), (int)string_value);
			}
		}
	}
}

bool bsparse::read(const char* url, const char* id, const char** requisits) {
	if(!requisits || requisits[0] == 0)
		return false;
	auto p_alloc = (const char*)loadt(url);
	if(!p_alloc)
		return false;
	auto p = p_alloc;
	auto requisits_count = zlen(requisits);
	char name[128], value[8192];
	while(*p) {
		p = stringbuilder::readidn(p, name, zendof(name));
		if(p[0] != ':')
			break;
		p = skipsp(p + 1);
		p = read_string(p, value, zendof(value));
		const char* strings[maximum_strings] = {};
		auto count = 0;
		auto pt = value;
		strings[0] = pt;
		while(pt[0]) {
			if(pt[0] == '.' && requisits_count > 1) {
				pt[0] = 0;
				pt = (char*)skipsp(pt + 1);
				strings[requisits_count - 1] = pt;
				break;
			} else if(pt[0] == '|') {
				pt[0] = 0;
				pt = (char*)skipsp(pt + 1);
				if(count < maximum_strings)
					count++;
				strings[count] = pt;
				continue;
			}
			pt++;
		}
		if(metadata)
			apply_value(id, name, requisits, strings, metadata);
	}
	delete p_alloc;
	return true;
}

const bsreq* bsparse::getmeta(const char* name) {
	for(auto pp = metadata; *pp; pp++) {
		if(strcmp(pp->id, name) == 0)
			return pp->type;
	}
	return 0;
}

array* bsparse::getsource(const bsreq* type) {
	for(auto pp = metadata; *pp; pp++) {
		if(pp->type==type)
			return pp->source;
	}
	return 0;
}