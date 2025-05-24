#include "pch.hpp"

#define NOMINMAX
#define CURL_STATICLIB

#include <imgui_impl_dx9.h>
#include <imgui_impl_win32.h>

#include "CruelerThanDAT.h"
#include "globals.h"
#include "FileNodes.h"
#include "CodIcons.h"
#include "Log.h"
#include "themeLoader.h"
#include "BasicShaders.h"
#include "FileUtils.h"
#include "CTDSettings.h"
#include "CPKManager.h"


std::unordered_map<int, std::string> TEXTURE_DEF = { {0, "Albedo 0"}, {1, "Albedo 1"}, {2, "Normal"}, {3, "Blended Normal"}, {4, "Cubemap"}, {7, "Lightmap"}, {10, "Tension Map"} };
int TEXTURE_CAP = 512;

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
ImVec4 clear_color = ImVec4(0.0f, 0.0f, 0.0f, 1.00f);
static LPDIRECT3D9              g_pD3D = nullptr;
static LPDIRECT3DDEVICE9        g_pd3dDevice = nullptr;
static bool                     g_DeviceLost = false;
static UINT                     g_ResizeWidth = 0, g_ResizeHeight = 0;
static D3DPRESENT_PARAMETERS    g_d3dpp = {};

static LPDIRECT3DVERTEXSHADER9 pSolidShaderVTX = nullptr;
static LPDIRECT3DPIXELSHADER9 pSolidShaderPX = nullptr;

LPDIRECT3DTEXTURE9 g_RenderTargetTexture = nullptr;
LPDIRECT3DSURFACE9 g_RenderTargetSurface = nullptr;

// TODO: Delete
static D3DXMATRIX projMatrix;

float yaw = 0.0f;
float pitch = 0.0f;
float radius = 10.0f;

D3DXVECTOR3 target = D3DXVECTOR3(0, 0, 0);
D3DXVECTOR3 cameraPos;
D3DXMATRIX view;

static CTDSettings appConfig;

bool hasHandledArguments = false;
bool showViewport = true;

std::string downloadURL = "";
static std::unordered_map<unsigned int, LPDIRECT3DTEXTURE9> textureMap;
static std::unordered_map<unsigned int, std::vector<char>> rawTextureInfo;

ThemeManager* themeManager;
CPKManager* cpkManager;


bool showAllSCRMeshes = false;
bool cruelerLog = true;

namespace HelperFunction {
	FileNode* LoadNode(std::string fileName, const std::vector<char>& data, bool forceEndianess , bool bigEndian) {
		std::string fileExtension = fileName.substr(fileName.find_last_of("."));
		uint32_t fileType = 0;
		FileNode* outputFile = nullptr;

		// fix extension compare
		size_t nullPos = fileExtension.find('\0');
		if (nullPos != std::string::npos) {
			fileExtension = fileExtension.substr(0, nullPos);
		}

		if (data.size() >= 4) {
			fileType = *reinterpret_cast<const uint32_t*>(&data[0]);
		}



		// TODO: DAT files smaller than 4 bytes (empty) aren't recognized as DAT files
		if (fileExtension == ".uid") {
			outputFile = new UidFileNode(fileName);
			if (forceEndianess) {
				outputFile->fileIsBigEndian = bigEndian;
			}
			outputFile->SetFileData(data);
			outputFile->LoadFile();
		}
		else if (fileExtension == ".uvd") {
			outputFile = new UvdFileNode(fileName);
			if (forceEndianess) {
				outputFile->fileIsBigEndian = bigEndian;
			}
			outputFile->SetFileData(data);
			outputFile->LoadFile();
		}
		else if (fileType == 5521732) {
			outputFile = new DatFileNode(fileName);
			if (forceEndianess) {
				outputFile->fileIsBigEndian = bigEndian;
			}
			outputFile->SetFileData(data);
			outputFile->LoadFile();
		}
		else if (fileType == 1145588546) {
			outputFile = new BnkFileNode(fileName);
			if (forceEndianess) {
				outputFile->fileIsBigEndian = bigEndian;
			}
			outputFile->SetFileData(data);
			outputFile->LoadFile(); 
		}
		else if (fileType == 876760407) {
			outputFile = new WmbFileNode(fileName);
			if (forceEndianess) {
				outputFile->fileIsBigEndian = bigEndian;
			}
			outputFile->SetFileData(data);
			outputFile->LoadFile();
		}
		else if (fileType == 5391187) {
			outputFile = new ScrFileNode(fileName);
			if (forceEndianess) {
				outputFile->fileIsBigEndian = bigEndian;
			}
			outputFile->SetFileData(data);
			outputFile->LoadFile();
		}
		else if (fileType == 7630701) {
			outputFile = new MotFileNode(fileName);
			if (forceEndianess) {
				outputFile->fileIsBigEndian = bigEndian;
			}
			outputFile->SetFileData(data);
			outputFile->LoadFile();
		}
		else if (fileType == 5000536 || fileType == 5068866) {
			outputFile = new BxmFileNode(fileName);
			outputFile->fileIsBigEndian = false;
			
			outputFile->SetFileData(data);
			outputFile->LoadFile();
		}
		else if (fileType == 3299660) {
			outputFile = new LY2FileNode(fileName);
			if (forceEndianess) {
				outputFile->fileIsBigEndian = bigEndian;
			}
			outputFile->SetFileData(data);
			outputFile->LoadFile();
		}
		else if (fileType == 1481001298) {
			outputFile = new WemFileNode(fileName);
			if (forceEndianess) {
				outputFile->fileIsBigEndian = bigEndian;
			}
			outputFile->SetFileData(data);
			outputFile->LoadFile();
		}
		else if (fileType == 4346967) {
			outputFile = new WtbFileNode(fileName);
			if (forceEndianess) {
				outputFile->fileIsBigEndian = bigEndian;
			}
			outputFile->SetFileData(data);
			//outputFile->LoadFile();
		}
		else {
			outputFile = new UnkFileNode(fileName);
			if (forceEndianess) {
				outputFile->fileIsBigEndian = bigEndian;
			}
			outputFile->SetFileData(data);
		}
		

		


		return outputFile;
	}
}

void CreateViewportRT(int width, int height)
{
    g_pd3dDevice->CreateTexture(width, height, 1, D3DUSAGE_RENDERTARGET,
        D3DFMT_A32B32G32R32F, D3DPOOL_DEFAULT, &g_RenderTargetTexture, NULL);

    g_RenderTargetTexture->GetSurfaceLevel(0, &g_RenderTargetSurface);
}

FileNode* FileNodeFromFilepath(std::string filePath) {
	auto start = std::chrono::high_resolution_clock::now();
	std::vector<char> fileData;
	if (!ReadFileIntoVector(filePath, fileData)) {
		// TODO: Handle this
	}
	// TODO: Do we need this? auto start_noio = std::chrono::high_resolution_clock::now();
	FileNode* node = HelperFunction::LoadNode(filePath, fileData);
	auto finish = std::chrono::high_resolution_clock::now();
	auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(finish - start);
	// TODO: Do we need this? auto millisecondsnoio = std::chrono::duration_cast<std::chrono::milliseconds>(finish - start_noio);
	CTDLog::Log::getInstance().LogNote("Loaded " + filePath + " in " + std::to_string(milliseconds.count()) + "ms");



	return node;
}

std::string WCharToString(const wchar_t* wstr) {
	if (!wstr) return "";

	int size_needed = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, nullptr, 0, nullptr, nullptr);
	std::string str(size_needed, 0);
	WideCharToMultiByte(CP_UTF8, 0, wstr, -1, &str[0], size_needed, nullptr, nullptr);

	return str;
}

std::string getDTTPath(const std::string& originalPath) {
	fs::path path(originalPath);
	if (path.has_filename() && path.has_parent_path()) {
		fs::path newPath = path.parent_path(); // Go up one dir
		newPath /= path.stem();      // Add the filename without extension
		newPath += ".dtt";           // Add .dtt extension
		return newPath.string();
	}
	return "";
}

void DX9WTAWTPLoad(BinaryReader& WTA, BinaryReader& WTP) {
	if (WTA.GetSize() < 28 || WTP.GetSize() <= 0) {
		return;
	}

	WTA.Seek(8);
	unsigned int textureCount = WTA.ReadUINT32();
	unsigned int offsetTextureOffsets = WTA.ReadUINT32();
	unsigned int offsetTextureSizes = WTA.ReadUINT32();
	WTA.Skip(sizeof(unsigned int)); // TODO: offsetTextureFlags
	unsigned int offsetTextureIdx = WTA.ReadUINT32();
	WTA.Skip(sizeof(unsigned int)); // Not bothered

	std::vector<int> offsets;
	WTA.Seek(offsetTextureOffsets);
	for (unsigned int i = 0; i < textureCount; i++) {
		offsets.push_back(WTA.ReadUINT32());
	}

	std::vector<int> sizes;
	WTA.Seek(offsetTextureSizes);
	for (unsigned int i = 0; i < textureCount; i++) {
		sizes.push_back(WTA.ReadUINT32());
	}

	std::vector<int> idx;
	WTA.Seek(offsetTextureIdx);
	for (unsigned int i = 0; i < textureCount; i++) {
		idx.push_back(WTA.ReadUINT32());
	}


	for (unsigned int i = 0; i < textureCount; i++) {
		LPDIRECT3DTEXTURE9 tmpTexture;
		WTP.Seek(offsets[i]);
		std::vector<char> data = WTP.ReadBytes(sizes[i]);
		LPCVOID ptr = static_cast<LPCVOID>(data.data());
		HRESULT hr = D3DXCreateTextureFromFileInMemory(g_pd3dDevice, ptr, sizes[i], &tmpTexture);
		if (FAILED(hr)) {
			CTDLog::Log::getInstance().LogError("Failed to load texture");
		}
		else {
			rawTextureInfo[idx[i]] = data;
			textureMap[idx[i]] = tmpTexture;
		}

	}

}


void PopulateTextures() {
	// keep track of variables: challenge level impossible
	for (FileNode* node : openFiles) {
		if (fs::exists(getDTTPath(node->fileName))) {
			FileNode* dtt = FileNodeFromFilepath(getDTTPath(node->fileName));
			for (FileNode* dtnode : dtt->children) {
				if (dtnode->fileExtension == "wtp") {
					FileNode* wtpFile = dtnode;
					FileNode* wtaFile = nullptr;
					for (FileNode* pnode : node->children) {
						if (pnode->fileExtension == "wta") {
							wtaFile = pnode;
						}
					}

					if (wtaFile) {
						// We have all the data, it's time to lock the fuck in
						BinaryReader wta = BinaryReader(wtaFile->fileData);
						BinaryReader wtp = BinaryReader(wtpFile->fileData);
						wta.SetEndianess(node->fileIsBigEndian);

						DX9WTAWTPLoad(wta, wtp);

						printf("Loading textures...");
					}

				}
				else if (dtnode->fileExtension == "wtb") {
					BinaryReader wta = BinaryReader(dtnode->fileData);
					BinaryReader wtp = BinaryReader(dtnode->fileData);
					wta.SetEndianess(node->fileIsBigEndian);

					DX9WTAWTPLoad(wta, wtp);
				}
			}

		}
		else {
			CTDLog::Log::getInstance().LogWarning("No DTT file associated with " + node->fileName);
		}


	}

}


size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
	((std::string*)userp)->append((char*)contents, size * nmemb);
	return size * nmemb;
}

void SelfUpdate() {
	CURL* curl = curl_easy_init();
	std::string response;

	if (curl) {
		curl_easy_setopt(curl, CURLOPT_URL, "https://api.github.com/repos/Gaming-With-Portals/CruelerThanDAT/releases/latest");
		curl_easy_setopt(curl, CURLOPT_USERAGENT, "CruelerThanDAT");
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
		curl_easy_perform(curl);
		curl_easy_cleanup(curl);
	}
	if (response == "") {
		return;
	}

	nlohmann::json data = nlohmann::json::parse(response);
	std::string tag = data["tag_name"];
	size_t underscorePos = tag.rfind('_');
	if (underscorePos != std::string::npos) {
		std::string numberPart = tag.substr(underscorePos + 1);
		int newBuild = std::stoi(numberPart);
		if (newBuild > BUILD_NUMBER) {
			CTDLog::Log::getInstance().LogUpdate();
			downloadURL = data["assets"][0]["browser_download_url"];
			if (downloadURL != "") {
				SHOULD_UPDATE = true;
			}
			
		}
		

	}


	
}
int lastMouseX = 0;
int lastMouseY = 0;
bool isDragging = false;

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
		return true;

    switch (msg)
    {
    case WM_LBUTTONDOWN:
    {
        isDragging = true;
        lastMouseX = LOWORD(lParam);
        lastMouseY = HIWORD(lParam);
        SetCapture(hWnd); // lock mouse input to window
        break;
    }

    case WM_LBUTTONUP:
    {
        isDragging = false;
        ReleaseCapture();
        break;
    }

    case WM_MOUSEMOVE:
    {
        if (isDragging)
        {
            int x = LOWORD(lParam);
            int y = HIWORD(lParam);

            int dx = x - lastMouseX;
            int dy = y - lastMouseY;

            lastMouseX = x;
            lastMouseY = y;

            float sensitivity = 0.01f;
            yaw += dx * sensitivity;
            pitch += dy * sensitivity;
        }
        break;
    }
    case WM_MOUSEWHEEL:
    {
        short delta = GET_WHEEL_DELTA_WPARAM(wParam); // +120 or -120
        float scrollSpeed = 5.0f; // tweak for sensitivity
        radius -= (delta / 120.0f) * scrollSpeed;


        radius = std::max(2.0f, std::min(1000.0f, radius));
        break;
    }

    case WM_SIZE:
        if (wParam == SIZE_MINIMIZED)
            return 0;
        g_ResizeWidth = (UINT)LOWORD(lParam); // Queue resize
        g_ResizeHeight = (UINT)HIWORD(lParam);
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
            return 0;
        break;
    case WM_DESTROY:
        ::PostQuitMessage(0);
        return 0;
    case WM_CREATE:
        DragAcceptFiles(hWnd, TRUE);  // Enable drag-and-drop
        break;
    case WM_DROPFILES: {
        HDROP hDrop = (HDROP)wParam;
        wchar_t filePath[MAX_PATH] = { 0 };

		// Get the first dropped file (you can loop for multiple files)
		if (DragQueryFileW(hDrop, 0, filePath, MAX_PATH)) {
			
			openFiles.push_back(FileNodeFromFilepath(WCharToString(filePath)));
			if (appConfig.AutomaticallyLoadTextures) {
				PopulateTextures();
			}

			// TODO: Process the file (load texture, model, etc.)
		}

		DragFinish(hDrop); // Free resources
		break;
		}
	}
	return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}

bool CreateDeviceD3D(HWND hWnd)
{
	if ((g_pD3D = Direct3DCreate9(D3D_SDK_VERSION)) == nullptr)
		return false;
	
	// Create the D3DDevice
	ZeroMemory(&g_d3dpp, sizeof(g_d3dpp));
	g_d3dpp.Windowed = TRUE;
	g_d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	g_d3dpp.BackBufferFormat = D3DFMT_UNKNOWN; // Need to use an explicit format with alpha if needing per-pixel alpha composition.
	g_d3dpp.EnableAutoDepthStencil = TRUE;
	g_d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
	g_d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_ONE;           // Present with vsync
	//g_d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;   // Present without vsync, maximum unthrottled framerate
	if (g_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &g_d3dpp, &g_pd3dDevice) < 0)
		return false;

	LPD3DXBUFFER pVertexShaderBuffer = nullptr;
	LPD3DXBUFFER pPixelShaderBuffer = nullptr;

	D3DXCompileShader(solidColorVTX, static_cast<UINT>(strlen(solidColorVTX)),
		NULL, NULL, "main", "vs_3_0", 0,
		&pVertexShaderBuffer, NULL, NULL);

	D3DXCompileShader(solidColorPX, static_cast<UINT>(strlen(solidColorPX)),
		NULL, NULL, "main", "ps_3_0", 0,
		&pPixelShaderBuffer, NULL, NULL);

	g_pd3dDevice->CreateVertexShader((DWORD*)pVertexShaderBuffer->GetBufferPointer(), &pSolidShaderVTX);
	g_pd3dDevice->CreatePixelShader((DWORD*)pPixelShaderBuffer->GetBufferPointer(), &pSolidShaderPX);

	g_pd3dDevice->SetVertexShader(pSolidShaderVTX);
	g_pd3dDevice->SetPixelShader(pSolidShaderPX);

	return true;
}

void CleanupDeviceD3D()
{
	if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = nullptr; }
	if (g_pD3D) { g_pD3D->Release(); g_pD3D = nullptr; }
}

void ResetDevice()
{
	ImGui_ImplDX9_InvalidateDeviceObjects();
	HRESULT hr = g_pd3dDevice->Reset(&g_d3dpp);
	if (hr == D3DERR_INVALIDCALL)
		IM_ASSERT(0);
	ImGui_ImplDX9_CreateDeviceObjects();
}






void RenderFrame() {



    static float fov = 50.0f;
    static float index = 180.0f;  
    //static float cameraPos[3] = { 0.0f, 0.0f, -15.0f };
    static float cameraVec[3] = { -1.0f, 0.2f, 0.0f };
	(void)cameraVec;
    static bool spinModel = false;
    g_pd3dDevice->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
    g_pd3dDevice->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
    g_pd3dDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
    g_pd3dDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
    g_pd3dDevice->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_NONE);

	// Start the ImGui frame
	ImGui_ImplDX9_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	ImGui::SetNextWindowPos(ImVec2(0, 0));
	ImGui::SetNextWindowSize(ImVec2(static_cast<float>(g_d3dpp.BackBufferWidth), 36));

	ImGui::Begin("TabCtrl", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar);
	
	if (SHOULD_UPDATE) {
		
		if (ImGui::Button("Update")) {
			SHOULD_UPDATE = false;
			URLDownloadToFileA(NULL, downloadURL.c_str(), "update.zip", NULL, NULL);
			ShellExecute(NULL, L"open", L"update.bat", NULL, NULL, SW_SHOWNORMAL);
			exit(0);

		}
		ImGui::SameLine();
	}

	ImGui::Text("Textures Loaded: %zu/%d", textureMap.size(), TEXTURE_CAP);


	ImGui::End();

	ImGui::SetNextWindowPos(ImVec2(0, 36));
	ImGui::SetNextWindowSize(ImVec2(350, static_cast<float>(g_d3dpp.BackBufferHeight) - 36));

	ImGui::Begin("DatView", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);

	if (ImGui::BeginTabBar("primary_control")) {
		
		if (ImGui::BeginTabItem("Data Viewer")) {
			if (openFiles.size() == 0) {
				ImGui::Text("No open files yet!");
				ImGui::Text("Drag a file onto the window to get started");
			}


			for (FileNode* node : openFiles) {
				node->Render();
			}

			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("CPK Viewer")) {

			cpkManager->Render();


			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Configuration")) {
			if (ImGui::Button("SAVE")) {
				appConfig.Write();
			}

			ImGui::Text("WMB/SCR");
			ImGui::Checkbox("Automatically Load Textures", &appConfig.AutomaticallyLoadTextures);
			ImGui::Checkbox("Show All SCR Meshes By Default", &appConfig.ShowAllMeshesByDefault);

			ImGui::Text("Theme");
			if (ImGui::Button("ImGui Theme")) {
				themeManager->ChooseStyle(0);
			}
			if (ImGui::Button("Visual Studio Theme")) {
				themeManager->ChooseStyle(1);
			}
			if (ImGui::Button("Half Life Theme")) {
				themeManager->ChooseStyle(2);
			}

			ImGui::EndTabItem();
		}



		ImGui::EndTabBar();
	}





	ImGui::End();

	ImGui::SetNextWindowPos(ImVec2(350, 36));
	int window_height = 0;
	if (cruelerLog) {
		ImGui::SetNextWindowSize(ImVec2(static_cast<float>(g_d3dpp.BackBufferWidth) - 350, static_cast<float>(g_d3dpp.BackBufferHeight) - 335));
		window_height = g_d3dpp.BackBufferHeight - 335;
	}
	else {
		ImGui::SetNextWindowSize(ImVec2(static_cast<float>(g_d3dpp.BackBufferWidth) - 350, static_cast<float>(g_d3dpp.BackBufferHeight) - 75));
		window_height = g_d3dpp.BackBufferHeight - 80;
	}
	
	if (showViewport == false) {
		ImGui::SetNextWindowBgAlpha(0.1f);
	}
	else {
		ImGui::SetNextWindowBgAlpha(1.0f);
	}

	ImGui::Begin("Editor", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

    if (g_RenderTargetSurface == nullptr || g_RenderTargetTexture == nullptr) {
        CreateViewportRT(ImGui::GetWindowSize().x, ImGui::GetWindowSize().y);
    }
    LPDIRECT3DSURFACE9 g_OriginalBackBuffer = nullptr;
    g_pd3dDevice->GetRenderTarget(0, &g_OriginalBackBuffer);
    g_pd3dDevice->SetRenderTarget(0, g_RenderTargetSurface);
    g_pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,
        D3DCOLOR_XRGB(50, 50, 50), 1.0f, 0);

	if (FileNode::selectedNode) {
		if (FileNode::selectedNode->nodeType == FileNodeTypes::BXM) {
			showViewport = true;
			ImGui::Text("- Binary XML");
			BxmFileNode* bxmNode = ((BxmFileNode*)FileNode::selectedNode);
			bxmNode->RenderGUI();
		}
		else if (FileNode::selectedNode->nodeType == FileNodeTypes::WTB) {
			showViewport = true;
			ImGui::Text("- Textures");
			WtbFileNode* wtbNode = ((WtbFileNode*)FileNode::selectedNode);
			if (ImGui::BeginListBox("##texturebox", ImVec2{ (float)g_d3dpp.BackBufferWidth - 350.0f, (float)window_height })) {
				for (int x = 0; x < wtbNode->textureCount; x++) {

					ImGui::Text("ID: %i", wtbNode->textureIdx[x]);

				}


			}
			ImGui::EndListBox();
		}
		else if (FileNode::selectedNode->nodeType == FileNodeTypes::LY2) {
			LY2FileNode* ly2Node = ((LY2FileNode*)FileNode::selectedNode);
			ly2Node->RenderGUI();
		}
		else if (FileNode::selectedNode->nodeType == FileNodeTypes::UID) {
			UidFileNode* uidNode = ((UidFileNode*)FileNode::selectedNode);
			uidNode->RenderGUI();
		}
		else if (FileNode::selectedNode->nodeType == FileNodeTypes::UVD) {
			UvdFileNode* uvdNode = ((UvdFileNode*)FileNode::selectedNode);
			uvdNode->RenderGUI();
		}
		else if (FileNode::selectedNode->nodeType == FileNodeTypes::WMB) {
			showViewport = false;
			WmbFileNode* wmbNode = ((WmbFileNode*)FileNode::selectedNode);
			
			if (ImGui::BeginTabBar("wmb_editor")) {
				if (ImGui::BeginTabItem("Meshes")) {
					wmbNode->RenderGUI();
					ImGui::EndTabItem();
				}

				if (ImGui::BeginTabItem("Materials")) {
					if (ImGui::Button("Fetch Textures")) {

						PopulateTextures();
					}


					int i = 0;
					for (CTDMaterial &mat : wmbNode->materials) {
						
						if (ImGui::TreeNode((("Material " + std::to_string(i))).c_str())) {

							ImGui::Text("Shader Name: %s", mat.shader_name.c_str());

							for (const auto& pair : mat.texture_data) {
								std::string textureString = "Tex" + std::to_string(pair.first) + ": %d";
								if (TEXTURE_DEF.find(pair.first) != TEXTURE_DEF.end()) {
									textureString = TEXTURE_DEF[pair.first] + ": %d";
								}

								ImGui::Text(textureString.c_str(), pair.second);
							}

							ImGui::TreePop();
						}
						
						i += 1;
					}


					ImGui::EndTabItem();
				}


				if (wmbNode->isSCR) {
					if (ImGui::BeginTabItem("SCR Options")) {
						ImGui::Checkbox("Show All SCR Meshes", &showAllSCRMeshes);

						ImGui::EndTabItem();
					}
				}

                if (ImGui::BeginTabItem("Visualizer")) {
                    ImGui::SetNextItemWidth(120.0f);
                    ImGui::SliderFloat("FOV", &fov, 20.0f, 100.0f);
                    ImGui::Checkbox("Spin Model?", &spinModel);
                    ImGui::EndTabItem();
                }

				ImGui::EndTabBar();
			}



		}

	}

    ImVec2 pos = ImVec2(350, 36);
    ImVec2 size = ImGui::GetWindowSize();
    ImGui::GetBackgroundDrawList()->AddImage(
        (void*)g_RenderTargetTexture, pos, ImVec2(pos.x + size.x, pos.y + size.y));


	ImGui::End();


	
	if (cruelerLog) {
		ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.1f, 0.1f, 0.1f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 1.0f, 1.0f, 0.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 1.0f, 1.0f, 0.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(1.0f, 1.0f, 1.0f, 0.0f));

		ImGui::SetNextWindowPos(ImVec2(350, static_cast<float>(g_d3dpp.BackBufferHeight )- 300));
		ImGui::SetNextWindowSize(ImVec2(static_cast<float>(g_d3dpp.BackBufferWidth )- 350, 300));
		ImGui::Begin("CruelerThanDAT Log", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
		
		if (ImGui::Button(ICON_CI_TERMINAL, ImVec2(25, 25))){
			cruelerLog = false;
		}
		ImGui::SameLine();
		
		if (ImGui::Button(ICON_CI_CLEAR_ALL, ImVec2(25, 25))) {
			CTDLog::Log::getInstance().logEntries.clear();
		}


		for (CTDLog::LogEntry* log : CTDLog::Log::getInstance().logEntries) {
			if (log->text == "update") {
				ImGui::TextColored(log->color, "%s CruelerThanDAT is ready to update! An update button has appeared in the top bar", std::string(ICON_CI_DESKTOP_DOWNLOAD).c_str());

			}
			else {
				ImGui::TextColored(log->color, "%s", log->text.c_str());
			}
			
		}

		ImGui::PopStyleColor(4);
	}
	else {
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 1.0f, 1.0f, 0.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 1.0f, 1.0f, 0.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(1.0f, 1.0f, 1.0f, 0.0f));
		ImGui::SetNextWindowPos(ImVec2(350, static_cast<float>(g_d3dpp.BackBufferHeight) - 40));
		ImGui::SetNextWindowSize(ImVec2(static_cast<float>(g_d3dpp.BackBufferWidth) - 350, 40));
		ImGui::Begin("CruelerThanDAT Log", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar);
		if (ImGui::Button(ICON_CI_TERMINAL, ImVec2(25, 25))) {
			cruelerLog = true;
		}
		ImGui::SameLine();
		ImGui::Text("CruelerThanDAT Log");

		ImGui::PopStyleColor(3);
	}


    //ImGui::Image((void*)g_RenderTargetTexture, ImVec2(128, 128));

	ImGui::End();
	
	if (spinModel) {
		index += 0.5f;
		if (index > 360.0f) {
			index = 0.0f;
		}
	}

	D3DXMATRIX matRotateY;    // a matrix to store the rotation information


	// build a matrix to rotate the model based on the increasing float value
	D3DXMatrixRotationY(&matRotateY, D3DXToRadian(index));

	// tell Direct3D about our matrix
	g_pd3dDevice->SetTransform(D3DTS_WORLD, &matRotateY);

	D3DXMATRIX matView;    // the view transform matrix

    pitch = std::max(-D3DX_PI * 0.49f, std::min(D3DX_PI * 0.49f, pitch));

    float x = radius * cosf(pitch) * sinf(yaw);
    float y = radius * sinf(pitch);
    float z = radius * cosf(pitch) * cosf(yaw);
    cameraPos = D3DXVECTOR3(x, y, z) + target;

    D3DXVECTOR3 up(0.0f, 1.0f, 0.0f);
    D3DXMatrixLookAtLH(&view, &cameraPos, &target, &up);
    g_pd3dDevice->SetTransform(D3DTS_VIEW, &view);

	D3DXMATRIX matProjection;     // the projection transform matrix

    D3DXMatrixPerspectiveFovLH(&matProjection,
        D3DXToRadian(fov),    // the horizontal field of view
        (FLOAT)size.x / (FLOAT)size.y, // aspect ratio
        1.0f,    // the near view-plane
        5000.0f);    // the far view-plane

	g_pd3dDevice->SetTransform(D3DTS_PROJECTION, &matProjection);
	




    g_pd3dDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);
    D3DCOLOR clear_col_dx = D3DCOLOR_RGBA((int)(clear_color.x * clear_color.w * 255.0f), (int)(clear_color.y * clear_color.w * 255.0f), (int)(clear_color.z * clear_color.w * 255.0f), (int)(clear_color.w * 255.0f));
    g_pd3dDevice->Clear(0, nullptr, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, clear_col_dx, 1.0f, 0);
    g_pd3dDevice->Clear(0, NULL, D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);
	
	g_pd3dDevice->SetRenderState(D3DRS_COLORWRITEENABLE,
		D3DCOLORWRITEENABLE_RED | 
		D3DCOLORWRITEENABLE_GREEN | 
		D3DCOLORWRITEENABLE_BLUE);
    if (g_pd3dDevice->BeginScene() >= 0)
    {
		const float pointSize = 5.0f;
		g_pd3dDevice->SetRenderState(D3DRS_POINTSIZE, *reinterpret_cast<const DWORD *>(&pointSize));
        g_pd3dDevice->SetRenderState(D3DRS_LIGHTING, FALSE);  // unless you're using normals and lights
        g_pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE); // disable backface culling for now
        g_pd3dDevice->SetRenderState(D3DRS_ZENABLE, TRUE);

		if (FileNode::selectedNode && FileNode::selectedNode->nodeType == FileNodeTypes::WMB) {
			WmbFileNode* wmbNode = ((WmbFileNode*)FileNode::selectedNode);
			g_pd3dDevice->SetTexture(0, textureMap[0]);

			if (showAllSCRMeshes && wmbNode->isSCR) {
				ScrFileNode* scrNode = (ScrFileNode*)wmbNode->scrNode;
				for (FileNode* child : scrNode->children) {
					WmbFileNode* scrChildNode = (WmbFileNode*)child;
					scrChildNode->RenderMesh();
				}

			}
			else {
				wmbNode->RenderMesh();
			}
			


		}

        g_pd3dDevice->EndScene();
		
		g_pd3dDevice->SetRenderState(D3DRS_COLORWRITEENABLE,
			D3DCOLORWRITEENABLE_RED | 
			D3DCOLORWRITEENABLE_GREEN | 
			D3DCOLORWRITEENABLE_BLUE |
			D3DCOLORWRITEENABLE_ALPHA);

        g_pd3dDevice->SetRenderTarget(0, g_OriginalBackBuffer);
        g_pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,
            D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);
        ImGui::EndFrame();
        g_pd3dDevice->BeginScene();


        g_pd3dDevice->SetRenderState(D3DRS_ZENABLE, FALSE);
        g_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);


		ImGui::Render();
		ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());


		g_pd3dDevice->EndScene();
	}
	HRESULT result = g_pd3dDevice->Present(nullptr, nullptr, nullptr, nullptr);
	if (result == D3DERR_DEVICELOST)
		g_DeviceLost = true;
}





int main(int argc, char* argv[])
{
	printf("-- CruelerThanDAT --\n");

	printf("Setting working directory...\n");
	char exePath[MAX_PATH];
	GetModuleFileNameA(NULL, exePath, MAX_PATH);

	// Strip off the executable name to get the directory
	std::string path(exePath);
	size_t lastSlash = path.find_last_of("\\/");
	if (lastSlash != std::string::npos) {
		path = path.substr(0, lastSlash);
		SetCurrentDirectoryA(path.c_str());
	}

	if (!std::filesystem::exists("Assets")) {
		printf("Assets are missing or corrupt. (Case 0)");
		std::cin.get();
	}
	else if (!std::filesystem::exists("Assets/img.dds")) {
		printf("Assets are missing or corrupt. (Case 1)");
		std::cin.get();
	}
	else if (!std::filesystem::exists("Assets/Themes")) {
		printf("Themes are missing or corrupt. (Case 0)");
		std::cin.get();
	}
	else if (!std::filesystem::exists("Assets/Model")) {
		printf("DirectX Data is missing or corrupt. (Case 0)");
		std::cin.get();
	}

	themeManager = new ThemeManager();
	themeManager->UpdateThemeList();
	

	appConfig.Read();


	cpkManager = new CPKManager();
	cpkManager->Init("");

	printf("D3DInit...");
	CTDLog::Log::getInstance().LogNote("Launching CruelerThanDAT...");
	CTDLog::Log::getInstance().LogNote("Waiting for DirectX9...");

	WNDCLASSEXW wc = { sizeof(wc), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr, L"ImGui Example", nullptr };
	::RegisterClassExW(&wc);
	HWND hwnd = ::CreateWindowW(wc.lpszClassName, L"CruelerThanDAT", WS_OVERLAPPEDWINDOW, 100, 100, 1600, 900, nullptr, nullptr, wc.hInstance, nullptr);


	// Initialize Direct3D
	if (!CreateDeviceD3D(hwnd))
	{
		CleanupDeviceD3D();
		::UnregisterClassW(wc.lpszClassName, wc.hInstance);
		return 1;
	}

	::ShowWindow(hwnd, SW_SHOWDEFAULT);
	::UpdateWindow(hwnd);

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();

	ImGui_ImplWin32_Init(hwnd);
	ImGui_ImplDX9_Init(g_pd3dDevice);
	
	// Load Fonts
	io.Fonts->AddFontDefault();

	ImFontConfig config;
	config.MergeMode = true;
	config.GlyphMinAdvanceX = 20;
	config.GlyphOffset = ImVec2(0, 5);
	static const ImWchar icon_ranges[] = { ICON_MIN_CI, ICON_MAX_CI, 0 };
	io.Fonts->AddFontFromFileTTF("Assets/codicon.ttf", 18, &config, icon_ranges);

	std::ifstream infile("Assets/codicon.ttf");
	if (!infile.good()) {
		CTDLog::Log::getInstance().LogError("Unable to locate Assets/codicons.ttf, icons will be broken!");
	}
	infile.close();

#ifdef _DEBUG

	CTDLog::Log::getInstance().LogWarning("CruelerThanDAT is running with DEBUG enabled, if this is a published build, please contact GamingWithPortals");
#endif


	
	themeManager->ChooseStyle(1);

	LPDIRECT3DTEXTURE9 pTexture;
	HRESULT hr = D3DXCreateTextureFromFile(g_pd3dDevice, L"Assets/img.dds", &pTexture);

	if (FAILED(hr)) {
		MessageBox(0, L"Failed to load texture!", L"Error", MB_OK);
		
	}

	printf("Loading basic texture...");
	textureMap[0] = pTexture;

	if (appConfig.ShowAllMeshesByDefault) {
		showAllSCRMeshes = true;
	}

	::ShowWindow(::GetConsoleWindow(), SW_HIDE);


	CTDLog::Log::getInstance().LogNote("CruelerThanDAT Ready");

	SelfUpdate();

	bool done = false;
	while (!done)
	{
		MSG msg;
		while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
		{
			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
			if (msg.message == WM_QUIT)
				done = true;
		}
		if (done)
			break;

		if (g_DeviceLost)
		{
			HRESULT hr0 = g_pd3dDevice->TestCooperativeLevel();
			if (hr0 == D3DERR_DEVICELOST)
			{
				::Sleep(10);
				continue;
			}
			if (hr0 == D3DERR_DEVICENOTRESET)
				ResetDevice();
			g_DeviceLost = false;
		}

		if (g_ResizeWidth != 0 && g_ResizeHeight != 0)
		{
			g_d3dpp.BackBufferWidth = g_ResizeWidth;
			g_d3dpp.BackBufferHeight = g_ResizeHeight;
			g_ResizeWidth = g_ResizeHeight = 0;
			ResetDevice();
		}

		RenderFrame();

		if (!hasHandledArguments) {
			for (int i = 1; i < argc; ++i) {
				if (std::filesystem::exists(argv[i])) {
					openFiles.push_back(FileNodeFromFilepath(argv[i]));

				}
			}
			hasHandledArguments = true;
		}

	}

}


