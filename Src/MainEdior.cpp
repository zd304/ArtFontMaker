#include "MainEditor.h"

static bool openCfgFlag = false;
static bool saveCfgFlag = false;
static bool exportFlag = false;
static bool loadFntFlag = false;
static bool saveTexsFlag = false;

ArtFont::ArtFont(int code, const char* szPath)
{
	mCode = code;
	mPath = szPath;
	mTexture = NULL;
	mWidth = 0;
	mHeight = 0;
	D3DXCreateTextureFromFileEx(Global::mDevice, szPath,
		D3DX_FROM_FILE,
		D3DX_FROM_FILE,
		D3DX_DEFAULT,
		0,
		D3DFMT_A8R8G8B8,
		D3DPOOL_MANAGED,
		D3DX_FILTER_TRIANGLE,
		D3DX_FILTER_TRIANGLE,
		D3DCOLOR_RGBA(0,0,0,255),
		NULL,
		NULL,
		&mTexture);
	if (mTexture == NULL)
		return;
	LPDIRECT3DSURFACE9 surface = NULL;
	mTexture->GetSurfaceLevel(0, &surface);
	D3DSURFACE_DESC desc;
	surface->GetDesc(&desc);
	mWidth = desc.Width;
	mHeight = desc.Height;

	x = 0;
	y = 0;
}

ArtFont::~ArtFont()
{
	if (mTexture)
	{
		mTexture->Release();
		mTexture = NULL;
	}
}

void ArtFont::SetTexture(const char* path)
{
	if (mTexture != NULL)
	{
		mTexture->Release();
		mTexture = NULL;
	}
	D3DXCreateTextureFromFileEx(Global::mDevice, path,
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
		&mTexture);
	if (mTexture == NULL)
		return;
	LPDIRECT3DSURFACE9 surface = NULL;
	mTexture->GetSurfaceLevel(0, &surface);
	D3DSURFACE_DESC desc;
	surface->GetDesc(&desc);
	mWidth = desc.Width;
	mHeight = desc.Height;
}

ArtFontGroup::ArtFontGroup(int id)
{
	mId = id;
}

ArtFontGroup::~ArtFontGroup()
{
	for (size_t i = 0; i < mFontDatas.size(); ++i)
	{
		ArtFont* item = mFontDatas[i];
		SAFE_DELETE(item);
	}
	mFontDatas.clear();
}

MainEditor::MainEditor(HWND hwnd, LPDIRECT3DDEVICE9 device)
{
	mHwnd = hwnd;
	mDevice = device;
	Global::mDevice = device;
	mArtFontGroups.clear();
	mTargetSize[0] = 128;
	mTargetSize[1] = 128;
	mDefaultSize = 32;
	mFdPng.ext = "png";
	mFdCfg.ext = "cfg";
	mFdSaveCfg.ext = "cfg";
	mFdExport.ext = "*";
	mFdLoadFnt.ext = "fnt";
	mFdSaveTexs.ext = "*";
	mFdPng.dlgName = "选择图片";
	mFdCfg.dlgName = "选择项目";
	mFdSaveCfg.dlgName = "保存项目";
	mFdExport.dlgName = "导出文件";
	mFdLoadFnt.dlgName = "拆解字体图集";
	mFdSaveTexs.dlgName = "保存字体拆分图片";
	mFdSaveCfg.mUsage = eFileDialogUsage_SaveFile;
	mFdExport.mUsage = eFileDialogUsage_OpenFolder;
	mFdSaveTexs.mUsage = eFileDialogUsage_OpenFolder;
	mTargetTexture = NULL;
	mMesh = NULL;

	tt[0] = 25;
	tt[1] = 0;
	tt[2] = 50;
	angle = 0.0f;
	mShowTargetTex = false;
	mShowFAS = true;

	char szPath[MAX_PATH] = { 0 };
	GetCurrentDirectory(MAX_PATH, szPath);
	std::string sCurPath = szPath;
	sCurPath = sCurPath.append("\\");

	mFdPng.SetDefaultDirectory(sCurPath);
	mFdCfg.SetDefaultDirectory(sCurPath);
	mFdSaveCfg.SetDefaultDirectory(sCurPath);
	mFdExport.SetDefaultDirectory(sCurPath);
	mFdLoadFnt.SetDefaultDirectory(sCurPath);
	mFdSaveTexs.SetDefaultDirectory(sCurPath);

	RECT rc;
	GetClientRect(hwnd, &rc);
	mWidth = (float)(rc.right - rc.left);
	mHeight = (float)(rc.bottom - rc.top);

	mOpenByAbsPath = false;
	mFAS = NULL;
}

MainEditor::~MainEditor()
{
	for (size_t i = 0; i < mArtFontGroups.size(); ++i)
	{
		ArtFontGroup* group = mArtFontGroups[i];
		if (group)
		{
			SAFE_DELETE(group);
		}
	}
	mArtFontGroups.clear();
	if (mTargetTexture)
	{
		mTargetTexture->Release();
		mTargetTexture = NULL;
	}
	SAFE_DELETE(mFAS);
}

struct VERTEX
{
	float    pos[3];
	float    uv[2];
};
#define D3DFVF_CUSTOMVERTEX (D3DFVF_XYZ|D3DFVF_DIFFUSE|D3DFVF_TEX1)

void MainEditor::OnEnable()
{
	ImGuiIO& io = ImGui::GetIO();
	io.Fonts->AddFontFromFileTTF("..\\Font\\msyh.ttf", 16.0f, NULL, io.Fonts->GetGlyphRangesChinese());

	LPDIRECT3DTEXTURE9 p = NULL;
	D3DXCreateTextureFromFileEx(Global::mDevice, "..\\Image\\disk.png",
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
		&p);
	Global::mDiskTexID = (ImTextureID)p;
	p = NULL;
	D3DXCreateTextureFromFileEx(Global::mDevice, "..\\Image\\folder.png",
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
		&p);
	Global::mFolderTexID = (ImTextureID)p;
	p = NULL;
	D3DXCreateTextureFromFileEx(Global::mDevice, "..\\Image\\png.png",
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
		&p);
	Global::mFileTexID = (ImTextureID)p;

	//D3DXCreateSphere(mDevice, 20.0f, 30, 30, &mSphere, NULL);
	//D3DXCreateBox(mDevice, 20, 20, 20, &mMesh, NULL);
	D3DXCreateMeshFVF(12, 24, D3DXMESH_MANAGED, D3DFVF_XYZ | D3DFVF_TEX1, mDevice, &mMesh);
	VERTEX vertices[] =
	{
		{ -5.0f, -5.0f, -5.0f, 0, 1 },
		{ -5.0f, 5.0f, -5.0f, 0, 0 },
		{ 5.0f, 5.0f, -5.0f, 1, 0 },
		{ 5.0f, -5.0f, -5.0f, 1, 1 },

		{ 5.0f, -5.0f, -5.0f, 0, 1 },
		{ 5.0f, 5.0f, -5.0f, 0, 0 },
		{ 5.0f, 5.0f, 5.0f, 1, 0 },
		{ 5.0f, -5.0f, 5.0f, 1, 1 },

		{ 5.0f, -5.0f, 5.0f, 0, 1 },
		{ 5.0f, 5.0f, 5.0f, 0, 0 },
		{ -5.0f, 5.0f, 5.0f, 1, 0 },
		{ -5.0f, -5.0f, 5.0f, 1, 1 },

		{ -5.0f, -5.0f, 5.0f, 0, 1 },
		{ -5.0f, 5.0f, 5.0f, 0, 0 },
		{ -5.0f, 5.0f, -5.0f, 1, 0 },
		{ -5.0f, -5.0f, -5.0f, 1, 1 },

		{ -5.0f, 5.0f, -5.0f, 0, 1 },
		{ -5.0f, 5.0f, 5.0f, 0, 0 },
		{ 5.0f, 5.0f, 5.0f, 1, 0 },
		{ 5.0f, 5.0f, -5.0f, 1, 1 },

		/*  {-5.0f, -5.0f, 0.0f, 0,1},
		{-5.0f, -5.0f, 10.0f, 0,0},
		{5.0f, -5.0f, 10.0f, 1,0},
		{5.0f, -5.0f, 0.0f, 1,1}*/


		{ -5.0f, -5.0f, -5.0f, 0, 1 },
		{ 5.0f, -5.0f, -5.0f, 0, 0 },
		{ 5.0f, -5.0f, 5.0f, 1, 0 },
		{ -5.0f, -5.0f, 5.0f, 1, 1 },  //底下的面。需要用逆时针。不然会被剔除;
	};
	void* pBuffer;
	mMesh->LockVertexBuffer(0, (void**)&pBuffer);
	memcpy(pBuffer, vertices, sizeof(vertices));
	mMesh->UnlockVertexBuffer();

	WORD* i = 0;
	mMesh->LockIndexBuffer(0, (void**)&i);

	// fill in the front face index data     
	i[0] = 0; i[1] = 1; i[2] = 2;
	i[3] = 0; i[4] = 2; i[5] = 3;

	// fill in the back face index data     
	i[6] = 4; i[7] = 5; i[8] = 6;
	i[9] = 4; i[10] = 6; i[11] = 7;

	// fill in the top face index data     
	i[12] = 8; i[13] = 9; i[14] = 10;
	i[15] = 8; i[16] = 10; i[17] = 11;

	// fill in the bottom face index data     
	i[18] = 12; i[19] = 13; i[20] = 14;
	i[21] = 12; i[22] = 14; i[23] = 15;

	// fill in the left face index data     
	i[24] = 16; i[25] = 17; i[26] = 18;
	i[27] = 16; i[28] = 18; i[29] = 19;

	// fill in the right face index data     
	i[30] = 20; i[31] = 21; i[32] = 22;
	i[33] = 20; i[34] = 22; i[35] = 23;

	mMesh->UnlockIndexBuffer();

	//std::string sPath = "F:\\IMGUI";
	//EditorUtility::PathChange(sPath, sPath);
}

void OpenCfg(MainEditor* editor, const char* cfg);
void SaveCfg(MainEditor* editor, const char* cfg);
void ExportFiles(MainEditor* editor, const char* path);
void LoadFnt(MainEditor* editor, const char* cfg);
void SaveTexs(MainEditor* editor, const char* cfg);

void MainEditor::OnGUI()
{
	Global::mItemID = 0;

	//ImGui::ShowTestWindow();

	bool b = true;
	ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
	ImGui::SetNextWindowSize(ImVec2(450, mHeight));
	ImGui::Begin(STU("图集制作"), &b, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoTitleBar);

	OnMenu();

	ImGui::Text(STU("图片大小"));
	ImGui::SameLine();
	ImGui::DragInt2("##targetSize", mTargetSize, 2, 2, 2048, STU("%.0f 像素"));
	ImGui::SameLine();
	if (ImGui::Button(STU("生成贴图")))
	{
		CreateTargetTexture();
	}
	ImGui::Text(STU("字体默认大小"));
	ImGui::SameLine();
	ImGui::InputInt("##defaultFontSize", &mDefaultSize);
	if (mDefaultSize <= 0)
	{
		mDefaultSize = 1;
	}

	bool addArtFont = false;
	static ArtFontGroup* curFontGroup = NULL;

	std::vector<ArtFontGroup*>::iterator it = mArtFontGroups.begin();
	while (it != mArtFontGroups.end())
	{
		ArtFontGroup* group = *it;
		std::vector<ArtFontGroup*>::iterator curIt = it;
		++it;
		char szTxt[128];
		memset(szTxt, 0, 128);
		sprintf_s(szTxt, "[组] %d { 字数: %d }", group->mId, group->mFontDatas.size());
		if (ImGui::CollapsingHeader(STU(szTxt), ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::PushID(Global::mItemID++);
			if (ImGui::BeginPopupContextItem(""))
			{
				if (ImGui::Selectable(STU("删除字体组")))
				{
					SAFE_DELETE(group);
					mArtFontGroups.erase(curIt);
					ImGui::EndPopup();
					ImGui::PopID();
					break;
				}
				if (ImGui::Selectable(STU("添加艺术字")))
				{
					addArtFont = true;
					curFontGroup = group;
				}
				ImGui::EndPopup();
			}
			ImGui::PopID();

			ImGui::Text(STU("分组ID"));
			ImGui::SameLine();
			ImGui::PushID(Global::mItemID++);
			ImGui::InputInt("", &group->mId);
			ImGui::PopID();
			ImGui::SameLine(0, 10);
			ImGui::PushID(Global::mItemID++);
			if (ImGui::Button(STU("添加艺术字")))
			{
				addArtFont = true;
				curFontGroup = group;
			}
			ImGui::PopID();

			ImGui::Columns(3, NULL, false);
			ImGui::SetColumnOffset(1, 20);
			ImGui::SetColumnOffset(2, 100);
			std::vector<ArtFont*>::iterator itf;
			for (itf = group->mFontDatas.begin();
				itf != group->mFontDatas.end(); ++itf)
			{
				ImGui::Separator();
				ArtFont* artFont = *itf;
				ImGui::NextColumn();
				ImGui::Image(artFont->mTexture, ImVec2(64, 64));
				ImGui::NextColumn();
				ImGui::Text(STU("文字编码"));
				ImGui::SameLine();
				ImGui::PushID(Global::mItemID++);
				ImGui::InputInt("", &artFont->mCode);
				if (artFont->mCode < 0)
					artFont->mCode = 0;
				ImGui::PopID();
				ImGui::PushID(Global::mItemID++);
				if (ImGui::Button(STU("更换贴图"), ImVec2(60, 24)))
				{
					mFdPng.Open();
				}
				if (mFdPng.DoModal())
				{
					std::string path = mFdPng.directory;
					path += mFdPng.fileName;
					artFont->SetTexture(path.c_str());
				}
				ImGui::PopID();
				ImGui::SameLine();
				ImGui::PushID(Global::mItemID++);
				if (ImGui::Button(STU("删除"), ImVec2(60, 24)))
				{
					SAFE_DELETE(artFont);
					group->mFontDatas.erase(itf);
					ImGui::PopID();
					ImGui::NextColumn();
					break;
				}
				ImGui::PopID();
				ImGui::SameLine();
				ImGui::Text(STU("尺寸: %d × %d"), artFont->mWidth, artFont->mHeight);
				ImGui::NextColumn();
			}
			ImGui::Columns(1);
		}
		else
		{
			ImGui::PushID(Global::mItemID++);
			if (ImGui::BeginPopupContextItem(""))
			{
				if (ImGui::Selectable(STU("删除字体组")))
				{
					SAFE_DELETE(group);
					mArtFontGroups.erase(curIt);
					ImGui::EndPopup();
					ImGui::PopID();
					break;
				}
				if (ImGui::Selectable(STU("添加艺术字")))
				{
					addArtFont = true;
					curFontGroup = group;
				}
				ImGui::EndPopup();
			}
			ImGui::PopID();
		}
	}

	if (ImGui::Button(STU("添加字符分组"), ImVec2(-1, 32)))
	{
		ImGui::OpenPopup("##addArtFontGroup");
	}
	static int artFontID = 0;
	if (ImGui::BeginPopupModal("##addArtFontGroup", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::Text(STU("艺术字组ID"));
		ImGui::SameLine();
		ImGui::DragInt("##artFontID", &artFontID);
		if (artFontID < 0) artFontID = 0;
		if (ImGui::Button(STU("确定")))
		{
			bool findExist = false;
			for (size_t i = 0; i < mArtFontGroups.size(); ++i)
			{
				if (mArtFontGroups[i]->mId == artFontID)
				{
					findExist = true;
					break;
				}
			}
			if (!findExist)
				mArtFontGroups.push_back(new ArtFontGroup(artFontID));
			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
		if (ImGui::Button(STU("取消")))
		{
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}

	static LPDIRECT3DTEXTURE9 fontTex = NULL;
	static std::string fontTexPath = "";
	static bool delFontTex = false;
	static int code = 0;
	if (addArtFont)
	{
		ImGui::OpenPopup("##addArtFont");
	}
	if (delFontTex)
	{
		delFontTex = false;
		if (fontTex != NULL)
		{
			fontTex->Release();
			fontTex = NULL;
		}
		fontTexPath = "";
		code = 0;
		curFontGroup = NULL;
	}
	if (ImGui::BeginPopupModal("##addArtFont", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::Image(fontTex, ImVec2(128, 128));
		if (ImGui::Button(STU("选择文字图片"), ImVec2(128, 24)))
		{
			mFdPng.Open();
		}
		if (mFdPng.DoModal())
		{
			fontTexPath = mFdPng.directory;
			fontTexPath += mFdPng.fileName;
			D3DXCreateTextureFromFileEx(Global::mDevice, fontTexPath.c_str(),
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
				&fontTex);
		}
		ImGui::Text(STU("文字编码"));
		ImGui::SameLine();
		ImGui::InputInt("##fontCode", &code);
		ImGui::Separator();

		if (ImGui::Button(STU("确定")))
		{
			delFontTex = true;
			ArtFont* artFont = new ArtFont(code, fontTexPath.c_str());
			curFontGroup->mFontDatas.push_back(artFont);
			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
		if (ImGui::Button(STU("取消")))
		{
			delFontTex = true;
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}	

	if (openCfgFlag)
	{
		openCfgFlag = false;
		mFdCfg.Open();
	}
	if (mFdCfg.DoModal())
	{
		std::string cfg = mFdCfg.directory;
		mProjectDirectory = mFdCfg.directory;
		cfg += mFdCfg.fileName;
		OpenCfg(this, cfg.c_str());
		memset(mFdSaveCfg.directory, 0, MAX_PATH);
		memset(mFdSaveCfg.cur_directory, 0, MAX_PATH);
		memcpy(mFdSaveCfg.directory, mFdCfg.directory, MAX_PATH);
		memcpy(mFdSaveCfg.cur_directory, mFdCfg.cur_directory, MAX_PATH);
	}

	if (saveCfgFlag)
	{
		saveCfgFlag = false;
		mFdSaveCfg.Open();
	}
	if (mFdSaveCfg.DoModal())
	{
		std::string cfg = mFdSaveCfg.directory;
		cfg += mFdSaveCfg.fileName;
		SaveCfg(this, cfg.c_str());
	}

	if (exportFlag)
	{
		exportFlag = false;
		mFdExport.Open();
	}
	if (mFdExport.DoModal())
	{
		std::string path = mFdExport.directory;
		path += mFdExport.fileName;
		ExportFiles(this, path.c_str());
	}

	if (loadFntFlag)
	{
		loadFntFlag = false;
		mFdLoadFnt.Open();
	}
	if (mFdLoadFnt.DoModal())
	{
		std::string path = mFdLoadFnt.directory;
		path += mFdLoadFnt.fileName;
		LoadFnt(this, path.c_str());
	}

	if (saveTexsFlag)
	{
		saveTexsFlag = false;
		mFdSaveTexs.Open();
	}
	if (mFdSaveTexs.DoModal())
	{
		std::string path = mFdSaveTexs.directory;
		path += mFdSaveTexs.fileName;
		SaveTexs(this, path.c_str());
	}

	ImGui::End();

	if (mTargetTexture && mShowTargetTex)
	{
		ImGui::Begin(STU("图片预览"), &mShowTargetTex, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse);

		float tex_w = (float)mTargetSize[0];
		float tex_h = (float)mTargetSize[1];
		ImVec2 tex_screen_pos = ImGui::GetCursorScreenPos();
		ImGui::Image(mTargetTexture, ImVec2((float)mTargetSize[0], (float)mTargetSize[1]));
		if (ImGui::IsItemHovered())
		{
			ImGui::BeginTooltip();
			float focus_sz = 32.0f;
			float focus_x = ImGui::GetMousePos().x - tex_screen_pos.x - focus_sz * 0.5f; if (focus_x < 0.0f) focus_x = 0.0f; else if (focus_x > tex_w - focus_sz) focus_x = tex_w - focus_sz;
			float focus_y = ImGui::GetMousePos().y - tex_screen_pos.y - focus_sz * 0.5f; if (focus_y < 0.0f) focus_y = 0.0f; else if (focus_y > tex_h - focus_sz) focus_y = tex_h - focus_sz;
			ImGui::Text("Min: (%.2f, %.2f)", focus_x, focus_y);
			ImGui::Text("Max: (%.2f, %.2f)", focus_x + focus_sz, focus_y + focus_sz);
			ImVec2 uv0 = ImVec2((focus_x) / tex_w, (focus_y) / tex_h);
			ImVec2 uv1 = ImVec2((focus_x + focus_sz) / tex_w, (focus_y + focus_sz) / tex_h);
			ImGui::Image(mTargetTexture, ImVec2(128, 128), uv0, uv1, ImColor(255, 255, 255, 255), ImColor(255, 255, 255, 128));
			ImGui::EndTooltip();
		}
		ImGui::End();
	}

	OnFAS();

	Global::mItemID = 0;
}

void MainEditor::OnRender()
{
	if (mMesh && mShowTargetTex)
	{
		mDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, false);
		mDevice->SetRenderState(D3DRS_ZENABLE, true);
		mDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, true);
		mDevice->SetRenderState(D3DRS_ZWRITEENABLE, true);
		mDevice->SetRenderState(D3DRS_LIGHTING, false);
		mDevice->SetRenderState(D3DRS_ALPHATESTENABLE, true);
		mDevice->SetRenderState(D3DRS_ALPHAREF, 0x77);
		mDevice->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATER);

		D3DXMATRIX proj;
		D3DXMatrixPerspectiveFovLH(&proj, 45.0f,
			1280.0f / 800.0f, 0.1f, 1000.0f);
		mDevice->SetTransform(D3DTS_PROJECTION, &proj);

		D3DXMATRIX view;
		D3DXVECTOR3 position(0.0f, 0.0f, -1.0f);
		D3DXVECTOR3 target(0.0f, 0.0f, 0.0f);
		D3DXVECTOR3 up(0.0f, 1.0f, 0.0f);
		D3DXMatrixLookAtLH(&view, &position, &target, &up);
		mDevice->SetTransform(D3DTS_VIEW, &view);

		D3DXMATRIX mat;
		D3DXMatrixTranslation(&mat, tt[0], tt[1], tt[2]);
		D3DXMATRIX matRot;
		D3DXMatrixRotationY(&matRot, angle);
		angle += 0.0001f;
		if (angle > 3.14f)
			angle = 0.0f;

		D3DXMatrixMultiply(&mat, &matRot, &mat);

		mDevice->SetTexture(0, mTargetTexture);
		mDevice->SetTransform(D3DTS_WORLD, &mat);

		mMesh->DrawSubset(0);
	}
}

struct ExportItem
{
	int mId;
	int mGroupID;
	int mCode;
	ArtFont* mFontInfo;
};

void ExportFiles(MainEditor* editor, const char* path)
{
	editor->CreateTargetTexture();
	if (!editor->mTargetTexture)
	{
		return;
	}
	std::string destFile = path;
	destFile += "\\font_0.png";
	HRESULT hr = D3DXSaveTextureToFileA(destFile.c_str(), D3DXIFF_PNG, editor->mTargetTexture, NULL);
	if (hr != S_OK)
	{
		return;
	}
	std::vector<ExportItem*> vecExportItems;
	int index = 1000;
	for (size_t i = 0; i < editor->mArtFontGroups.size(); ++i)
	{
		ArtFontGroup* group = editor->mArtFontGroups[i];
		for (size_t j = 0; j < group->mFontDatas.size(); ++j)
		{
			ArtFont* artFont = group->mFontDatas[j];
			ExportItem* item = new ExportItem();
			item->mId = index++;
			item->mCode = artFont->mCode;
			item->mGroupID = group->mId;
			item->mFontInfo = artFont;
			vecExportItems.push_back(item);
		}
	}
	std::vector<char> sCharVec;
	char h1 = -1, h2 = -2;
	sCharVec.push_back(h1);
	sCharVec.push_back(h2);
	std::wstring whead = L"新编码\t组序号\t原编码\r\n";
	for (size_t i = 0; i < whead.length(); ++i)
	{
		unsigned short code = (unsigned short)whead[i];
		unsigned short code1 = (code & 255);
		unsigned short code2 = (code >> 8);
		sCharVec.push_back((char)code1);
		sCharVec.push_back((char)code2);
	}

	int count = vecExportItems.size();
	for (size_t i = 0; i < vecExportItems.size(); ++i)
	{
		ExportItem* item = vecExportItems[i];

		wchar_t szTemp[32];
		memset(szTemp, 0, sizeof(wchar_t) * 32);
		swprintf_s(szTemp, L"%d", item->mId);
		std::wstring line = szTemp;
		line.append(L"\t");
		memset(szTemp, 0, sizeof(wchar_t) * 32);
		swprintf_s(szTemp, L"%d", item->mGroupID);
		line.append(szTemp).append(L"\t");
		memset(szTemp, 0, sizeof(wchar_t) * 32);
		swprintf_s(szTemp, L"%d", item->mCode);
		line.append(szTemp).append(L"\r\n");

		for (size_t i = 0; i < line.length(); ++i)
		{
			unsigned short code = (unsigned short)line[i];
			unsigned short code1 = (code & 255);
			unsigned short code2 = (code >> 8);
			sCharVec.push_back((char)code1);
			sCharVec.push_back((char)code2);
		}
	}

	destFile = path;
	destFile += "\\font_map.txt";
	std::ofstream out(destFile.c_str(), std::ios::binary);
	if (!out.is_open())
	{
		return;
	}
	out.write(&sCharVec[0], sCharVec.size());
	out.close();

	std::string fntTxt = "info face=\"Arial\" size=32 bold=0 italic=0 charset=\"\" unicode=1 stretchH=100 smooth=1 aa=1 padding=0,0,0,0 spacing=0,0 outline=0\r\ncommon lineHeight=%d base=26 scaleW=%d scaleH=%d pages=1 packed=0 alphaChnl=1 redChnl=0 greenChnl=0 blueChnl=0\r\npage id=0 file=\"%s\"\r\nchars count=%d\r\n";
	char fileTxt[20480];
	memset(fileTxt, 0, 20480);
	sprintf_s(fileTxt, fntTxt.c_str(), editor->mDefaultSize, editor->mTargetSize[0], editor->mTargetSize[1], "font_0.png", count);
	fntTxt = fileTxt;
	const char* charFmt = "char id=%d   x=%d    y=%d     width=%d    height=%d    xoffset=0     yoffset=0     xadvance=%d    page=0  chnl=15\r\n";
	for (size_t i = 0; i < vecExportItems.size(); ++i)
	{
		ExportItem* item = vecExportItems[i];
		ArtFont* artFont = item->mFontInfo;
		memset(fileTxt, 0, 20480);
		sprintf_s(fileTxt, charFmt, item->mId, artFont->x, artFont->y, artFont->mWidth, artFont->mHeight, artFont->mWidth);
		fntTxt = fntTxt.append(fileTxt);
	}

	destFile = path;
	destFile += "\\font_0.fnt";
	out = std::ofstream(destFile.c_str(), std::ios::binary);
	if (!out.is_open())
	{
		return;
	}
	out.write(fntTxt.c_str(), fntTxt.length());
	out.close();

	for (size_t i = 0; i < vecExportItems.size(); ++i)
	{
		ExportItem* item = vecExportItems[i];
		SAFE_DELETE(item);
	}
	vecExportItems.clear();
}

void SaveCfg(MainEditor* editor, const char* cfg)
{
	TiXmlDocument doc;
	TiXmlElement* pRootEle = new TiXmlElement("config");
	doc.LinkEndChild(pRootEle);

	TiXmlElement* target_size = new TiXmlElement("target_size");
	target_size->SetAttribute("x", editor->mTargetSize[0]);
	target_size->SetAttribute("y", editor->mTargetSize[1]);
	pRootEle->LinkEndChild(target_size);

	TiXmlElement* default_size = new TiXmlElement("default_size");
	default_size->SetAttribute("size", editor->mDefaultSize);
	pRootEle->LinkEndChild(default_size);

	TiXmlElement* fonts_ele = new TiXmlElement("fonts");

	for (size_t i = 0; i < editor->mArtFontGroups.size(); ++i)
	{
		ArtFontGroup* group = editor->mArtFontGroups[i];

		TiXmlElement* group_ele = new TiXmlElement("group");
		group_ele->SetAttribute("id", group->mId);

		for (size_t j = 0; j < group->mFontDatas.size(); ++j)
		{
			ArtFont* artFont = group->mFontDatas[j];

			TiXmlElement* font_ele = new TiXmlElement("font");
			font_ele->SetAttribute("code", artFont->mCode);
			std::string relPath;
			EditorUtility::ToRelPath(artFont->mPath, editor->mProjectDirectory, relPath);
			font_ele->SetAttribute("path", relPath.c_str());
			group_ele->LinkEndChild(font_ele);
		}

		fonts_ele->LinkEndChild(group_ele);
	}
	pRootEle->LinkEndChild(fonts_ele);

	doc.SaveFile(cfg);
	doc.Clear();
}

void OpenCfg(MainEditor* editor, const char* cfg)
{
	TiXmlDocument doc;
	if (doc.LoadFile(cfg))
	{
		TiXmlElement* pRootEle = doc.RootElement();

		TiXmlElement* target_size_ele = pRootEle->FirstChildElement("target_size");
		editor->mTargetSize[0] = atoi(target_size_ele->Attribute("x"));
		editor->mTargetSize[1] = atoi(target_size_ele->Attribute("y"));

		TiXmlElement* default_size_ele = pRootEle->FirstChildElement("default_size");
		if (default_size_ele)
		{
			editor->mDefaultSize = atoi(default_size_ele->Attribute("size"));
		}

		TiXmlElement* fontsEle = pRootEle->FirstChildElement("fonts");
		for (size_t i = 0; i < editor->mArtFontGroups.size(); ++i)
		{
			ArtFontGroup* group = editor->mArtFontGroups[i];
			if (group)
			{
				SAFE_DELETE(group);
			}
		}
		editor->mArtFontGroups.clear();

		TiXmlElement* groupEle = fontsEle->FirstChildElement("group");
		while (groupEle)
		{
			int id = atoi(groupEle->Attribute("id"));
			ArtFontGroup* group = new ArtFontGroup(id);

			TiXmlElement* fontEle = groupEle->FirstChildElement("font");
			while (fontEle)
			{
				int code = atoi(fontEle->Attribute("code"));
				std::string relPath = fontEle->Attribute("path");
				std::string absPath = relPath;
				if (!editor->mOpenByAbsPath)
					EditorUtility::ToAbsPath(relPath, editor->mProjectDirectory, absPath);
				ArtFont* artFont = new ArtFont(code, absPath.c_str());
				group->mFontDatas.push_back(artFont);
				fontEle = fontEle->NextSiblingElement("font");
			}

			editor->mArtFontGroups.push_back(group);
			groupEle = groupEle->NextSiblingElement("group");
		}
		doc.Clear();
	}
}

void LoadFnt(MainEditor* editor, const char* cfg)
{
	if (editor->mFAS != NULL)
	{
		SAFE_DELETE(editor->mFAS);
	}
	editor->mFAS = new FontAtlasSeparate(editor->mDevice);
	editor->mShowFAS = true;

	editor->mFAS->LoadFnt(cfg);
}

void SaveTexs(MainEditor* editor, const char* cfg)
{
	std::string path = cfg;
	std::map<int, IDirect3DTexture9*>::iterator it;
	char fileTxt[64];
	for (it = editor->mFAS->mTexs.begin(); it != editor->mFAS->mTexs.end(); ++it)
	{
		IDirect3DTexture9* tex = it->second;

		memset(fileTxt, 0, 64);
		sprintf_s(fileTxt, "Font_%d.png", it->first);

		std::string destFile = path + "\\" + fileTxt;

		HRESULT hr = D3DXSaveTextureToFileA(destFile.c_str(), D3DXIFF_PNG, tex, NULL);
		if (hr != S_OK)
		{
			continue;
		}
	}
}

void MainEditor::OnMenu()
{
	openCfgFlag = false;
	saveCfgFlag = false;
	exportFlag = false;
	loadFntFlag = false;
	saveTexsFlag = false;
	ImGuiIO& io = ImGui::GetIO();
	if (io.KeyCtrl)
	{
		if (ImGui::IsKeyPressed('s') || ImGui::IsKeyPressed('S'))
		{
			saveCfgFlag = true;
			//SaveCfg(this);
		}
		if (ImGui::IsKeyPressed('o') || ImGui::IsKeyPressed('O'))
		{
			openCfgFlag = true;
		}
		if (ImGui::IsKeyPressed('b') || ImGui::IsKeyPressed('B'))
		{
			exportFlag = true;
		}
	}
	if (ImGui::BeginMenuBar())
	{
		if (ImGui::BeginMenu(STU("文件")))
		{
			if (ImGui::MenuItem(STU("保存(ctrl+s)"), NULL))
			{
				saveCfgFlag = true;
				//SaveCfg(this);
			}
			if (ImGui::MenuItem(STU("打开(ctrl+o)"), NULL))
			{
				openCfgFlag = true;
			}
			if (ImGui::MenuItem(STU("导出(ctrl+b)"), NULL))
			{
				exportFlag = true;
			}
			ImGui::Separator();
			ImGui::MenuItem(STU("按绝对路径打开"), NULL, &mOpenByAbsPath);
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu(STU("工具")))
		{
			ImGui::MenuItem(STU("预览"), NULL, &mShowTargetTex);
			ImGui::Separator();
			if (ImGui::MenuItem(STU("拆解字体图片"), NULL, (bool*)NULL, !mFAS))
			{
				loadFntFlag = true;
			}
			if (ImGui::MenuItem(STU("导出拆解图片"), NULL, (bool*)NULL, mFAS != NULL))
			{
				saveTexsFlag = true;
			}

			ImGui::EndMenu();
		}
		ImGui::EndMenuBar();
	}
}

void MainEditor::OnFAS()
{
	if (!mShowFAS && mFAS)
	{
		SAFE_DELETE(mFAS);
	}
	if (!mShowFAS || !mFAS)
		return;
	ImGui::Begin(STU("拆分字体图集"), &mShowFAS);
	std::map<int, IDirect3DTexture9*>::iterator it;
	for (it = mFAS->mTexs.begin(); it != mFAS->mTexs.end(); ++it)
	{
		IDirect3DTexture9* tex = it->second;
		ImTextureID tex_id = tex;
		D3DSURFACE_DESC desc;
		tex->GetLevelDesc(0, &desc);
		ImGui::Text("%d", it->first);
		ImGui::SameLine(0.0f, 10.0f);
		ImGui::Image(tex_id, ImVec2((float)desc.Width, (float)desc.Height), ImVec2(0, 0), ImVec2(1, 1), ImVec4(1, 1, 1, 1), ImVec4(0.7f, 0.7f, 0.7f, 1.0f));
	}
	ImGui::End();
}

void MainEditor::CreateTargetTexture()
{
	if (mTargetTexture)
	{
		mTargetTexture->Release();
		mTargetTexture = NULL;
	}
	int width = mTargetSize[0];
	int height = mTargetSize[1];
	HRESULT hr = D3DXCreateTexture(mDevice, width, height, D3DX_DEFAULT, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &mTargetTexture);
	if (!mTargetTexture)
	{
		return;
	}

	RECT rclock{ 0, 0, width, height };
	D3DLOCKED_RECT lrc;

	int curU = 0;
	int curV = 0;
	int maxV = 0;

	mTargetTexture->LockRect(0, &lrc, &rclock, 0);

	for (size_t i = 0; i < mArtFontGroups.size(); ++i)
	{
		ArtFontGroup* group = mArtFontGroups[i];
		for (size_t j = 0; j < group->mFontDatas.size(); ++j)
		{
			ArtFont* artFont = group->mFontDatas[j];
			if (artFont->mWidth >(unsigned int)width || artFont->mHeight >(unsigned int)height)
			{
				return;
			}
			int right = curU + artFont->mWidth;
 			int bottom = curV + artFont->mHeight;
			if (right > width)
			{
				curU = 0;
				curV += maxV;
				maxV = 0;
				right = curU + artFont->mWidth;
				bottom = curV + artFont->mHeight;
			}
			if (bottom > height)
			{
				return;
			}
			maxV = std::max<int>(artFont->mHeight, maxV);
			artFont->x = curU;
			artFont->y = curV;

			curU = right;

			RECT rcFont{ 0, 0, artFont->mWidth - 1, artFont->mHeight - 1 };
			D3DLOCKED_RECT lrcFont;

			artFont->mTexture->LockRect(0, &lrcFont, &rcFont, 0);
			
			for (int u = 0; u < (int)artFont->mWidth; ++u)
			{
				for (int v = 0; v < (int)artFont->mHeight; ++v)
				{
					int x = artFont->x + u;
					int y = artFont->y + v;
					((unsigned int*)lrc.pBits)[y * width + x] = ((unsigned int*)lrcFont.pBits)[v * artFont->mWidth + u];
				}
			}

			artFont->mTexture->UnlockRect(0);
		}
	}
	mTargetTexture->UnlockRect(0);

	mShowTargetTex = true;
}

void MainEditor::OnDestroy()
{
	LPDIRECT3DTEXTURE9 p = NULL;
	p = (LPDIRECT3DTEXTURE9)Global::mDiskTexID;
	if (p)
	{
		p->Release();
		Global::mDiskTexID = NULL;
	}
	
	p = (LPDIRECT3DTEXTURE9)Global::mFolderTexID;
	if (p)
	{
		p->Release();
		Global::mFolderTexID = NULL;
	}

	p = (LPDIRECT3DTEXTURE9)Global::mFileTexID;
	if (p)
	{
		p->Release();
		Global::mFileTexID = NULL;
	}
	if (mMesh)
	{
		mMesh->Release();
		mMesh = NULL;
	}
}
