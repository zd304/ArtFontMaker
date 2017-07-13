#include "EditorDef.h"

int Global::mItemID = 0;
ImTextureID Global::mFolderTexID = NULL;
ImTextureID Global::mFileTexID = NULL;
ImTextureID Global::mDiskTexID = NULL;
LPDIRECT3DDEVICE9 Global::mDevice = NULL;

namespace EditorUtility
{
	char* string_To_UTF8_C(char* cs)
	{
		int len = strlen(cs);
		int nwLen = ::MultiByteToWideChar(CP_ACP, 0, cs, -1, NULL, 0);

		wchar_t * pwBuf = new wchar_t[nwLen + 1];//一定要加1，不然会出现尾巴  
		ZeroMemory(pwBuf, nwLen * 2 + 2);

		::MultiByteToWideChar(CP_ACP, 0, cs, len, pwBuf, nwLen);

		int nLen = ::WideCharToMultiByte(CP_UTF8, 0, pwBuf, -1, NULL, NULL, NULL, NULL);

		ZeroMemory(cs, nLen + 1);

		::WideCharToMultiByte(CP_UTF8, 0, pwBuf, nwLen, cs, nLen, NULL, NULL);

		delete[]pwBuf;
		pwBuf = NULL;

		return cs;
	}

	char* UTF8_To_string_C(char* cs)
	{
		int len = strlen(cs);
		int nwLen = MultiByteToWideChar(CP_UTF8, 0, cs, -1, NULL, 0);

		wchar_t * pwBuf = new wchar_t[nwLen + 1];//一定要加1，不然会出现尾巴 
		memset(pwBuf, 0, nwLen * 2 + 2);

		MultiByteToWideChar(CP_UTF8, 0, cs, len, pwBuf, nwLen);

		int nLen = WideCharToMultiByte(CP_ACP, 0, pwBuf, -1, NULL, NULL, NULL, NULL);

		memset(cs, 0, nLen + 1);

		WideCharToMultiByte(CP_ACP, 0, pwBuf, nwLen, cs, nLen, NULL, NULL);

		delete[]pwBuf;

		pwBuf = NULL;

		return cs;
	}

	std::string string_To_UTF8(const std::string & str)
	{
		int nwLen = ::MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, NULL, 0);

		wchar_t * pwBuf = new wchar_t[nwLen + 1];//一定要加1，不然会出现尾巴  
		ZeroMemory(pwBuf, nwLen * 2 + 2);

		::MultiByteToWideChar(CP_ACP, 0, str.c_str(), str.length(), pwBuf, nwLen);

		int nLen = ::WideCharToMultiByte(CP_UTF8, 0, pwBuf, -1, NULL, NULL, NULL, NULL);

		char * pBuf = new char[nLen + 1];
		ZeroMemory(pBuf, nLen + 1);

		::WideCharToMultiByte(CP_UTF8, 0, pwBuf, nwLen, pBuf, nLen, NULL, NULL);

		std::string retStr(pBuf);

		delete[]pwBuf;
		delete[]pBuf;

		pwBuf = NULL;
		pBuf = NULL;

		return retStr;
	}

	std::string char_To_UTF8(const char* szTxt)
	{
		std::string str = szTxt;

		int nwLen = ::MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, NULL, 0);

		wchar_t * pwBuf = new wchar_t[nwLen + 1];//一定要加1，不然会出现尾巴  
		ZeroMemory(pwBuf, nwLen * 2 + 2);

		::MultiByteToWideChar(CP_ACP, 0, str.c_str(), str.length(), pwBuf, nwLen);

		int nLen = ::WideCharToMultiByte(CP_UTF8, 0, pwBuf, -1, NULL, NULL, NULL, NULL);

		char * pBuf = new char[nLen + 1];
		ZeroMemory(pBuf, nLen + 1);

		::WideCharToMultiByte(CP_UTF8, 0, pwBuf, nwLen, pBuf, nLen, NULL, NULL);

		std::string retStr(pBuf);

		delete[]pwBuf;
		delete[]pBuf;

		pwBuf = NULL;
		pBuf = NULL;

		return retStr;
	}

	std::string UTF8_To_string(const std::string& str)
	{
		int nwLen = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, NULL, 0);

		wchar_t * pwBuf = new wchar_t[nwLen + 1];//一定要加1，不然会出现尾巴 
		memset(pwBuf, 0, nwLen * 2 + 2);

		MultiByteToWideChar(CP_UTF8, 0, str.c_str(), str.length(), pwBuf, nwLen);

		int nLen = WideCharToMultiByte(CP_ACP, 0, pwBuf, -1, NULL, NULL, NULL, NULL);

		char * pBuf = new char[nLen + 1];
		memset(pBuf, 0, nLen + 1);

		WideCharToMultiByte(CP_ACP, 0, pwBuf, nwLen, pBuf, nLen, NULL, NULL);

		std::string retStr = pBuf;

		delete[]pBuf;
		delete[]pwBuf;

		pBuf = NULL;
		pwBuf = NULL;

		return retStr;
	}

	void StringReplace(std::string &strBase, const std::string& strSrc, const std::string& strDes)
	{
		std::string::size_type pos = 0;
		std::string::size_type srcLen = strSrc.size();
		std::string::size_type desLen = strDes.size();
		pos = strBase.find(strSrc, pos);
		while ((pos != std::string::npos))
		{
			strBase.replace(pos, srcLen, strDes);
			pos = strBase.find(strSrc, (pos + desLen));
		}
	}

	void PathChange(const std::string& input, std::string& output)
	{
		int ddpos = input.find("..");
		if (ddpos < 0)
		{
			output = input;
			return;
		}

		std::string sRel = input;
		char szPath[MAX_PATH] = { 0 };
		GetCurrentDirectory(MAX_PATH, szPath);
		std::string sDir = szPath;
		sRel = sDir + "\\" + sRel;

		std::vector<std::string> inputpath, outputpath;
		std::string tmp;
		for (size_t i = 0; i < sRel.size(); i++)
		{
			if (sRel[i] != '\\')
			{
				tmp += sRel[i];
			}
			else
			{
				inputpath.push_back(tmp);
				tmp.clear();
			}
		}
		if (!tmp.empty())
		{
			inputpath.push_back(tmp);
			tmp.clear();
		}

		while (!inputpath.empty())
		{
			if (inputpath.back() != "..")
			{
				tmp = inputpath.back();
				outputpath.push_back(tmp);
				inputpath.pop_back();
			}
			else
			{
				inputpath.pop_back();
				inputpath.pop_back();
			}
		}

		while (!outputpath.empty())
		{
			inputpath.push_back(outputpath.back());
			outputpath.pop_back();
		}

		output.clear();
		for (size_t i = 0; i < inputpath.size(); ++i)
		{
			output.append(inputpath[i]).append("\\");
		}
	}
}