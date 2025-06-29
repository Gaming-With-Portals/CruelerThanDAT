#include "pch.hpp"

#define NOMINMAX
#define CURL_STATICLIB
#define SDL_MAIN_USE_CALLBACKS 1  

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include <glad/glad.h>
#include <glm/glm.hpp>

#include <imgui_impl_opengl3.h>
#include <imgui_impl_sdl3.h>
#include "gli/gli.hpp"

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

ImVec4 clear_color = ImVec4(0.0f, 0.0f, 0.0f, 1.00f);
static bool                     g_DeviceLost = false;
static UINT                     g_ResizeWidth = 0, g_ResizeHeight = 0;
static bool hasReset = false;
unsigned int fbo, colorTex, depthRbo;
int last_fbo_res_x, last_fbo_res_y;

static SDL_Window* window = nullptr;

HWND hwnd;

static int RES_X = 1600;
static int RES_Y = 900;
static float globalProgress = 0.0f;

// TODO: Delete
static glm::mat4 projMatrix;

float yaw = 0.0f;
float pitch = 0.0f;
float radius = 10.0f;

glm::vec3 target = glm::vec3(0.f);
glm::vec3 cameraPos = glm::vec3(0.f);
glm::mat4 view = glm::mat4(1.f);

static CTDSettings appConfig;

bool hasHandledArguments = false;
bool showViewport = true;

std::string downloadURL = "";
static std::unordered_map<unsigned int, unsigned int> textureMap;
//static LPDIRECT3DTEXTURE9 applicationIcon; // TODO: Replace by some GL or SDL thing
static std::unordered_map<unsigned int, std::vector<char>> rawTextureInfo;

ThemeManager* themeManager;
CPKManager* cpkManager;

bool dragging = false;
int drag_offset_x = 0;
int drag_offset_y = 0;

bool showAllSCRMeshes = false;
bool cruelerLog = true;

namespace HelperFunction {
	FileNode* LoadNode(std::string fileName, const std::vector<char>& data, bool forceEndianess, bool bigEndian) {
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

void CreateFramebuffer(int res_x, int res_y) {
	last_fbo_res_x = res_x;
	last_fbo_res_y = res_y;

	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);

	glGenTextures(1, &colorTex);
	glBindTexture(GL_TEXTURE_2D, colorTex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, res_x, res_y, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTex, 0);

	glGenRenderbuffers(1, &depthRbo);
	glBindRenderbuffer(GL_RENDERBUFFER, depthRbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, res_x, res_y);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, depthRbo);


	glBindFramebuffer(GL_FRAMEBUFFER, 0);
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
		WTP.Seek(offsets[i]);
		std::vector<char> data = WTP.ReadBytes(sizes[i]);
		gli::texture tex = gli::load(data.data(), data.size());
		if (tex.empty()) {
			CTDLog::Log::getInstance().LogError("Failed to load texture");
		}
		else {
			unsigned int textureID;
			glGenTextures(1, &textureID);

			gli::gl GL(gli::gl::PROFILE_GL33);
			gli::gl::format const Format = GL.translate(tex.format(), tex.swizzles());
			GLenum target = GL.translate(tex.target());

			glBindTexture(target, textureID);

			for (std::size_t Level = 0; Level < tex.levels(); ++Level) {
				glm::tvec3<GLsizei> extent(tex.extent(Level));
				glCompressedTexImage2D(target, static_cast<GLint>(Level), Format.Internal, extent.x, extent.y, 0, static_cast<GLsizei>(tex.size(Level)), tex.data(0, 0, Level));
			}

			glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			rawTextureInfo[idx[i]] = data;
			textureMap[idx[i]] = textureID;
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
						return;
					}

				}
				else if (dtnode->fileExtension == "wtb") {
					BinaryReader wta = BinaryReader(dtnode->fileData);
					BinaryReader wtp = BinaryReader(dtnode->fileData);
					wta.SetEndianess(node->fileIsBigEndian);

					DX9WTAWTPLoad(wta, wtp);
					return;
				}
			}
			for (FileNode* dtnode : node->children) {
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
						return;
					}

				}
				else if (dtnode->fileExtension == "wtb") {
					BinaryReader wta = BinaryReader(dtnode->fileData);
					BinaryReader wtp = BinaryReader(dtnode->fileData);
					wta.SetEndianess(node->fileIsBigEndian);

					DX9WTAWTPLoad(wta, wtp);
					return;
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

SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event)
{
	if (event->type == SDL_EVENT_QUIT) {
		return SDL_APP_SUCCESS;  /* end the program, reporting success to the OS. */
	}
	return SDL_APP_CONTINUE;  /* carry on with the program! */
}

SDL_AppResult SDL_AppIterate(void* appstate)
{

	return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void* appstate, SDL_AppResult result)
{

}

void RenderFrame() {



	static float fov = 50.0f;
	static float index = 180.0f;
	//static float cameraPos[3] = { 0.0f, 0.0f, -15.0f };
	static float cameraVec[3] = { -1.0f, 0.2f, 0.0f };
	(void)cameraVec;
	static bool spinModel = false;

	// Start the ImGui frame
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplSDL3_NewFrame();
	ImGui::NewFrame();
	glViewport(0, 0, (int)RES_X, (int)RES_Y);
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	ImGui::SetNextWindowPos(ImVec2(0, 0));
	ImGui::SetNextWindowSize(ImVec2(static_cast<float>(RES_X), 26));

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

	if (ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
		ReleaseCapture();
		PostMessage(hwnd, WM_NCLBUTTONDOWN, HTCAPTION, 0);
	}

	//ImGui::Text("Textures Loaded: %zu/%d", textureMap.size(), TEXTURE_CAP);
	
	ImGui::Image((ImTextureID)1, ImVec2(16, 16)); // TODO: Figure out what's the correct thing for this
	ImGui::SameLine();
	ImGui::SetCursorPosY(9);
	ImGui::Text("CruelerThanDAT");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(240);
	ImGui::SetCursorPosY(-1);
	ImGui::SetCursorPosX(0);

	ImGui::ProgressBar(globalProgress, ImVec2(static_cast<float>(RES_X), 4.0f), "");

	float spacing = 4.0f;
	float btnW = 36.0f;
	float totalBtnWidth = (btnW * 2.0f) + spacing;

	ImGui::SameLine(ImGui::GetContentRegionAvail().x - totalBtnWidth);

	if (ImGui::Button("_", ImVec2(btnW, 0))) {
		ShowWindow(GetActiveWindow(), SW_MINIMIZE); 
	}
	ImGui::SameLine();

	if (ImGui::Button("X", ImVec2(btnW, 0))) {
		exit(0);
	}

	ImGui::End();
	
	ImGui::SetNextWindowPos(ImVec2(0, 36));
	ImGui::SetNextWindowSize(ImVec2(350, static_cast<float>(RES_Y) - 36));

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
		ImGui::SetNextWindowSize(ImVec2(static_cast<float>(RES_X) - 350, static_cast<float>(RES_Y) - 335));
		window_height = RES_Y - 335;
	}
	else {
		ImGui::SetNextWindowSize(ImVec2(static_cast<float>(RES_X) - 350, static_cast<float>(RES_Y) - 75));
		window_height = RES_Y - 80;
	}

	if (showViewport == false) {
		ImGui::SetNextWindowBgAlpha(0.1f);
	}
	else {
		ImGui::SetNextWindowBgAlpha(1.0f);
	}

	ImGui::Begin("Editor", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

	//LPDIRECT3DSURFACE9 g_OriginalBackBuffer = nullptr;
	//g_pd3dDevice->GetRenderTarget(0, &g_OriginalBackBuffer);
	//g_pd3dDevice->SetRenderTarget(0, g_RenderTargetSurface);
	//g_pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,
	//	D3DCOLOR_XRGB(50, 50, 50), 1.0f, 0);

	if (FileNode::selectedNode) {
		if (FileNode::selectedNode->nodeType == FileNodeTypes::BXM) {
			showViewport = true;
			BxmFileNode* bxmNode = ((BxmFileNode*)FileNode::selectedNode);
			bxmNode->RenderGUI();
		}
		else if (FileNode::selectedNode->nodeType == FileNodeTypes::WTB) {
			showViewport = true;
			ImGui::Text("- Textures");
			WtbFileNode* wtbNode = ((WtbFileNode*)FileNode::selectedNode);
			if (ImGui::BeginListBox("##texturebox", ImVec2{ (float)RES_X - 350.0f, (float)window_height })) {
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
		else if (FileNode::selectedNode->nodeType == FileNodeTypes::BNK) {
			BnkFileNode* bnkNode = ((BnkFileNode*)FileNode::selectedNode);
			bnkNode->RenderGUI();
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
					ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.1f, 0.1f, 0.1f, 0.0f));
					ImGui::BeginChild("BoneSidebar", ImVec2(325, 0));
					if (ImGui::Button("Fetch Textures")) {

						PopulateTextures();
					}


					int i = 0;
					for (CTDMaterial& mat : wmbNode->materials) {

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

					ImGui::EndChild();
					ImGui::PopStyleColor();

					ImGui::EndTabItem();
				}
				if (!wmbNode->isSCR) {
					if (ImGui::BeginTabItem("Skeleton")) {

						wmbNode->RenderBoneGUI();


						ImGui::EndTabItem();
					}
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
					//ImGui::Checkbox("Spin Model?", &spinModel);
					ImGui::EndTabItem(); 
				}

				ImGui::EndTabBar();
			}



		}

	}

	ImVec2 pos = ImGui::GetWindowPos();
	ImVec2 size = ImGui::GetWindowSize();
	ImGui::GetBackgroundDrawList()->AddImage(
		(ImTextureID)(uintptr_t)colorTex, pos, ImVec2(pos.x + size.x, pos.y + size.y), ImVec2(0, 1), ImVec2(1, 0));
	if (last_fbo_res_x != size.x || last_fbo_res_y != size.y) {
		glDeleteTextures(1, &colorTex);
		glDeleteRenderbuffers(1, &depthRbo);
		glDeleteFramebuffers(1, &fbo);
		CreateFramebuffer(size.x, size.y);
		CameraManager::Instance().SetWindowSize(size.x, size.y);
		CameraManager::Instance().SetWindowPos(pos.x, pos.y);
	}

	ImGui::End();



	if (cruelerLog) {
		ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.1f, 0.1f, 0.1f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 1.0f, 1.0f, 0.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 1.0f, 1.0f, 0.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(1.0f, 1.0f, 1.0f, 0.0f));

		ImGui::SetNextWindowPos(ImVec2(350, static_cast<float>(RES_Y) - 300));
		ImGui::SetNextWindowSize(ImVec2(static_cast<float>(RES_X) - 350, 300));
		ImGui::Begin("CruelerThanDAT Log", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

		if (ImGui::Button(ICON_CI_TERMINAL, ImVec2(25, 25))) {
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
		ImGui::SetNextWindowPos(ImVec2(350, static_cast<float>(RES_Y) - 40));
		ImGui::SetNextWindowSize(ImVec2(static_cast<float>(RES_X) - 350, 40));
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

	// tell Direct3D about our matrix
	//g_pd3dDevice->SetTransform(D3DTS_WORLD, &matRotateY);

	
	#define CRUEL_PI 3.14159265f // TODO: Move this somewhere that makes sense
	pitch = std::max(-CRUEL_PI * 0.49f, std::min(CRUEL_PI * 0.49f, pitch));

	float x = radius * cosf(pitch) * sinf(yaw);
	float y = radius * sinf(pitch);
	float z = radius * cosf(pitch) * cosf(yaw);
	cameraPos = glm::vec3(x, y, z) + target;

	glm::vec3 up(0.0f, 1.0f, 0.0f);
	view = glm::lookAt(cameraPos, target, up);
	//g_pd3dDevice->SetTransform(D3DTS_VIEW, &view);

	glm::mat4 matProjection = glm::perspective(glm::radians(fov),
		static_cast<float>(size.x) / static_cast<float>(size.y),
		.1f, 5000.f);


	//g_pd3dDevice->SetTransform(D3DTS_PROJECTION, &matProjection);





	//g_pd3dDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);
	//D3DCOLOR clear_col_dx = glm::vec4((int)(clear_color.x * clear_color.w * 255.0f), (int)(clear_color.y * clear_color.w * 255.0f), (int)(clear_color.z * clear_color.w * 255.0f), (int)(clear_color.w * 255.0f));
	//g_pd3dDevice->Clear(0, nullptr, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, clear_col_dx, 1.0f, 0);
	//g_pd3dDevice->Clear(0, NULL, D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);

	//g_pd3dDevice->SetRenderState(D3DRS_COLORWRITEENABLE,
	//	D3DCOLORWRITEENABLE_RED |
	//	D3DCOLORWRITEENABLE_GREEN |
	//	D3DCOLORWRITEENABLE_BLUE);

	const float pointSize = 5.0f;
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glViewport(0, 0, last_fbo_res_x, last_fbo_res_y);
	glEnable(GL_DEPTH_TEST);

	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if (FileNode::selectedNode && FileNode::selectedNode->nodeType == FileNodeTypes::WMB) {
		WmbFileNode* wmbNode = ((WmbFileNode*)FileNode::selectedNode);
		//g_pd3dDevice->SetTexture(0, textureMap[0]);

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
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	ImGui::EndFrame();

	ImGui::Render();

	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	SDL_GL_SwapWindow(window);



}


SDL_AppResult SDL_AppInit(void** appstate, int argc, char* argv[])
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
	CTDLog::Log::getInstance().LogNote("Waiting for OpenGL...");

	SDL_Init(SDL_INIT_VIDEO);

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

	window = SDL_CreateWindow("CruelerThanDAT",
		RES_X, RES_Y, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_BORDERLESS);

	SDL_GLContext context = SDL_GL_CreateContext(window);
	SDL_GL_MakeCurrent(window, context);
	SDL_GL_SetSwapInterval(1);

	if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) {
		return SDL_APP_FAILURE;
	}

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();

	ImGui_ImplSDL3_InitForOpenGL(window, context);
	ImGui_ImplOpenGL3_Init("#version 330 core");

	// Load Fonts
	io.Fonts->AddFontDefault();
	

	ImFontConfig config;
	config.MergeMode = true;
	config.GlyphMinAdvanceX = 20;
	config.GlyphOffset = ImVec2(0, 5);
	static const ImWchar icon_ranges[] = { ICON_MIN_CI, ICON_MAX_CI, 0 };
	io.Fonts->AddFontFromFileTTF("Assets/codicon.ttf", 18, &config, icon_ranges);
	io.Fonts->Build();
	std::ifstream infile("Assets/codicon.ttf");
	if (!infile.good()) {
		CTDLog::Log::getInstance().LogError("Unable to locate Assets/codicons.ttf, icons will be broken!");
	}
	infile.close();

#ifdef _DEBUG

	CTDLog::Log::getInstance().LogWarning("CruelerThanDAT is running with DEBUG enabled, if this is a published build, please contact GamingWithPortals");
#endif



	themeManager->ChooseStyle(1);
	ImGui::GetStyle().WindowMinSize = ImVec2(32, 32);


	if (appConfig.ShowAllMeshesByDefault) {
		showAllSCRMeshes = true;
	}

	::ShowWindow(::GetConsoleWindow(), SW_HIDE);


	CTDLog::Log::getInstance().LogNote("CruelerThanDAT Ready");

	SelfUpdate();
	CreateFramebuffer(32, 32);
	bool done = false;
	while (!done)
	{
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			ImGui_ImplSDL3_ProcessEvent(&event);
			if (event.type == SDL_EVENT_QUIT)
				done = true;
			switch (event.type) {
				case SDL_EVENT_DROP_FILE:
				{
					std::string droppedPath = event.drop.data;
					openFiles.push_back(FileNodeFromFilepath(droppedPath));
					if (appConfig.AutomaticallyLoadTextures) {
						PopulateTextures();
					}
					break;
				}
				case SDL_EVENT_MOUSE_BUTTON_DOWN:
					if (event.button.button == SDL_BUTTON_LEFT) {
						int x = event.button.x;
						int y = event.button.y;

						if (y >= 0 && y <= 30) {
							dragging = true;

							int win_x, win_y;
							SDL_GetWindowPosition(window, &win_x, &win_y);
							drag_offset_x = x;
							drag_offset_y = y;
						}
					}
					break;

				case SDL_EVENT_MOUSE_BUTTON_UP:
					if (event.button.button == SDL_BUTTON_LEFT) {
						dragging = false;
					}
					break;

				case SDL_EVENT_MOUSE_MOTION:
					if (dragging) {
						float global_x = event.motion.x;
						float global_y = event.motion.y;

						int win_x, win_y;
						SDL_GetGlobalMouseState(&global_x, &global_y); 
						SDL_SetWindowPosition(window, global_x - drag_offset_x, global_y - drag_offset_y);
					}
					break;
			}

			CameraManager::Instance().Input(&event);

		}
		if (done)
			break;

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
	return SDL_APP_CONTINUE;
}

