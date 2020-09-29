#include "main.h"

using namespace code;

template<class T> const char* getnm(const void* object, stringbuilder& sb) {
	return ((T*)object)->name;
}
template<> const char* getnm<metadata>(const void* object, stringbuilder& sb) {
	auto p = (metadata*)object;
	p->getname(sb);
	return sb;
}
static bool allow_base_type(const void* object, int index) {
	auto& e = bsdata<metadata>::elements[index];
	if(e.isreference() || e.isarray() || e.ispredefined() || e.is(Static) || &e == object)
		return false;
	return true;
}
static bool allow_type(const void* object, int index) {
	auto& e = bsdata<metadata>::elements[index];
	if(e.isreference() || e.isarray() || e.is(Static))
		return false;
	return true;
}
static bool allow_static_member(const void* object) {
	auto p = (const requisit*)object;
	if(p->parent->is(Static))
		return false;
	return true;
}
static bool allow_count(const void* object) {
	auto p = (const requisit*)object;
	if(p->is(Method))
		return false;
	return true;
}
static bool allow_dimension_member(const void* object) {
	auto p = (const requisit*)object;
	if(p->is(Static))
		return false;
	return true;
}

DGINF(metatypei) = {
	{"������������", DGREQ(id)},
	{}};
DGINF(metadata) = {
	{"������������", DGREQ(id)},
	{"������� ���", DGREQ(type), {getnm<metadata>, allow_base_type}, {metadata::isbasetype}},
	{"����������� ������", DGCHK(flags, 1 << Static)},
	{}};
DGINF(requisit) = {
	{"������������", DGREQ(id)},
	{"���", DGREQ(type), {getnm<metadata>, allow_type}},
	{"����������", DGREQ(count), {}, {allow_count}},
	{"��������� ���� ������", DGCHK(flags, 1 << Public)},
	{"����������� ���� ������", DGCHK(flags, 1 << Static), {}, {allow_static_member}},
	{"���� �������� �������� ��� ������", DGCHK(flags, 1 << Dimension), {}, {allow_dimension_member}},
	{}};