#include "draw.h"
#include "main.h"
#include "markup.h"

DGINF(directioni) = {{"Наименование", DGREQ(name)}, {}};
DGINF(map_typei) = {{"Наименование", DGREQ(name)}, {}};
DGINF(resourcei) = {{"Наименование", DGREQ(name)}, {}};
DGINF(point) = {{"x", DGREQ(x)}, {"y", DGREQ(y)}, {}};
DGINF(object) = {{"Кадр", DGREQ(frame)},
{"Позиция", DGINH(point)},
{"Горизонтальное отражение", DGCHK(flags, ImageMirrorH)},
{"Вертикальное отражение", DGCHK(flags, ImageMirrorV)},
{}};