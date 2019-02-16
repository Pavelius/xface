#include "xface/draw_grid.h"
#include "main.h"

using namespace draw;

bsreq metadata_type[] = {
	BSREQ(metadata, id, text_type),
	BSREQ(metadata, type, metadata_type),
	BSREQ(metadata, size, number_type),
{}};
bsreq requisit_type[] = {
	BSREQ(requisit, id, text_type),
	BSREQ(requisit, type, metadata_type),
	BSREQ(requisit, parent, metadata_type),
	BSREQ(requisit, count, number_type),
	BSREQ(requisit, offset, number_type),
{}};

void view_metadata() {
	while(ismodal()) {

	}
}