#include "crt.h"
#include "io_plugin.h"

struct csv_writer : serializer {
	io::stream& e;
	int	level;
	void open(const char* name, serializer::type_s type) override {
		level++;
	}
	void set(const char* name, int value, serializer::type_s type) override {
	}
	void set(const char* name, const char* value, serializer::type_s type) override {
	}
	void close(const char* name, serializer::type_s type) override {
		level--;
	}
	csv_writer(io::stream& e) : e(e), level(0) {
	}
};

static struct csv_reader_parser : public io::plugin {
	const char* read(const char* p, serializer::reader& e) override {
		serializer::node root;
		root.type = serializer::Struct;
		//return read_object(p, root, e);
		return 0;
	}
	serializer* write(io::stream& e) override {
		return new csv_writer(e);
	}
	csv_reader_parser() {
		name = "csv";
		fullname = "CSV data object";
		filter = "*.csv";
	}
} reader_parser_instance;