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
#include "Core/FileNodes.h"
#include <vector>
#include <fstream>
#include <memory>
#include "Assets/CodIcons.h"
#include <shellapi.h>
#include <chrono>
#include "Core/Log.h"
#include "Themes/themeLoader.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
ImVec4 clear_color = ImVec4(0.0f, 0.0f, 0.0f, 1.00f);
static LPDIRECT3D9              g_pD3D = nullptr;
static LPDIRECT3DDEVICE9        g_pd3dDevice = nullptr;
static bool                     g_DeviceLost = false;
static UINT                     g_ResizeWidth = 0, g_ResizeHeight = 0;
static D3DPRESENT_PARAMETERS    g_d3dpp = {};
bool showViewport = true;
std::vector<FileNode*> openFiles;
ThemeManager* themeManager;

bool cruelerLog = true;

namespace HelperFunction {
    FileNode* LoadNode(std::string fileName, const std::vector<char>& data, bool forceEndianess , bool bigEndian) {
        std::string fileExtension = fileName.substr(fileName.find_last_of("."));
        uint32_t fileType = *reinterpret_cast<const uint32_t*>(&data[0]);
        FileNode* outputFile = nullptr;



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



bool ReadFileIntoVector(const std::string& filePath, std::vector<char>& data) {
    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filePath << std::endl;
        return false;
    }

    // Get file size and resize the vector to hold the file data
    file.seekg(0, std::ios::end);
    size_t fileSize = file.tellg();
    file.seekg(0, std::ios::beg);
    data.resize(fileSize);

    // Read the file into the vector
    file.read(reinterpret_cast<char*>(data.data()), fileSize);
    return true;
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
    // Start the ImGui frame
    ImGui_ImplDX9_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2(g_d3dpp.BackBufferWidth, 36));

    ImGui::Begin("TabCtrl", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar);

    if (ImGui::Button("Load")) {

    }
    ImGui::SameLine();
    if (ImGui::Button("ImGui Theme")) {
        themeManager->ChooseStyle(0);
    }
    ImGui::SameLine();
    if (ImGui::Button("Visual Studio Theme")) {
        themeManager->ChooseStyle(1);
    }
    ImGui::SameLine();
    if (ImGui::Button("Half Life Theme")) {
        themeManager->ChooseStyle(2);
    }


    ImGui::End();

    ImGui::SetNextWindowPos(ImVec2(0, 36));
    ImGui::SetNextWindowSize(ImVec2(350, g_d3dpp.BackBufferHeight - 36));

    ImGui::Begin("DatView", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);

    if (ImGui::BeginTabBar("primary_control")) {
        
        if (ImGui::BeginTabItem("Data Viewer")) {
            for (FileNode* node : openFiles) {
                node->Render();
            }

            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("CPK Viewer")) {

            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Configuration")) {
            ImGui::Text("Theme");


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
    


    ImGui::EndFrame();
    g_pd3dDevice->SetRenderState(D3DRS_ZENABLE, FALSE);
    g_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
    g_pd3dDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);
    D3DCOLOR clear_col_dx = D3DCOLOR_RGBA((int)(clear_color.x * clear_color.w * 255.0f), (int)(clear_color.y * clear_color.w * 255.0f), (int)(clear_color.z * clear_color.w * 255.0f), (int)(clear_color.w * 255.0f));
    g_pd3dDevice->Clear(0, nullptr, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, clear_col_dx, 1.0f, 0);
    if (g_pd3dDevice->BeginScene() >= 0)
    {
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
    //::ShowWindow(::GetConsoleWindow(), SW_HIDE);

    themeManager = new ThemeManager();
    themeManager->UpdateThemeList();
    

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


