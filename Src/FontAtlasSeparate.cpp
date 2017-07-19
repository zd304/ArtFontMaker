#include "FontAtlasSeparate.h"

FontAtlasSeparate::FontAtlasSeparate(IDirect3DDevice9* device)
{
	mDevice = device;
}

FontAtlasSeparate::~FontAtlasSeparate()
{
	std::map<int, IDirect3DTexture9*>::iterator it;
	for (it = mTexs.begin(); it != mTexs.end(); ++it)
	{
		IDirect3DTexture9* tex = it->second;
		if (tex == NULL)
			continue;
		tex->Release();
	}
	mTexs.clear();
}

void FontAtlasSeparate::LoadFnt(const char* szCfg)
{
	std::ifstream file(szCfg, std::fstream::in);
	if (!file)
	{
		return;
	}

	std::string fntPath = szCfg;
	fntPath = fntPath.substr(0, fntPath.rfind('\\') + 1);
	IDirect3DTexture9* atlas = NULL;

	char p[1024];
	int index = 0;
	while (!file.eof())
	{
		memset(p, 0, 1024);
		file.getline(p, 1024);
		std::string s = p;
		if (index == 2)
		{
			int fileIndex = s.find("file=");
			s = s.substr(fileIndex + 6, s.length() - fileIndex - 7);
			s = fntPath + s;
			HRESULT hr = D3DXCreateTextureFromFileEx(Global::mDevice, s.c_str(),
				D3DX_FROM_FILE,
				D3DX_FROM_FILE,
				D3DX_DEFAULT,
				0,
				D3DFMT_A8R8G8B8,
				D3DPOOL_MANAGED,
				D3DX_FILTER_TRIANGLE,
				D3DX_FILTER_TRIANGLE,
				D3DCOLOR_RGBA(0, 0, 0, 255),
				NULL,
				NULL,
				&atlas);
			if (FAILED(hr))
			{
				break;
			}
		}
		else if (index > 3)
		{
			int x, y, width, height, id;

			std::string key;
			std::string value;
			bool afterEqual = false;
			bool lastSpace = true;
			for (size_t i = 0; i < s.length(); ++i)
			{
				char c = s[i];
				if (c == '=')
				{
					afterEqual = true;
					lastSpace = true;
				}
				else if (c == ' ' || c == '\t' || c == '\n' || c == '\r')
				{
					if (!lastSpace)
					{
						if (afterEqual)
						{
							if (key == "x")
							{
								x = atoi(value.c_str());
							}
							else if (key == "y")
							{
								y = atoi(value.c_str());
							}
							else if (key == "width")
							{
								width = atoi(value.c_str());
							}
							else if (key == "height")
							{
								height = atoi(value.c_str());
							}
							else if (key == "id")
							{
								id = atoi(value.c_str());
							}
							key.clear();
							value.clear();
							afterEqual = false;
						}
						else
						{
							key.clear();
							value.clear();
						}
					}
					lastSpace = true;
				}
				else
				{
					if (afterEqual)
					{
						value.push_back(c);
					}
					else
					{
						key.push_back(c);
					}
					lastSpace = false;
				}
			}

			IDirect3DTexture9* tex = NULL;
			HRESULT hr = D3DXCreateTexture(mDevice, width, height, D3DX_DEFAULT, 0, D3DFMT_A8B8G8R8, D3DPOOL_MANAGED, &tex);
			if (FAILED(hr))
			{
				continue;
			}

			D3DSURFACE_DESC desc;
			atlas->GetLevelDesc(0, &desc);

			D3DLOCKED_RECT lrc1;
			D3DLOCKED_RECT lrc2;
			tex->LockRect(0, &lrc1, 0, 0);
			atlas->LockRect(0, &lrc2, 0, 0);

			for (int u = 0; u < width; ++u)
			{
				for (int v = 0; v < height; ++v)
				{
					((unsigned int*)lrc1.pBits)[v * width + u] = ((unsigned int*)lrc2.pBits)[(y + v) * desc.Width + (u + x)];
				}
			}

			atlas->UnlockRect(0);
			tex->UnlockRect(0);
			mTexs[id] = tex;
		}
		++index;
	}
	file.close();
	file.clear();
}