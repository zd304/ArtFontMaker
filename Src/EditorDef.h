#pragma once

#include <Windows.h>
#include <d3d9.h>
#include <d3dx9.h>
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#include <io.h>
#include <imgui.h>
#include <tinyxml.h>
#include <string>
#include <vector>
#include <algorithm>
#include <fstream>
#include <direct.h>
#include <map>

namespace EditorUtility
{
	char* string_To_UTF8_C(char* cs);
	char* UTF8_To_string_C(char* cs);
	std::string string_To_UTF8(const std::string & str);
	std::string UTF8_To_string(const std::string& str);
	void StringReplace(std::string &strBase, const std::string& strSrc, const std::string& strDes);
	void PathChange(const std::string& input, std::string& output);

	void split(const std::string& str, const char* c, std::vector<std::string>& res);

	bool ToRelPath(const std::string& absPath, const std::string& refPath, std::string& output);
	bool ToAbsPath(const std::string& relPath, const std::string& refPath, std::string& output);
};

#define STUC(x, l) EditorUtility::string_To_UTF8_C(x, l)
#define STU(x) EditorUtility::string_To_UTF8(x).c_str()
#define UTS(x) EditorUtility::UTF8_To_string(x).c_str()
#define SAFE_DELETE(x) if (x) { delete x; x = NULL; }
#define SAFE_DELETE_ARRAY(x) if (x) { delete [] x; x = NULL; }

class Global
{
public:
	static int mItemID;
	static ImTextureID mFolderTexID;
	static ImTextureID mFileTexID;
	static ImTextureID mDiskTexID;
	static LPDIRECT3DDEVICE9 mDevice;
};