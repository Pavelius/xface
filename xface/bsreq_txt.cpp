#include "bsreq.h"
#include "crt.h"
#include "io_plugin.h"

using namespace io;

struct bsdata_writer_txt {
	writer&	ew;
	bsdata_writer_txt(writer& ew) : ew(ew) {}

	bool write_object(const void* pv, const bsreq* pf, const char* id, bool run) {
		if(run) {
			if(!write_object(pv, pf, id, false))
				return false;
			ew.open(id);
			while(*pf) {
				write_field(pv, pf, pf->id, true);
				pf++;
			}
			ew.close(id);
		} else {
			while(*pf) {
				if(write_field(pv, pf, pf->id, false))
					return true;
				pf++;
			}
			return false;
		}
		return true;
	}

	bool write_element(const void* pv, const bsreq* pf, int index, const char* id, bool skip_zero, bool run) {
		if(pf->is(KindNumber)) {
			auto value = pf->get(pf->ptr(pv, index));
			if(!value && skip_zero)
				return false;
			if(run)
				ew.set(id, value);
		} else if(pf->is(KindText)) {
			auto value = (const char*)pf->get(pf->ptr(pv, index));
			if(!value)
				value = "";
			if(!value[0] && skip_zero)
				return false;
			if(run)
				ew.set(id, value);
		} else if(pf->is(KindReference)) {
			auto value = pf->get(pf->ptr(pv, index));
			if(!value) {
				if(!skip_zero)
					ew.set(id, value);
				return false;
			}
			return write_field((void*)value, pf->type, id, run);
		} else if(pf->is(KindEnum)) {
			auto value = pf->get(pf->ptr(pv, index));
			if(!value && skip_zero)
				return false;
			return write_field(pf->source->ptr(value), pf->type, id, run);
		} else if(pf->is(KindCFlags)) {
			unsigned value = pf->get(pf->ptr(pv, index));
			if(!value && skip_zero)
				return false;
			auto pk = pf->type;
			if(!pk)
				return false;
			if(run) {
				auto pb = pf->source;
				ew.open(id, io::Array);
				for(unsigned i = 0; i < pb->getcount(); i++) {
					if((value & (1 << i)) == 0)
						continue;
					auto pv1 = pb->ptr(i);
					auto nm = (const char*)pk->get(pk->ptr(pv1));
					if(!nm)
						continue;
					ew.set("value", nm);
				}
				ew.close(id, io::Array);
			}
		} else
			return write_object(pf->ptr(pv, index), pf->type, id, run);
		return true;
	}

	bool write_field(const void* pv, const bsreq* pf, const char* id, bool run) {
		if(pf->count > 1) {
			if(run) {
				auto minimal = -1;
				for(unsigned i = 0; i < pf->count; i++) {
					if(write_element(pv, pf, i, "element", true, false))
						minimal = i;
				}
				if(minimal < 0)
					return false;
				ew.open(id, io::Array);
				for(auto i = 0; i <= minimal; i++)
					write_element(pv, pf, i, "element", false, true);
				ew.close(id, Array);
			} else {
				for(unsigned i = 0; i < pf->count; i++) {
					if(write_element(pv, pf, i, "element", true, false))
						return true;
				}
				return false;
			}
			return true;
		} else
			return write_element(pv, pf, 0, id, true, run);
	}

};

struct bsdata_reader_txt : reader {
	enum param_s : unsigned char { Meta, Object, Database };
	bsinf* findbase(const char* id) const {
		return 0;
	}
	void* findvalue(const char* value, const bsreq* pk, array* pd) const {
		void* pv = 0;
		if(pk->is(KindText))
			pv = (void*)pd->find(value, pk->offset);
		else if(pk->is(KindNumber)) {
			auto number = sz2num(value);
			auto size = pk->size;
			if(size > sizeof(number))
				size = sizeof(number);
			pv = (void*)pd->find(&number, pk->offset, pk->size);
		} else
			return 0;
		if(!pv) {
			pv = pd->add();
			auto kv = getvalue(value, pk);
			pk->set(pk->ptr(pv), kv);
		}
		return pv;
	}
	int getvalue(const char* value, const bsreq* pf) const {
		if(pf->is(KindText))
			return (int)szdup(value);
		else if(pf->is(KindNumber))
			return sz2num(value);
		else if(pf->is(KindReference) || pf->is(KindEnum) || pf->is(KindCFlags)) {
			auto pv = findvalue(value, pf->type, pf->source);
			if(pf->is(KindReference))
				return (int)pv;
			return pf->source->indexof(pv);
		}
		return 0;
	}
	int getindex(const node& e) const {
		if(e.parent && e.parent->type == io::Array)
			return e.index;
		return 0;
	}
	void setvalue(void* object, const bsreq* pf, int index, const char* value) {
		if(!object || !pf)
			return;
		if(pf->is(KindCFlags)) {
			object = pf->ptr(object, 0);
			auto i0 = getvalue(value, pf);
			auto i1 = pf->get(object);
			pf->set(object, i1 | (1<<i0));
		} else {
			object = pf->ptr(object, index);
			pf->set(object, getvalue(value, pf));
		}
	}
	void open(node& e) override {
		if(e.name[0] == 0)
			return;
		if(!e.parent)
			return;
		if(e == "element" && !e.parent->parent)
			return;
		auto pv = (void*)e.get(Object);
		if(!pv)
			return;
		auto pf = (const bsreq*)e.get(Meta);
		if(!pf)
			return;
		if(e == "element") {
		} else
			pf = pf->find(e.name);
		if(!pf)
			return;
		if(e.type == Array)
			e.set(Meta, (int)pf);
		else {
			e.set(Meta, (int)pf->type);
			e.set(Object, (int)pf->ptr(pv, getindex(e)));
		}
	}
	void set(node& e, const char* value) override {
		if(e == "typeid") {
			if(!e.parent)
				return;
			// Это идентификатор типа
			auto pd = findbase(value);
			e.parent->set(Database, (int)&pd->source);
			e.parent->set(Meta, (int)pd->meta);
			return;
		}
		const bsreq* pf = 0;
		auto index = 0;
		if(e == "element") {
			pf = (bsreq*)e.get(Meta);
			index = e.index;
		} else
			pf = ((bsreq*)e.get(Meta))->find(e.name);
		if(!pf)
			return;
		auto pv = (void*)e.get(Object);
		if(!pv) {
			auto pd = (array*)e.get(Database);
			if(!pd)
				return;
			// Допускаются только простые ключи
			pv = findvalue(value, pf, pd);
			if(e.parent)
				e.parent->set(Object, (int)pv);
		} else
			setvalue(pv, pf, index, value);
	};
};

//int bsdata::writetxt(const char* url) {
//	io::plugin* pp = plugin::find(szext(url));
//	if(!pp)
//		return 0;
//	io::file file(url, StreamWrite | StreamText);
//	if(!file)
//		return 0;
//	io::writer* pw = pp->write(file);
//	if(!pw)
//		return 0;
//	auto object_count = 0;
//	bsdata_writer_txt bw(*pw);
//	pw->open("records", Array);
//	for(auto& ps : bsmeta<bsdata>()) {
//		for(unsigned i = 0; i < ps.count; i++) {
//			auto p = ps.get(i);
//			pw->open("record");
//			pw->set("typeid", ps.id);
//			for(auto pf = ps.meta; *pf; pf++)
//				bw.write_field(p, pf, pf->id, true);
//			pw->close("record");
//			object_count++;
//		}
//	}
//	pw->close("records", Array);
//	return object_count;
//}

//int bsdata::readtxt(const char* url) {
//	io::plugin* pp = plugin::find(szext(url));
//	if(!pp)
//		return 0;
//	auto p = loadt(url);
//	bsdata_reader_txt ev;
//	pp->read(p, ev);
//	delete p;
//	return 1;
//}