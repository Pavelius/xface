#pragma once

struct storage {
	enum type_s : unsigned char {
		NoType,
		Text, TextPtr, Number, Bool,
	};
	type_s			type;
	short unsigned	size;
	void*			data;
	constexpr storage() : type(NoType), size(0), data(0) {}
	constexpr storage(type_s t, void* d, short unsigned s) : type(t), size(s), data(d) {}
	constexpr storage(bool& value) : type(Bool), size(sizeof(value)), data(&value) {}
	constexpr storage(const char*& value) : type(TextPtr), size(sizeof(value)), data(&value) {}
	constexpr storage(char* value, unsigned size) : type(Text), size(size), data(value) {}
	template<class T> constexpr storage(T& value) : type(Number), size(sizeof(value)), data(&value) {}
	template<unsigned N> constexpr storage(char(&value)[N]) : type(Text), size(sizeof(value)/ sizeof(value[0])), data(value) {}
	explicit operator bool() const { return data != 0; }
	bool operator==(const storage& e) const { return data==e.data && size==e.size && type==e.type; }
	int				get() const;
	const char*		get(char* result, const char* result_end) const;
	void			getf(char* result, const char* result_end) const;
	void			set(const char* result) const;
	void			set(int value) const;
};
