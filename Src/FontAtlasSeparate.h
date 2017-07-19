#ifndef __FONT_ATLAS_SEPARATE_H__
#define __FONT_ATLAS_SEPARATE_H__

#include "EditorDef.h"

class FontAtlasSeparate
{
public:
	FontAtlasSeparate(IDirect3DDevice9* device);
	~FontAtlasSeparate();

	void LoadFnt(const char* szCfg);
public:
	IDirect3DDevice9* mDevice;
	std::map<int, IDirect3DTexture9*> mTexs;
};

#endif