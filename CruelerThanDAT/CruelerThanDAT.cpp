// CruelerThanDAT.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#define NOMINMAX
#include <Windows.h>
#include <d3d9.h>
#include <d3dx9.h>
#include "imgui.h"
#include "imgui_impl_dx9.h"
#include "imgui_impl_win32.h"
#include <iostream>
#include "globals.h"
#include "Core/FileNodes.h"
#include <vector>
#include <fstream>
#include <memory>
#include "Assets/CodIcons.h"
#include <shellapi.h>
#include <chrono>
#include "Core/Log.h"
#include "Themes/themeLoader.h"
#include "Core/Utility/BasicShaders.h"
#include "Core/Utility/FileUtils.h"
#include "Core/CTDSettings.h"

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

static D3DXMATRIX viewMatrix;
static D3DXMATRIX projMatrix;

static CTDSettings appConfig;

bool showViewport = true;
std::vector<FileNode*> openFiles;
static std::unordered_map<unsigned int, LPDIRECT3DTEXTURE9> textureMap;

ThemeManager* themeManager;

bool showAllSCRMeshes = false;
bool cruelerLog = true;

namespace HelperFunction {
    FileNode* LoadNode(std::string fileName, const std::vector<char>& data, bool forceEndianess , bool bigEndian) {
        std::string fileExtension = fileName.substr(fileName.find_last_of("."));
        uint32_t fileType = 0;
        FileNode* outputFile = nullptr;
        if (data.size() >= 4) {
            fileType = *reinterpret_cast<const uint32_t*>(&data[0]);
        }



        // TODO: DAT files smaller than 4 bytes (empty) aren't recognized as DAT files
        if (fileType == 5521732) {
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

FileNode* FileNodeFromFilepath(std::string filePath) {
    auto start = std::chrono::high_resolution_clock::now();
    std::vector<char> fileData;
    if (!ReadFileIntoVector(filePath, fileData)) {
    }
    auto start_noio = std::chrono::high_resolution_clock::now();
    FileNode* node = HelperFunction::LoadNode(filePath, fileData);
    auto finish = std::chrono::high_resolution_clock::now();
    auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(finish - start);
    auto millisecondsnoio = std::chrono::duration_cast<std::chrono::milliseconds>(finish - start_noio);
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
    WTA.Seek(8);
    unsigned int textureCount = WTA.ReadUINT32();
    unsigned int offsetTextureOffsets = WTA.ReadUINT32();
    unsigned int offsetTextureSizes = WTA.ReadUINT32();
    unsigned int offsetTextureFlags = WTA.ReadUINT32();
    unsigned int offsetTextureIdx = WTA.ReadUINT32();
    WTA.ReadUINT32(); // Not bothered

    std::vector<int> offsets;
    WTA.Seek(offsetTextureOffsets);
    for (int i = 0; i < textureCount; i++) {
        offsets.push_back(WTA.ReadUINT32());
    }

    std::vector<int> sizes;
    WTA.Seek(offsetTextureSizes);
    for (int i = 0; i < textureCount; i++) {
        sizes.push_back(WTA.ReadUINT32());
    }

    std::vector<int> idx;
    WTA.Seek(offsetTextureIdx);
    for (int i = 0; i < textureCount; i++) {
        idx.push_back(WTA.ReadUINT32());
    }


    for (int i = 0; i < textureCount; i++) {
        LPDIRECT3DTEXTURE9 tmpTexture;
        WTP.Seek(offsets[i]);
        std::vector<char> data = WTP.ReadBytes(sizes[i]);
        LPCVOID ptr = static_cast<LPCVOID>(data.data());
        D3DXCreateTextureFromFileInMemory(g_pd3dDevice, ptr, sizes[i], &tmpTexture);

        textureMap[idx[i]] = tmpTexture;
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

                        DX9WTAWTPLoad(wta, wtp);

                        printf("Loading textures...");
                    }

                }
                else if (dtnode->fileExtension == "wtb") {
                    BinaryReader wta = BinaryReader(dtnode->fileData);
                    BinaryReader wtp = BinaryReader(dtnode->fileData);

                    DX9WTAWTPLoad(wta, wtp);
                }
            }

        }
        else {
            CTDLog::Log::getInstance().LogWarning("No DTT file associated with " + node->fileName);
        }


    }

}


LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg)
    {
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

	D3DXCompileShader(solidColorVTX, strlen(solidColorVTX),
		NULL, NULL, "main", "vs_3_0", 0,
		&pVertexShaderBuffer, NULL, NULL);

	D3DXCompileShader(solidColorPX, strlen(solidColorPX),
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
    static float fov = 20.0f;
    static float index = 180.0f;    // an ever-increasing float value
    static float cameraPos[3] = { 0.0f, 0.0f, -15.0f };
    static bool spinModel = false;

    // Start the ImGui frame
    ImGui_ImplDX9_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2(g_d3dpp.BackBufferWidth, 36));

    ImGui::Begin("TabCtrl", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar);
    
    ImGui::Text("Textures Loaded: %d/%d", textureMap.size(), TEXTURE_CAP);



    ImGui::End();

    ImGui::SetNextWindowPos(ImVec2(0, 36));
    ImGui::SetNextWindowSize(ImVec2(350, g_d3dpp.BackBufferHeight - 36));

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
        ImGui::SetNextWindowSize(ImVec2(g_d3dpp.BackBufferWidth - 350, g_d3dpp.BackBufferHeight - 335));
        window_height = g_d3dpp.BackBufferHeight - 335;
    }
    else {
        ImGui::SetNextWindowSize(ImVec2(g_d3dpp.BackBufferWidth - 350, g_d3dpp.BackBufferHeight - 75));
        window_height = g_d3dpp.BackBufferHeight - 80;
    }
    
    if (showViewport == false) {
        ImGui::SetNextWindowBgAlpha(0.1f);
    }
    else {
        ImGui::SetNextWindowBgAlpha(1.0f);
    }

    ImGui::Begin("Editor", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);



	if (FileNode::selectedNode) {
		if (FileNode::selectedNode->nodeType == FileNodeTypes::BXM) {
            showViewport = true;
			ImGui::Text("- Binary XML");
			BxmFileNode* bxmNode = ((BxmFileNode*)FileNode::selectedNode);
			bxmNode->RenderXMLTree(bxmNode->baseNode);
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
                    ImGui::SetNextItemWidth(120.0f);
                    ImGui::SliderFloat("Rotation", &index, 0.0f, 360.0f);
                    ImGui::Checkbox("Spin Model?", &spinModel);

                    ImGui::SetNextItemWidth(240.0f);
                    ImGui::SliderFloat3("Position", cameraPos, -800.0f, 800.0f);
                    ImGui::EndTabItem();
                }

                ImGui::EndTabBar();
            }



        }

	}


    ImGui::End();


    
    if (cruelerLog) {
        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.1f, 0.1f, 0.1f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 1.0f, 1.0f, 0.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 1.0f, 1.0f, 0.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(1.0f, 1.0f, 1.0f, 0.0f));

        ImGui::SetNextWindowPos(ImVec2(350, g_d3dpp.BackBufferHeight - 300));
        ImGui::SetNextWindowSize(ImVec2(g_d3dpp.BackBufferWidth - 350, 300));
        ImGui::Begin("CruelerThanDAT Log", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
        
        if (ImGui::Button(ICON_CI_TERMINAL, ImVec2(25, 25))){
            cruelerLog = false;
        }
        ImGui::SameLine();
        
        if (ImGui::Button(ICON_CI_CLEAR_ALL, ImVec2(25, 25))) {
            CTDLog::Log::getInstance().logEntries.clear();
        }


        for (CTDLog::LogEntry* log : CTDLog::Log::getInstance().logEntries) {
            ImGui::TextColored(log->color, log->text.c_str());
        }

        ImGui::PopStyleColor(4);
    }
    else {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 1.0f, 1.0f, 0.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 1.0f, 1.0f, 0.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(1.0f, 1.0f, 1.0f, 0.0f));
        ImGui::SetNextWindowPos(ImVec2(350, g_d3dpp.BackBufferHeight - 40));
        ImGui::SetNextWindowSize(ImVec2(g_d3dpp.BackBufferWidth - 350, 40));
        ImGui::Begin("CruelerThanDAT Log", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar);
        if (ImGui::Button(ICON_CI_TERMINAL, ImVec2(25, 25))) {
            cruelerLog = true;
        }
        ImGui::SameLine();
        ImGui::Text("CruelerThanDAT Log");

        ImGui::PopStyleColor(3);
    }


    

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

    D3DXVECTOR3 eyePos(cameraPos[0], cameraPos[1], cameraPos[2]);
    D3DXVECTOR3 lookAt(0.0f, 0.0f, 0.0f);
    D3DXVECTOR3 upDir(0.0f, 1.0f, 0.0f);

    D3DXMatrixLookAtLH(&matView, &eyePos, &lookAt, &upDir);

    g_pd3dDevice->SetTransform(D3DTS_VIEW, &matView);    // set the view transform to matView

    D3DXMATRIX matProjection;     // the projection transform matrix

    D3DXMatrixPerspectiveFovLH(&matProjection,
        D3DXToRadian(fov),    // the horizontal field of view
        (FLOAT)1280 / (FLOAT)720, // aspect ratio
        1.0f,    // the near view-plane
        5000.0f);    // the far view-plane

    g_pd3dDevice->SetTransform(D3DTS_PROJECTION, &matProjection);
    



    ImGui::EndFrame();
    g_pd3dDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);
    D3DCOLOR clear_col_dx = D3DCOLOR_RGBA((int)(clear_color.x * clear_color.w * 255.0f), (int)(clear_color.y * clear_color.w * 255.0f), (int)(clear_color.z * clear_color.w * 255.0f), (int)(clear_color.w * 255.0f));
    g_pd3dDevice->Clear(0, nullptr, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, clear_col_dx, 1.0f, 0);
    g_pd3dDevice->Clear(0, NULL, D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);
    if (g_pd3dDevice->BeginScene() >= 0)
    {
        g_pd3dDevice->SetRenderState(D3DRS_POINTSIZE, 5.0f);
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





int main()
{



    themeManager = new ThemeManager();
    themeManager->UpdateThemeList();
    

    appConfig.Read();

    printf("D3DInit...");

    WNDCLASSEXW wc = { sizeof(wc), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr, L"ImGui Example", nullptr };
    ::RegisterClassExW(&wc);
    HWND hwnd = ::CreateWindowW(wc.lpszClassName, L"CruelerThanDAT (DirectX 9)", WS_OVERLAPPEDWINDOW, 100, 100, 1600, 900, nullptr, nullptr, wc.hInstance, nullptr);


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


    CTDLog::Log::getInstance().LogNote("CruelerThanDAT Ready");
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
            HRESULT hr = g_pd3dDevice->TestCooperativeLevel();
            if (hr == D3DERR_DEVICELOST)
            {
                ::Sleep(10);
                continue;
            }
            if (hr == D3DERR_DEVICENOTRESET)
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

    }

}


