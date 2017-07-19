#pragma once

#include "EditorDef.h"
#include "FileDialog.h"
#include "FontAtlasSeparate.h"

class ArtFont
{
public:
	ArtFont(int code, const char* szPath);
	~ArtFont();

	void SetTexture(const char* path);
public:
	int mCode;
	std::string mPath;
	LPDIRECT3DTEXTURE9 mTexture;
	unsigned int x;
	unsigned int y;
	unsigned int mWidth;
	unsigned int mHeight;
};

class ArtFontGroup
{
public:
	ArtFontGroup(int id);
	~ArtFontGroup();
public:
	int mId;
	std::vector<ArtFont*> mFontDatas;
};

class MainEditor
{
public:
	MainEditor(HWND hwnd, LPDIRECT3DDEVICE9 device);
	~MainEditor();
	void OnEnable();
	void OnGUI();
	void OnRender();
	void OnDestroy();

	void CreateTargetTexture();
private:
	void OnMenu();
	void OnFAS();
public:
	HWND mHwnd;
	float mWidth;
	float mHeight;
	LPDIRECT3DDEVICE9 mDevice;
	FileDialog mFdPng;
	FileDialog mFdCfg;
	FileDialog mFdSaveCfg;
	FileDialog mFdExport;
	FileDialog mFdLoadFnt;
	FileDialog mFdSaveTexs;
	std::vector<ArtFontGroup*> mArtFontGroups;
	int mTargetSize[2];
	int mDefaultSize;
	LPDIRECT3DTEXTURE9 mTargetTexture;
	LPD3DXMESH mMesh;
	bool mOpenByAbsPath;
	std::string mProjectDirectory;
	FontAtlasSeparate* mFAS;
	bool mShowFAS;
private:
	float angle;
	float tt[3];
	bool mShowTargetTex;
	std::string mCodeTxt;
};