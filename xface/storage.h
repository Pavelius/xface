#pragma once

class storage {
	enum type_s : unsigned char {
		NoType,
		Text, TextPtr, Number, Bool,
	};
	type_s			type;
	short unsigned	size;
	void*			data;
public:
	constexpr storage() : type(NoType), size(0), data(0) {}
	constexpr storage(bool& value) : type(Bool), size(sizeof(value)), data(&value) {}
	template<class T> constexpr storage(T& value) : type(Number), size(sizeof(value)), data(&value) {}
	template<unsigned N> constexpr storage(char(&value)[N]) : type(Text), size(sizeof(value)/ sizeof(value[0])), data(value) {}
	explicit operator bool() const { return data != 0; }
	int				get() const;
	const char*		get(char* result, const char* result_end, bool force_to_result) const;
};
