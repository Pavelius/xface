#include "draw.h"
#include "main.h"
#include "markup.h"

DGINF(directioni) = {{"������������", DGREQ(name)}, {}};
DGINF(map_typei) = {{"������������", DGREQ(name)}, {}};
DGINF(resourcei) = {{"������������", DGREQ(name)}, {}};
DGINF(point) = {{"x", DGREQ(x)}, {"y", DGREQ(y)}, {}};
DGINF(object) = {{"����", DGREQ(frame)},
{"�������", DGINH(point)},
{"�������������� ���������", DGCHK(flags, ImageMirrorH)},
{"������������ ���������", DGCHK(flags, ImageMirrorV)},
{}};