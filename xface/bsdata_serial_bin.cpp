#include "archive.h"
#include "bsdata.h"

typedef adat<bsreq, 2048> requisitset;
typedef amap<const bsreq*, const bsreq*> typeset;

template<> void archive::set<bsreq>(bsreq& e) {
	set(e.id);
	set(e.offset);
	set(e.size);
	set(e.lenght);
	set(e.count);
	set(e.type);
	set(e.reference);
	set(e.subtype);
}

static const bsreq* getmeta(requisitset& requisits, typeset& types, const bsreq* fields) {
	if(fields->issimple())
		return fields;
	auto result = types.get(fields);
	if(result)
		return result;
	auto fields_copy = requisits.data + requisits.count;
	types.add(fields, fields_copy);
	auto fields_count = zlen(fields) + 1;
	memcpy(fields_copy, fields, fields_count * sizeof(bsreq));
	requisits.count += fields_count;
	for(int i = 0; i < fields_count - 1; i++)
		requisits.data[i].type = getmeta(requisits, types, requisits.data[i].type);
	return fields_copy;
}

void bsdata::write(const char* url, const array& source, const bsreq* fields) {
	requisitset requisits;
	typeset		types;
	io::file file(url, StreamWrite);
	if(!file)
		return;
	array references[] = {requisits};
	archive a(file, true, references);
	if(!a.signature("MTD"))
		return;
	if(!a.version(1, 0))
		return;
	getmeta(requisits, types, fields);
	a.set(requisits);
}