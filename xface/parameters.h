#pragma once

class parameters {
	enum class kind : unsigned char {
		Text, Number,
	};
	struct element {
		const char*				id;
		long					value;
		kind					type;
	};
	char						buffer[4096];
	int							buffer_count;
	element						elements[32];
	int							count;
public:
	constexpr parameters() : buffer(), elements{}, buffer_count(0), count(0) {}
	element*					add(const char* id);
	void						add(const char* id, long value);
	void						add(const char* id, const char* value);
	const char*					addstr(const char* id);
	constexpr const element*	begin() const { return elements; }
	constexpr const element*	end() const { return elements + count; }
	constexpr unsigned			getcount() const { return count; }
	const element*				get(const char* id) const;
	int							getnum(const char* id) const;
	const char*					getstr(const char* id) const;
};