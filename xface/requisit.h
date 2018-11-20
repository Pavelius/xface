#pragma once

unsigned					rmoptimal(unsigned need_count);
void*						rmreserve(void* data, unsigned new_size);

namespace compiler {
enum standart_s : unsigned {
	Number = 0xFFFFFF00, Text, Pointer,
	Null = 0xFFFFFFFF,
};
struct requisit;
struct classtype;
struct manager {
	template<class T>
	struct pref {
		T*			data;
		unsigned	count;
		unsigned	count_maximum;
		constexpr pref() : data(0), count(0), count_maximum(0) {}
		void reserve(unsigned count) {
			if(count >= count_maximum) {
				count_maximum = rmoptimal(count + 1);
				data = (T*)rmreserve(data, count_maximum * sizeof(T));
			}
		}
		void reserve() { reserve(count + 1); }
	};
	unsigned		add(unsigned parent, const char* name, unsigned type, unsigned count = 1, unsigned size = 0);
	unsigned		create(const char* id);
	unsigned		dereference(unsigned v) const;
	unsigned		get(const char* v);
	unsigned		getsize(unsigned v) const;
	bool			ispredefined(unsigned v) const { return v >= Number; }
	bool			isreference(unsigned v) const;
	unsigned		reference(unsigned v);
	void			write(const char* url);
private:
	pref<unsigned>	strings;
	pref<classtype>	classes;
	pref<requisit>	requisits;
	pref<char>		section_strings;
};
}