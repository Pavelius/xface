#pragma once

struct section {
	static const unsigned minimal_count = 512;
	unsigned char*		data;
	unsigned			count;
	unsigned			count_maximum;
	constexpr section() : data(0), count(0), count_maximum(0) {}
	constexpr section(unsigned char* data) : data(data), count(0), count_maximum(minimal_count) {}
	~section();
	void				add(unsigned char v);
	constexpr const unsigned char* begin() const { return data; }
	constexpr const unsigned char* end() const { return data + count; }
	void				clear();
};
