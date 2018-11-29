#include "crt.h"
#include "draw.h"
#include "win.h"

bool draw::dialog::open(const char* title, char* path, const char* filter, int filter_index, const char* ext) {
	OPENFILENAME	ofn;
	memset(&ofn, 0, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = GetActiveWindow();
	ofn.hInstance = GetModuleHandleA(0);
	ofn.lpstrFilter = filter;
	ofn.nFilterIndex = filter_index;
	ofn.lpstrFile = path;
	ofn.nMaxFile = 260;
	ofn.lpstrTitle = title;
	ofn.lpstrDefExt = ext;
	ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST;
	return GetOpenFileNameA(&ofn) != 0;
}

bool draw::dialog::save(const char* title, char* path, const char* filter, int filter_index) {
	OPENFILENAME ofn;
	memset(&ofn, 0, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = GetActiveWindow();
	ofn.hInstance = GetModuleHandleA(0);
	ofn.lpstrFilter = filter;
	ofn.nFilterIndex = filter_index;
	ofn.lpstrFile = path;
	ofn.nMaxFile = 260;
	ofn.lpstrTitle = title;
	ofn.Flags = OFN_EXPLORER | OFN_CREATEPROMPT | OFN_OVERWRITEPROMPT;
	auto result = GetSaveFileNameA(&ofn) != 0;
	if(result) {
		if(filter && !szext(path)) {
			auto p = filter;
			while(--ofn.nFilterIndex != 0) {
				p = zend(p) + 1;
				p = zend(p) + 1;
			}
			auto p1 = szext(zend(p) + 1);
			if(p1) {
				zcat(path, '.');
				auto ps = zend(path);
				while(ischa(*p1))
					*ps++ = *p1++;
				*ps++ = 0;
			}
		}
	}
	return result;
}

bool draw::dialog::color(struct color& result, struct color* custom) {
	static COLORREF custom_colors[16];
	CHOOSECOLOR	cc;	// common dialog box structure
	// Initialize CHOOSECOLOR
	struct color r = result;
	iswap(r.r, r.b);
	memset(&cc, 0, sizeof(cc));
	cc.lStructSize = sizeof(cc);
	cc.hwndOwner = GetActiveWindow();
	cc.rgbResult = *((int*)&r);
	cc.lpCustColors = custom_colors;
	cc.Flags = CC_FULLOPEN | CC_RGBINIT | CC_ANYCOLOR;
	if(ChooseColorA(&cc)) {
		*((int*)&result) = cc.rgbResult;
		iswap(result.r, result.b);
		return true;
	}
	return false;
}

static int CALLBACK BrowseFolderCallback(void* hwnd, unsigned uMsg, unsigned lParam, unsigned lpData) {
	if(uMsg == BFFM_INITIALIZED) {
		const char* path = reinterpret_cast<const char*>(lpData);
		SendMessageA(hwnd, BFFM_SETSELECTIONA, true, (int)path);
	}
	return 0;
}

bool draw::dialog::folder(const char* title, char* path) {
	BROWSEINFOA binf = {0};
	binf.hwndOwner = GetActiveWindow();
	binf.lParam = reinterpret_cast<unsigned>(path);
	binf.lpfn = BrowseFolderCallback;
	binf.ulFlags = BIF_RETURNONLYFSDIRS | BIF_USENEWUI;
	void* it = SHBrowseForFolderA(&binf);
	if(!it)
		return false;
	SHGetPathFromIDListA(it, path);
	szurlc(path);
	return true;
}