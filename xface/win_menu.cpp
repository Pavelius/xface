#include "crt.h"
#include "draw_control.h"
#include "win.h"

using namespace draw;
using namespace draw::controls;

struct menu_builder : control::command::builder {

	control*	parent;
	void*		hMenu;
	int			count;
	
	menu_builder(control* parent) : hMenu(0), parent(parent), count(0) {
		hMenu = CreatePopupMenu();
	}

	~menu_builder() {
		DestroyMenu(hMenu);
		hMenu = 0;
	}

	static unsigned get_menu_data(void* handle, unsigned index) {
		MENUITEMINFO mi = {0};
		mi.cbSize = sizeof(mi);
		mi.fMask = MIIM_DATA;
		if(!GetMenuItemInfoA(handle, index - 1, 1, &mi))
			return 0;
		return mi.dwItemData;
	}

	const control::command* finish() const override {
		HWND hwnd = GetActiveWindow();
		if(!hwnd)
			return 0;
		POINT pt; pt.x = hot.mouse.x; pt.y = hot.mouse.y;
		ClientToScreen(hwnd, &pt);
		// show menu
		auto result = TrackPopupMenuEx(hMenu, TPM_RETURNCMD | TPM_NONOTIFY, pt.x, pt.y, hwnd, 0);
		if(result)
			result = get_menu_data(hMenu, result);
		hot.pressed = false;
		return (control::command*)result;
	}

	void add(const control::command& cmd) override {
		MENUITEMINFO mi = {0};
		mi.cbSize = sizeof(mi);
		mi.fMask = MIIM_STRING | MIIM_FTYPE | MIIM_STATE | MIIM_ID | MIIM_SUBMENU | MIIM_DATA;
		mi.fType = MFT_STRING;
		char temp[260]; temp[0] = 0;
		if(cmd.name)
			zcpy(temp, cmd.name, sizeof(temp) - 1);
		if(cmd.key) {
			zcat(temp, "\t");
			draw::key2str(zend(temp), cmd.key);
		}
		szupper(temp, 1);
		if((parent->*cmd.proc)(false))
			mi.fState = MFS_ENABLED;
		else
			mi.fState = MFS_DISABLED | MFS_GRAYED;
		mi.dwTypeData = temp;
		mi.wID = ++count;
		mi.dwItemData = (unsigned)&cmd;
		InsertMenuItemA(hMenu, mi.wID, 0, &mi);
	}

	void addseparator() override {
		AppendMenuA(hMenu, MF_SEPARATOR, -1, 0);
	}

};

control::command::builder* control::createmenu() {
	return new menu_builder(this);
}