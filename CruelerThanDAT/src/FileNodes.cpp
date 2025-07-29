#include "pch.hpp"
#include "FileNodes.h"

#include "CTDSettings.h"
#include "globals.h"
#include "tinyxml2.h"
#include "BinaryHandler.h"
#include "CodIcons.h"
#include "Log.h"
#include "CRC32.h"
#include "HashDataContainer.h"
#include "ImGuiExtended.h"
#include "TextEditor.h"
#include "WMB.h"
#include "CTDModel.h"
#include "UID.h"
#include "UVD.h"
#include <Wwise/wwise.h>
#include <glad/glad.h>
#include "shader.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <Camera.h>

const FbxSystemUnit fbxsdk::FbxSystemUnit::m(100.0);

FileNode* FileNode::selectedNode = nullptr;

void closeNode(FileNode* target) {
	FileNode::selectedNode = nullptr;
    auto it = std::find(openFiles.begin(), openFiles.end(), target);
    if (it != openFiles.end()) { // Ensure it exists before erasing
        openFiles.erase(it);
    }

	delete target;
}


int IntLength(int value) {
	int length = 0;
	while (value > 0) {
		value >>= 1;
		length++;
	}
	return length;
}


bool HelperFunction::WriteVectorToFile(const std::vector<char> dataVec, const std::string& filename) {
	std::string fullPath = filename;

	std::ofstream out(fullPath, std::ios::binary);
	if (!out) return false;

	out.write(dataVec.data(), dataVec.size());
	return out.good();
}

bool HelperFunction::WorldToScreen(const glm::vec3& worldPos, glm::vec2& screenPos, const glm::mat4& view, const glm::mat4& proj, const GLint* viewport)
{
	glm::vec4 clipSpacePos = proj * view * glm::vec4(worldPos, 1.0f);

	if (clipSpacePos.w <= 0.0f)
		return false;

	glm::vec3 ndc = glm::vec3(clipSpacePos) / clipSpacePos.w;

	screenPos.x = (ndc.x * 0.5f + 0.5f) * viewport[2] + viewport[0];
	screenPos.y = (1.0f - (ndc.y * 0.5f + 0.5f)) * viewport[3] + viewport[1];

	return true;
}

std::vector<std::string> BXMInternal::SplitString(const std::string& str, char delimiter) {
	std::vector<std::string> result;
	std::string current;

	for (char c : str) {
		if (c == delimiter) {
			if (!current.empty()) {
				result.push_back(current);
				current.clear();
			}
		}
		else {
			current += c;
		}
	}
	if (!current.empty()) {
		result.push_back(current);
	}

	return result;
}

int HelperFunction::Align(int value, int alignment) {
	return (value + (alignment - 1)) & ~(alignment - 1);
}

glm::vec4 HelperFunction::DecodeTangent(uint32_t packedTangent) {

	uint8_t tx = (packedTangent >> 0) & 0xFF;
	uint8_t ty = (packedTangent >> 8) & 0xFF;
	uint8_t tz = (packedTangent >> 16) & 0xFF;
	uint8_t tw = (packedTangent >> 24) & 0xFF;

	auto decodeByte = [](uint8_t v) -> float {
		return (static_cast<float>(v) - 127.0f) / 127.0f;
		};

	glm::vec4 tangent;
	tangent.x = decodeByte(tx);
	tangent.y = decodeByte(ty);
	tangent.z = decodeByte(tz);
	tangent.w = decodeByte(tw);

	return tangent;
}

MGRVector HelperFunction::DecodeNormal(uint32_t packed) {
	int nx = packed & ((1 << 11) - 1);
	int ny = (packed >> 11) & ((1 << 11) - 1);
	int nz = (packed >> 22) & ((1 << 10) - 1);

	if (nx & (1 << 10)) nx = -(int)((1 << 11) - nx);
	if (ny & (1 << 10)) ny = -(int)((1 << 11) - ny);
	if (nz & (1 << 9))  nz = -(int)((1 << 10) - nz);

	float fx = (float)nx / ((1 << 10) - 1);
	float fy = (float)ny / ((1 << 10) - 1);
	float fz = (float)nz / ((1 << 9) - 1);

	float len = sqrtf(fx * fx + fy * fy + fz * fz);
	return { fx / len, fy / len, fz / len };
}

float HelperFunction::HalfToFloat(uint16_t h) {
	uint16_t h_exp = (h & 0x7C00) >> 10;  // exponent
	uint16_t h_sig = h & 0x03FF;          // significand
	uint32_t f_sgn = (h & 0x8000) << 16;  // sign bit

	uint32_t f_exp, f_sig;

	if (h_exp == 0) {
		// Zero / Subnormal
		if (h_sig == 0) {
			f_exp = 0;
			f_sig = 0;
		}
		else {
			// Normalize it
			h_exp = 1;
			while ((h_sig & 0x0400) == 0) {
				h_sig <<= 1;
				h_exp--;
			}
			h_sig &= 0x03FF;
			f_exp = (127 - 15 + h_exp) << 23;
			f_sig = h_sig << 13;
		}
	}
	else if (h_exp == 0x1F) {
		// Inf / NaN
		f_exp = 0xFF << 23;
		f_sig = h_sig << 13;
	}
	else {
		f_exp = (h_exp - 15 + 127) << 23;
		f_sig = h_sig << 13;
	}

	uint32_t f = f_sgn | f_exp | f_sig;
	float result;
	std::memcpy(&result, &f, sizeof(result));
	return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

const std::vector<std::string> BXMInternal::possibleParams = {
	"RoomNo",
	"PlayerPos",
	"isRestartPoint",
	"CameraYaw",
	"CameraEnable",
	"CameraXEnable",
	"CameraYEnable",
	"isPlWaitPayment",
};

FileNode::FileNode(std::string fName) {
	fileName = fName;
	fileExtension = fileName.substr(fileName.find_last_of(".") + 1);
}

FileNode::~FileNode() {

}

void FileNode::PopupOptionsEx() {

}

void FileNode::ExportFile() {
	OPENFILENAME ofn;
	wchar_t szFile[260] = { 0 };
	LPWSTR pFile = szFile;

	mbstowcs_s(0, pFile, fileName.length() + 1, fileName.c_str(), _TRUNCATE);

	ZeroMemory(&ofn, sizeof(OPENFILENAME));

	ofn.lpstrFile = pFile;
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = NULL;
	ofn.nMaxFile = 260;
	ofn.lpstrFilter = fileFilter;

	ofn.nFilterIndex = 1;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;

	if (GetSaveFileName(&ofn) == TRUE) {
		std::ofstream outputFile(ofn.lpstrFile, std::ios::binary); // Open in binary mode!
		if (outputFile.is_open()) {
			SaveFile();
			outputFile.write(fileData.data(), fileData.size());
		}

	}
	else {
		std::cout << "Save operation cancelled." << std::endl;
	}

	ImGui::CloseCurrentPopup();

}

void FileNode::ReplaceFile() {
	OPENFILENAME ofn;
	wchar_t szFile[260] = { 0 };
	LPWSTR pFile = szFile;

	mbstowcs_s(0, pFile, fileName.length() + 1, fileName.c_str(), _TRUNCATE);

	ZeroMemory(&ofn, sizeof(OPENFILENAME));

	ofn.lpstrFile = pFile;
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = NULL;
	ofn.nMaxFile = 260;
	ofn.lpstrFilter = fileFilter;

	ofn.nFilterIndex = 1;
	ofn.Flags = OFN_PATHMUSTEXIST;

	if (GetOpenFileName(&ofn) == TRUE) {
		std::ifstream file(ofn.lpstrFile, std::ios::binary | std::ios::ate); 
		if (file.is_open()) {
			fileData.clear();
			std::streamsize size = file.tellg(); 
			fileData.resize(size);         
			file.seekg(0, std::ios::beg);     
			file.read(fileData.data(), size);
			file.close();
			std::cout << "Replaced file! " << std::endl;
		}
		else {
			std::cout << "Error" << std::endl;
		}

	}

	ImGui::CloseCurrentPopup();

	LoadFile();

}

void FileNode::PopupOptions(CruelerContext *ctx) {
	auto it = std::find(openFiles.begin(), openFiles.end(), this);
	if (ImGui::BeginPopupContextItem())
	{
		if (ImGui::Button("Export", ImVec2(150, 20))) {
			ExportFile();
		}

		if (it != openFiles.end()) { // Ensure it exists before erasing
			if (ImGui::Button("Close", ImVec2(150, 20))) {
				closeNode(this);
			}
		}

		else {
			if (ImGui::Button("Replace", ImVec2(150, 20))) {
				ReplaceFile();
				isEdited = true;
			}
			if (parent) {
				if (ImGui::Button("Delete", ImVec2(150, 20))) {

					auto it = std::find(parent->children.begin(), parent->children.end(), this);
					if (it != parent->children.end()) {
						parent->children.erase(it);
					}

					delete this;
				}


			}
		}



		//PopupOptionsEx();

		ImGui::EndPopup();
	}
}


void FileNode::Render(CruelerContext *ctx) {
	std::string displayName = fileIcon + fileName;



	if (fileIsBigEndian) {
		displayName += " (X360/PS3)";
	}
	if (children.size() == 0) {

		ImGui::PushStyleColor(ImGuiCol_Text, TextColor);
		if (ImGui::TreeNodeEx(displayName.c_str(), ImGuiTreeNodeFlags_Leaf)) {
			PopupOptions(ctx);

			if (ImGui::IsItemClicked()) {
				selectedNode = this; // Update selected node
			}



			ImGui::TreePop();
		}
		ImGui::PopStyleColor();
	}
	else {
		ImGui::PushStyleColor(ImGuiCol_Text, TextColor);



		if (ImGui::TreeNode(displayName.c_str())) {

			PopupOptions(ctx);

			if (ImGui::IsItemClicked()) {
				selectedNode = this; // Update selected node
			}
			ImGui::PopStyleColor();
			for (FileNode* node : children) {
				node->Render(ctx);
			}

			ImGui::TreePop();
		}
		else {
			ImGui::PopStyleColor();
		}
	}

}

void FileNode::SetFileData(const std::vector<char>& data) {
	fileData = data;
}

const std::vector<char>& FileNode::GetFileData() const {
	return fileData;
}

UnkFileNode::UnkFileNode(std::string fName) : FileNode(fName) {
}

void UnkFileNode::LoadFile() {
}

void UnkFileNode::SaveFile() {
}

LY2FileNode::LY2FileNode(std::string fName) : FileNode(fName) {
	fileIcon = ICON_CI_ARRAY;
	TextColor = { 1.0f, 0.0f, 0.376f, 1.0f };
	nodeType = LY2;
	fileFilter = L"Layout Files (*.ly2)\0*.ly2;\0";
	ly2Flags = 0;
	propTypeCount = 0;
	mysteryPointer = 0;
	mysteryCount = 0;
}

void LY2FileNode::LoadFile() {
	BinaryReader reader(fileData, true);
	reader.SetEndianess(fileIsBigEndian);
	reader.Seek(0x4);
	ly2Flags = reader.ReadUINT32();
	propTypeCount = reader.ReadUINT32();
	mysteryPointer = reader.ReadUINT32();
	mysteryCount = reader.ReadUINT32();

	for (int x = 0; x < propTypeCount; x++) {
		LY2Node node = LY2Node();
		node.flag_a = reader.ReadUINT32();
		node.flag_b = reader.ReadUINT32();
		node.prop_category = reader.ReadString(2);
		node.prop_id = reader.ReadUINT16();
		node.offset = reader.ReadUINT32();
		node.count = reader.ReadUINT32();

		size_t pos = reader.Tell();
		reader.Seek(node.offset);
		for (int y = 0; y < node.count; y++) {
			LY2Instance instance = LY2Instance();
			instance.pos[0] = reader.ReadFloat();
			instance.pos[1] = reader.ReadFloat();
			instance.pos[2] = reader.ReadFloat();
			instance.scale[0] = reader.ReadFloat();
			instance.scale[1] = reader.ReadFloat();
			instance.scale[2] = reader.ReadFloat();
			instance.unknownB4C = reader.ReadUINT32();
			instance.unknownB4E = reader.ReadUINT32();
			instance.unknownB4F = reader.ReadUINT32();
			instance.unknownB4G = reader.ReadINT32();

			node.instances.push_back(instance);


		}
		nodes.push_back(node);
		reader.Seek(pos);

	}

	reader.Seek(mysteryPointer);
	for (int x = 0; x < mysteryCount; x++) {
		LY2ExData ex = LY2ExData();
		ex.a = reader.ReadUINT16();
		ex.b = reader.ReadUINT16();
		ex.c = reader.ReadUINT32();
		ex.d = reader.ReadUINT32();
		extradata.push_back(ex);
	}

}

void LY2FileNode::SaveFile() {
	if (!isEdited) {
		return;
	}
	BinaryWriter* writer = new BinaryWriter();

	writer->WriteString("LY2");
	writer->WriteByteZero();
	writer->WriteINT32(4);
	writer->WriteUINT32(static_cast<uint32_t>(nodes.size()));

	int extradataoffset = 20;
	int mainchunkend = 20;
	for (LY2Node& node : nodes) {
		extradataoffset += (20 + (40 * static_cast<int>(node.instances.size())));
		mainchunkend += 20;

	}
	writer->WriteUINT32(extradataoffset);
	writer->WriteUINT32(static_cast<uint32_t>(extradata.size()));
	int offset = 0;
	for (LY2Node& node : nodes) {
		writer->WriteUINT32(node.flag_a);
		writer->WriteUINT32(node.flag_b);
		writer->WriteString(node.prop_category);
		writer->WriteUINT16(node.prop_id);
		writer->WriteUINT32(mainchunkend + offset);
		writer->WriteUINT32(static_cast<uint32_t>(node.instances.size()));
		offset += 40 * static_cast<int>(node.instances.size());
	}
	for (LY2Node& node : nodes) {
		for (LY2Instance& inst : node.instances) {
			writer->WriteFloat(inst.pos[0]);
			writer->WriteFloat(inst.pos[1]);
			writer->WriteFloat(inst.pos[2]);
			writer->WriteFloat(inst.scale[0]);
			writer->WriteFloat(inst.scale[1]);
			writer->WriteFloat(inst.scale[2]);
			// TODO: Rotation encoding
			writer->WriteUINT32(inst.unknownB4C);
			writer->WriteUINT32(inst.unknownB4E);
			writer->WriteUINT32(inst.unknownB4F);
			writer->WriteINT32(inst.unknownB4G);
		}
	}
	for (LY2ExData& exData : extradata) {
		writer->WriteUINT16(exData.a);
		writer->WriteUINT16(exData.b);
		writer->WriteUINT32(exData.c);
		writer->WriteUINT32(exData.d);
	}

	fileData = writer->GetData();
}

void LY2FileNode::RenderGUI(CruelerContext *ctx) {
	if (ImGui::BeginTabBar("ly2_bar")) {
		if (ImGui::BeginTabItem("Object Editor")) {
			int i = 0;
			for (LY2Node& node : nodes) {
				for (int j = 0; j < node.instances.size();) {
					LY2Instance& instance = node.instances[j];
					ImGui::PushID(i);

					if (ImGui::CollapsingHeader((node.prop_category + std::format("{:04x}", node.prop_id)).c_str())) {
						isEdited = true;
						ImGui::InputFloat3("Position", instance.pos, "%.2f");
						ImGui::InputFloat3("Scale", instance.scale, "%.2f");
						//ImGui::InputFloat3("Rotation", instance.rot, "%.2f");

						if (ImGui::Button("-")) {
							node.instances.erase(node.instances.begin() + j);
							ImGui::PopID();
							continue;
						}
					}

					ImGui::PopID();
					j++;
					i += 1;
				}


			}
			ImGui::PushID(i + 1);
			ImGui::SetNextItemWidth(60.0f);
			ImGui::InputText("", input_id, 7);
			ImGui::SameLine();
			if (ImGui::Button("+")) {
				isEdited = true;
				std::string hexStr(input_id + 2, 4);
				LY2Node node = LY2Node();
				node.prop_id = static_cast<unsigned short>(std::stoi(hexStr, nullptr, 16));
				node.prop_category = std::string(input_id, 2);
				node.instances.push_back(LY2Instance());
				nodes.push_back(node);


			}
			ImGui::PopID();
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Group Editor")) {
			int i = 0;
			for (LY2Node& node : nodes) {
				if (node.instances.empty()) {
					continue; // Ignore
				}
				ImGui::PushID(i);
				if (ImGui::CollapsingHeader(("Group " + std::to_string(i) + " " + node.prop_category + std::format("{:04x}", node.prop_id)).c_str())) {
					ImGui::Text("Item Count %zu", node.instances.size());
					ImGui::InputInt("Flag A", &node.flag_a);
					ImGui::InputInt("Flag B", &node.flag_b);
				}
				ImGui::PopID();
				i += 1;
			}

			if (ImGui::Button("Optimize Groups")) {
				// Basically, every group with equal IDs and flags can be combined into one.
				for (size_t j = 0; j < nodes.size(); ++j) {
					if (nodes[j].instances.empty()) continue;

					for (size_t k = j + 1; k < nodes.size(); ++k) {
						if (nodes[k].flag_a == nodes[j].flag_a &&
							nodes[k].flag_b == nodes[j].flag_b &&
							nodes[k].prop_id == nodes[j].prop_id &&
							nodes[k].prop_category == nodes[j].prop_category) {

							nodes[j].instances.insert(nodes[j].instances.end(), nodes[k].instances.begin(), nodes[k].instances.end());

							nodes[k].instances.clear();
						}
					}
				}
			}
			ImGui::SameLine();
			HelpMarker("Combines groups with matching flags and IDs into one group to optimize the file");

			ImGui::EndTabItem();
		}

		ImGui::EndTabBar();
	}

}

	UvdFileNode::UvdFileNode(std::string fName) : FileNode(fName) {
		fileIcon = ICON_CI_LIBRARY;
		TextColor = { 1.0f, 0.0f, 0.376f, 1.0f };
		nodeType = UVD;
		fileFilter = L"UI Element File(*.uvd)\0*.uvd;\0";
	}
	void UvdFileNode::LoadFile() {
		BinaryReader reader(fileData, fileIsBigEndian);
		reader.Seek(0x4);
		uint32_t entriesCount = reader.ReadUINT32();
		uint32_t entriesOffset = reader.ReadUINT32();
		uint32_t texturesOffset = reader.ReadUINT32();
		uint32_t textureCount = (static_cast<uint32_t>(reader.GetSize()) - texturesOffset) / 36;

		reader.Seek(texturesOffset);
		for (uint32_t a = 0; a < textureCount; a++) {
			UvdTexture texture = UvdTexture();
			texture.name = reader.ReadString(32);
			texture.id = reader.ReadUINT32();
			uvdTextures.push_back(texture);
		}

		reader.Seek(entriesOffset);
		for (uint32_t a = 0; a < entriesCount; a++) {
			UvdEntry texture = UvdEntry();
			texture.Read(reader);
			uvdEntries.push_back(texture);
		}




	}

	void UvdFileNode::RenderGUI(CruelerContext *ctx) {
		if (ImGui::BeginTabBar("uvd_editor_bar")) {
			if (ImGui::BeginTabItem("UVD Entries")) {
				for (UvdEntry entry : uvdEntries) {
					if (ctx->textureMap.find(entry.textureID) != ctx->textureMap.end()) {

						GLint texWidth;
						GLint texHeight;

						glBindTexture(GL_TEXTURE_2D, ctx->textureMap[entry.textureID]);

						glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &texWidth);
						glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &texHeight);


						ImVec2 uv0(entry.x / (float)texWidth, entry.y / (float)texHeight);
						ImVec2 uv1((entry.x + entry.width) / (float)texWidth, (entry.y + entry.height) / (float)texHeight);
						ImGui::Image((ImTextureID)(intptr_t)ctx->textureMap[entry.textureID], ImVec2(64, 64), uv0, uv1);
					}
					else {
						ImGui::Image((ImTextureID)(intptr_t)ctx->textureMap[0], ImVec2(64, 64));
					}



					ImGui::SameLine();
					ImGui::Text("ID: %d", entry.ID);

					if (ImGui::TreeNode(entry.name.c_str())) {
						ImGui::TreePop();
					}

				}


				ImGui::EndTabItem();
			}


			ImGui::EndTabBar();
		}
	}

	void UvdFileNode::SaveFile() {

	}

	UidFileNode::UidFileNode(std::string fName) : FileNode(fName) {
		fileIcon = ICON_CI_EDITOR_LAYOUT;
		TextColor = { 1.0f, 0.0f, 0.376f, 1.0f };
		nodeType = UID;
		fileFilter = L"User Interface Description(*.uid)\0*.uid;\0";
	}
	void UidFileNode::LoadFile() {
		BinaryReader reader(fileData, fileIsBigEndian);

		uidHeader.Read(reader);

		if (uidHeader.offset1 > 0) {
			reader.Seek(uidHeader.offset1);
			for (int i = 0; i < uidHeader.size1; i++) {
				UIDEntry1 uid1;
				uid1.Read(reader);
				UIDEntry1List.push_back(uid1);
			}
			
			int endOffset = uidHeader.offset2;
			if (endOffset == 0)
				endOffset = uidHeader.offset3;
			if (endOffset == 0)
				endOffset = static_cast<int>(reader.GetSize());

			std::vector<int> allOffsets;
			for (UIDEntry1& entry : UIDEntry1List) {
				if (entry.data1Offset != 0) {
					allOffsets.push_back(entry.data1Offset);
				}
				if (entry.data2Offset != 0) {
					allOffsets.push_back(entry.data2Offset);
				}
				if (entry.data3Offset != 0) {
					allOffsets.push_back(entry.data3Offset);
				}
			}
			allOffsets.push_back(endOffset);

			std::unordered_map<int, int> offsetSizes;
			for (size_t i = 0; i + 1 < allOffsets.size(); ++i) {
				int offset = allOffsets[i];
				int size = allOffsets[i + 1] - offset;
				offsetSizes[offset] = size;
			}

			for (UIDEntry1& entry : UIDEntry1List) {
				entry.readAdditionalData(reader, offsetSizes);
			}

		}
		if (uidHeader.offset2 > 0) {
			reader.Seek(uidHeader.offset2);
			for (int i = 0; i < uidHeader.size2; i++) {
				UIDEntry2 uid2;
				uid2.Read(reader);
				UIDEntry2List.push_back(uid2);
			}

		}
		if (uidHeader.offset3 > 0) {
			reader.Seek(uidHeader.offset3);
			for (int i = 0; i < uidHeader.size3; i++) {
				UIDEntry3 uid3;
				uid3.Read(reader);
				UIDEntry3List.push_back(uid3);
			}

		}



	}
	void UidFileNode::SaveFile() {
		if (!isEdited) {
			return;
		}
		

		BinaryWriter* writer = new BinaryWriter(fileIsBigEndian);
		UIDHeader header;
		header.size1 = static_cast<int>(UIDEntry1List.size());
		header.size2 = static_cast<int>(UIDEntry2List.size());
		header.size3 = static_cast<int>(UIDEntry3List.size());
		header.u0 = uidHeader.u0;
		header.u1 = uidHeader.u1;
		header.frameLength = uidHeader.frameLength;
		header.width = uidHeader.width;
		header.height = uidHeader.height;
		header.u2 = uidHeader.u2;

		header.offset1 = 36;


		
		std::vector<int> dataPositions;
		int pointer = 36 + (432 * static_cast<int>(UIDEntry1List.size()));
		for (UIDEntry1& uid : UIDEntry1List) {
			if (uid.data1.data.size() > 0) {
				dataPositions.push_back(pointer);
				pointer += static_cast<int>(uid.data1.data.size());
			}
			else {
				dataPositions.push_back(0);
			}
			if (uid.data2.data.size() > 0) {
				dataPositions.push_back(pointer);
				pointer += static_cast<int>(uid.data2.data.size());
			}
			else {
				dataPositions.push_back(0);
			}
			if (uid.data3.data.size() > 0) {
				dataPositions.push_back(pointer);
				pointer += static_cast<int>(uid.data3.data.size());
			}
			else {
				dataPositions.push_back(0);
			}


		}


		header.offset2 = pointer;
		header.offset3 = header.offset2 + (sizeof(UIDEntry2) * static_cast<int>(UIDEntry2List.size()));

		header.Write(writer);
		int i = 0;
		writer->Seek(header.offset1);
		for (UIDEntry1& uid : UIDEntry1List) {
			uid.data1Offset = dataPositions[i];
			i += 1;
			uid.data2Offset = dataPositions[i];
			i += 1;
			uid.data3Offset = dataPositions[i];
			i += 1;

			uid.Write(writer);
			
		}


		for (UIDEntry1& uid : UIDEntry1List) {
			uid.writeAdditionalData(writer);
		}

		writer->Seek(header.offset2);
		for (UIDEntry2 uid : UIDEntry2List) {
			uid.Write(writer);
		}

		writer->Seek(header.offset3);
		for (UIDEntry3 uid : UIDEntry3List) {
			uid.Write(writer);
		}

		fileData = writer->GetData();


	}

	void UidFileNode::RenderGUI(CruelerContext *ctx) {
		int i = 0;
		isEdited = true;
		if (parent) {
			for (FileNode* child : parent->children) {
				if (child->nodeType == UVD) {
					pairedUVD = (UvdFileNode*)child;
					break;
				}
			}
		}



		ImGui::Checkbox("Visaulize UID Positions", &UIDVisualize);


		ImGui::GetForegroundDrawList()->AddText(ImVec2(800.0f, 900.0f), IM_COL32(255.0f, 255.0f, 255.0f,255.0f), "UID Preview Canvas");
		ImGui::GetForegroundDrawList()->AddRectFilled(ImVec2(800.0f, 100.0f), ImVec2(1440.0f, 450.0f), IM_COL32(2.0f, 2.0f, 2.0f, 255.0f));
		for (UIDEntry1& entry : UIDEntry1List) {
			bool foundUVD = false;
			UvdEntry associatedUVDEntry;
			ImGui::PushID(i);
			if (entry.data1.dataType == Entry1Type::UIDrawImage && pairedUVD) {
				if (ctx->textureMap.find(entry.data1.dataStructure.img.textureID) != ctx->textureMap.end()) {
					for (UvdEntry uvdEntry : pairedUVD->uvdEntries) {
						if (uvdEntry.ID == entry.data1.dataStructure.img.uvdID) {
							associatedUVDEntry = uvdEntry;
							GLint texWidth;
							GLint texHeight;

							glBindTexture(GL_TEXTURE_2D, ctx->textureMap[uvdEntry.textureID]);

							glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &texWidth);
							glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &texHeight);
							ImVec2 uv0(uvdEntry.x / (float)texWidth, uvdEntry.y / (float)texHeight);
							ImVec2 uv1((uvdEntry.x + uvdEntry.width) / (float)texWidth, (uvdEntry.y + uvdEntry.height) / (float)texHeight);
						
							ImGui::Image((ImTextureID)(intptr_t)ctx->textureMap[entry.data1.dataStructure.img.textureID], ImVec2(17, 17), uv0, uv1); // ImVec4(entry.rgb.r, entry.rgb.g, entry.rgb.b, entry.rgb.a), Fuck you, ImGui, I liked that feature 
							ImGui::SameLine();
							foundUVD = true;
							break;
						}
					}


				}
			}
			if (!foundUVD) {
				ImGui::ColorEdit4("", entry.rgb, ImGuiColorEditFlags_NoInputs);
				ImGui::SameLine();
			}
			
			if (ImGui::TreeNode(("UID Entry: " + std::to_string(i) + " (" + MGRUI::Entry1TypeFriendlyFormatted[entry.data1Flag] + ")").c_str())) {
				ImGui::PushItemWidth(240.0f);
				ImGui::InputFloat3("Position", entry.position);
				ImGui::InputFloat3("Rotation", entry.rotation);
				ImGui::InputFloat3("Scale", entry.scale);
				ImGui::ColorEdit4("Color", entry.rgb);
				if (entry.data1.dataType == Entry1Type::UIDrawImage) {
					entry.data1.dataStructure.img.Render();
				}


				if (ImGui::TreeNode("Extra Data")) {
					if (entry.data3.data.size() > 0) {
						if (ImGui::TreeNode("Animation Data")) {
							if (ImGui::Button("Delete Animation Data")) {
								entry.data3.data.clear();
							}
							ImGui::TreePop();
						}
					}



					ImGui::TreePop();
				}

				ImGui::PopItemWidth();
				ImGui::TreePop();

				if (ImGui::Button("Duplicate")) {
					UIDEntry1List.push_back(entry);
				}
			}


			ImGui::PopID();
			
			if (UIDVisualize) {
				float scalex = entry.position.x * (640.0f / 1280.0f) + 800.0f;
				float scaley = entry.position.y * (360.0f / 720.0f) + 100.0f;

				if (foundUVD) {
					GLint texWidth;
					GLint texHeight;

					glBindTexture(GL_TEXTURE_2D, ctx->textureMap[associatedUVDEntry.textureID]);

					glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &texWidth);
					glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &texHeight);
					ImVec2 uv0(associatedUVDEntry.x / (float)texWidth, associatedUVDEntry.y / (float)texHeight);
					ImVec2 uv1((associatedUVDEntry.x + associatedUVDEntry.width) / (float)texWidth, (associatedUVDEntry.y + associatedUVDEntry.height) / (float)texHeight);
					ImVec2 screenPos = ImVec2(scalex, scaley); // top-left position
					ImVec2 screenEnd = ImVec2(screenPos.x + (associatedUVDEntry.width * entry.scale.x), screenPos.y + (associatedUVDEntry.height * entry.scale.y));
					foundUVD = true;
					ImGui::GetForegroundDrawList()->AddImage((ImTextureID)(intptr_t)ctx->textureMap[entry.data1.dataStructure.img.textureID], screenPos, screenEnd, uv0, uv1, IM_COL32_WHITE);
				}
				else {
					if (false) {
						ImGui::GetForegroundDrawList()->AddText(ImVec2(scalex, scaley), IM_COL32(entry.rgb.r * 255.0f, entry.rgb.g * 255.0f, entry.rgb.b * 255.0f, entry.rgb.a * 255.0f), ("UID Entry: " + std::to_string(i)).c_str());
					}
					
				}

				
			}

			i += 1;
		}

	}



	MotFileNode::MotFileNode(std::string fName) : FileNode(fName) {
		fileIcon = ICON_CI_RECORD;
		TextColor = { 1.0f, 0.0f, 0.376f, 1.0f };
		nodeType = MOT;
		fileFilter = L"XSI Motion Files(*.mot)\0*.mot;\0";
	}
	void MotFileNode::LoadFile() {



	}
	void MotFileNode::SaveFile() {

	}

	std::string BxmFileNode::ConvertToXML(BXMInternal::XMLNode* node, int indentLevel) {
		if (!node) return "";

		std::string indent(indentLevel * 2, ' '); // Indentation for readability
		std::string xml = indent + "<" + node->name;

		// Process attributes
		for (const auto& attr : node->childAttributes) {
			if (!attr) continue;
			xml += " " + attr->name + "=\"" + attr->value + "\"";
		}

		if (node->childNodes.empty() && node->value.empty()) {
			xml += "/>\n"; // Self-closing tag
			return xml;
		}

		xml += ">";

		// Add value if present
		if (!node->value.empty()) {
			xml += node->value;

		}

		// Process child nodes recursively
		if (!node->childNodes.empty()) {
			xml += "\n";
			for (const auto& child : node->childNodes) {
				xml += ConvertToXML(child, indentLevel + 1);
			}
			xml += indent;
		}

		xml += "</" + node->name + ">\n";

		return xml;
	}


	BxmFileNode::BxmFileNode(std::string fName) : FileNode(fName) {
		fileIcon = ICON_CI_CODE;
		TextColor = { 1.0f, 0.0f, 0.914f, 1.0f };
		fileIsBigEndian = false;
		nodeType = BXM;
		fileFilter = L"Binary XML Files(*.bxm)\0*.bxm;\0";
	}

	void BxmFileNode::RenderGUI(CruelerContext *ctx) {

		if (ImGui::Button("Save")) {
			TinyXMLToGwpBXM();
			isEdited = true;
		}

		ImEditor.Render("TextEditor");

	

	}


	BXMInternal::XMLNode* BxmFileNode::ReadXMLNode(BinaryReader reader, BXMInternal::XMLNode* parent) {

		BXMInternal::XMLNode* node = new BXMInternal::XMLNode();
		node->parent = parent;
		int childCount = reader.ReadUINT16();
		int firstChildIndex = reader.ReadUINT16();
		int attributeNumber = reader.ReadUINT16();
		int dataIndex = reader.ReadUINT16();

		reader.Seek(dataOffset + (dataIndex * 4)); // Seek to data position

		int nameOffset = reader.ReadUINT16();
		int valueOffset = reader.ReadUINT16();

		if (nameOffset != 0xFFFF) {
			reader.Seek(stringOffset + nameOffset);
			node->name = reader.ReadNullTerminatedString();
		}
		if (valueOffset != 0xFFFF) {
			reader.Seek(stringOffset + valueOffset);
			node->value = reader.ReadNullTerminatedString();
		}

		for (int i = 0; i < attributeNumber; i++) {
			reader.Seek(dataOffset + ((dataIndex + i + 1) * 4));
			nameOffset = reader.ReadUINT16();
			valueOffset = reader.ReadUINT16();
			BXMInternal::XMLAttribute* attrib = new BXMInternal::XMLAttribute();

			if (nameOffset != 0xFFFF) {
				reader.Seek(stringOffset + nameOffset);
				attrib->name = reader.ReadNullTerminatedString();
			}
			if (valueOffset != 0xFFFF) {
				reader.Seek(stringOffset + valueOffset);
				attrib->value = reader.ReadNullTerminatedString();
			}




			node->childAttributes.push_back(attrib);
		}



		for (int i = 0; i < childCount; i++) {
			reader.Seek(infoOffset + ((firstChildIndex + i) * 8));
			node->childNodes.push_back(ReadXMLNode(reader, node));

		}

		return node;
	}

	void BxmFileNode::LoadFile() {
		BinaryReader reader(fileData, true);
		reader.Seek(0x8);
		int nodeCount = reader.ReadUINT16();
		int dataCount = reader.ReadUINT16();
		reader.Skip(sizeof(int)); // TODO: dataSize

		infoOffset = 16;
		dataOffset = 16 + 8 * nodeCount;
		stringOffset = 16 + 8 * nodeCount + dataCount * 4;
		

		baseNode = ReadXMLNode(reader, nullptr);
		xmlData = ConvertToXML(baseNode);

		ImEditor.SetText(xmlData);
		
		return;
	}

	std::vector<BXMInternal::XMLNode*> BxmFileNode::FlattenXMLTree(BXMInternal::XMLNode* root) {
		std::vector<BXMInternal::XMLNode*> output;
		std::queue<BXMInternal::XMLNode*> q;
		q.push(root);

		while (!q.empty()) {
			BXMInternal::XMLNode* node = q.front();
			q.pop();
			output.push_back(node);

			for (BXMInternal::XMLNode* child : node->childNodes) {
				q.push(child);
			}
		}

		return output;
	}

	BXMInternal::XMLNode* BxmFileNode::ConvertXML(tinyxml2::XMLElement* element, BXMInternal::XMLNode* parent) {
		BXMInternal::XMLNode* node = new BXMInternal::XMLNode();
		node->name = element->Name();
		node->value = element->GetText() ? element->GetText() : "";
		node->parent = parent;

		const tinyxml2::XMLAttribute* attr = element->FirstAttribute();
		while (attr) {
			auto* attrPtr = new BXMInternal::XMLAttribute();
			attrPtr->name = attr->Name();
			attrPtr->value = attr->Value();
			node->childAttributes.push_back(attrPtr);
			attr = attr->Next();
		}

		for (tinyxml2::XMLElement* child = element->FirstChildElement(); child; child = child->NextSiblingElement()) {
			BXMInternal::XMLNode* childNode = ConvertXML(child, node);
			node->childNodes.push_back(childNode);
		}

		return node;
	}

	int BxmFileNode::TinyXMLToGwpBXM() {
		tinyxml2::XMLDocument doc;
		if (doc.Parse(ImEditor.GetText().c_str()) != tinyxml2::XML_SUCCESS) {
			CTDLog::Log::getInstance().LogError("Failed to repack BXM! Check your XML file for errors.");
			return -1;
		}

		tinyxml2::XMLElement* root = doc.RootElement();
		if (!root) {
			CTDLog::Log::getInstance().LogError("Failed to repack BXM! Your XML requires a root node to be valid.");
			return -1;
		}

		baseNode = ConvertXML(root);

		return 0;
	}

	void BxmFileNode::SaveFile() {
		if (!isEdited) {
			return;
		}

		if (TinyXMLToGwpBXM() != 0) {
			CTDLog::Log::getInstance().LogError(fileName + " failed to repack!");
		}

		BinaryWriter* writer = new BinaryWriter(true);
		writer->WriteString("BXM");
		writer->WriteByteZero();
		writer->WriteUINT32(0);
		std::vector<BXMInternal::XMLNode*> nodes = FlattenXMLTree(baseNode);
		std::unordered_set<std::string> uniqueStrings;
		for (BXMInternal::XMLNode* node : nodes) {
			uniqueStrings.insert(node->name);
			uniqueStrings.insert(node->value);
			for (BXMInternal::XMLAttribute* attrib : node->childAttributes) {
				uniqueStrings.insert(attrib->name);
				uniqueStrings.insert(attrib->value);
			}
		}

		std::unordered_map<std::string, int> stringOffsets;
		int offsetTicker = 0;
		for (std::string nodeString : uniqueStrings) {
			if (nodeString.empty()) {
				stringOffsets[nodeString] = 0xFFFF;
				offsetTicker += 1; // I have no idea why this happens but it works 
			}
			else {
				stringOffsets[nodeString] = offsetTicker;
				offsetTicker += (nodeString.size() + 1);
			}


		}

		struct DataInfo {
			int name;
			int value;
		};


		std::vector<DataInfo> dataInfos;
		std::unordered_map <BXMInternal::XMLNode*, int> nodeInfoToDataIndice;
		int i = 0;
		for (BXMInternal::XMLNode* node : nodes) {
			dataInfos.push_back({ stringOffsets[node->name], stringOffsets[node->value] });
			nodeInfoToDataIndice[node] = i;
			for (BXMInternal::XMLAttribute* attrib : node->childAttributes) {
				dataInfos.push_back({ stringOffsets[attrib->name], stringOffsets[attrib->value] });
				i++;
			}
			
			i++;
		}

		writer->WriteUINT16(nodes.size());
		writer->WriteUINT16(dataInfos.size());
		writer->WriteUINT32(offsetTicker);

		struct BXMNodeInfo {
			int childSize;
			int firstChildIdx;
			int attributeSize;
			int dataIdx;
		};

		std::unordered_map<BXMInternal::XMLNode*, BXMNodeInfo> nodeInfos;
		std::unordered_map<BXMInternal::XMLNode*, int> nodeToIndex;
		for (int i = 0; i < nodes.size(); ++i) {
			nodeToIndex[nodes[i]] = i;
			nodeInfos[nodes[i]] = { (int)nodes[i]->childNodes.size(), -1, (int)nodes[i]->childAttributes.size(), nodeInfoToDataIndice[nodes[i]] };
		}


		for (BXMInternal::XMLNode* node : nodes) {
			BXMNodeInfo& info = nodeInfos[node];
			int nextIndex = -1;

			if (!node->childNodes.empty()) {
				BXMInternal::XMLNode* firstChild = node->childNodes.front();
				nextIndex = nodeToIndex[firstChild];
			}
			else {
				BXMInternal::XMLNode* parent = node->parent;

				if (parent != nullptr) {
					BXMInternal::XMLNode* lastChild = parent->childNodes.back();
					int lastChildIndex = nodeToIndex[lastChild];
					nextIndex = lastChildIndex + 1;
				}
				else {
					nextIndex = static_cast<int>(nodes.size());
				}
			}

			info.firstChildIdx = nextIndex;
		}


		for (BXMInternal::XMLNode* node : nodes) {
			writer->WriteUINT16(nodeInfos[node].childSize);
			writer->WriteUINT16(nodeInfos[node].firstChildIdx);
			writer->WriteUINT16(nodeInfos[node].attributeSize);
			writer->WriteUINT16(nodeInfos[node].dataIdx);
		}

		for (DataInfo info : dataInfos) {
			writer->WriteUINT16(info.name);
			writer->WriteUINT16(info.value);
		}

		for (std::string str : uniqueStrings) {
			writer->WriteString(str);
			writer->WriteByteZero();
		}

		fileData = writer->GetData();
	}


	WtbFileNode::WtbFileNode(std::string fName) : FileNode(fName) {
		fileIcon = ICON_CI_FILE_MEDIA;
		nodeType = WTB;
		TextColor = { 1.0f, 0.0f, 0.7f, 1.0f };
		fileFilter = L"Texture Container(*.wtb, *.wta)\0*.wtb;*.wta;\0";
	}
	void WtbFileNode::LoadFile() {
		BinaryReader reader(fileData, false);
		reader.SetEndianess(fileIsBigEndian);

		reader.Seek(0x4);
		if (reader.ReadUINT32() == 1) {
			textureCount = reader.ReadUINT32();
			int offsetTextureOffsets = reader.ReadUINT32();
			int offsetTextureSizes = reader.ReadUINT32();
			int offsetTextureFlags = reader.ReadUINT32();
			int offsetTextureIdx = reader.ReadUINT32();
			reader.Skip(sizeof(int)); // TODO: offsetTextureInfo




			reader.Seek(offsetTextureOffsets);
			for (int x = 0; x < textureCount; x++) {
				textureOffsets.push_back(reader.ReadUINT32());
			}

			reader.Seek(offsetTextureSizes);
			for (int x = 0; x < textureCount; x++) {
				textureSizes.push_back(reader.ReadUINT32());
			}

			reader.Seek(offsetTextureFlags);
			for (int x = 0; x < textureCount; x++) {
				textureFlags.push_back(reader.ReadUINT32());
			}

			reader.Seek(offsetTextureIdx);
			for (int x = 0; x < textureCount; x++) {
				textureIdx.push_back(reader.ReadUINT32());
			}


			/*for (int x = 0; x < textureCount; x++) {
				reader.Seek(textureOffsets[x]);
				textureData.push_back(reader.ReadBytes(textureSizes[x]));

			}*/




		}


	}
	void WtbFileNode::SaveFile() {

	}

	WemFileNode::WemFileNode(std::string fName) : FileNode(fName) {
		fileIcon = ICON_CI_MUSIC;
		nodeType = WEM;
		fileFilter = L"WWISE Audio Container (*.wem)\0*.wem\0"
			L"WAV Audio Files (*.wav)\0*.wav\0";
	}
	void WemFileNode::LoadFile() {

	}
	void WemFileNode::SaveFile() {

	}

	void WemFileNode::ReplaceFile()
	{
		OPENFILENAME ofn;
		wchar_t szFile[260] = { 0 };
		LPWSTR pFile = szFile;

		mbstowcs_s(0, pFile, fileName.length() + 1, fileName.c_str(), _TRUNCATE);

		ZeroMemory(&ofn, sizeof(OPENFILENAME));

		ofn.lpstrFile = pFile;
		ofn.lStructSize = sizeof(OPENFILENAME);
		ofn.hwndOwner = NULL;
		ofn.nMaxFile = 260;
		ofn.lpstrFilter = fileFilter;

		ofn.nFilterIndex = 1;
		ofn.Flags = OFN_PATHMUSTEXIST;
		int conversion_status = -1;
		if (GetOpenFileName(&ofn) == TRUE) {
			std::wstring title = (ofn.lpstrFile);
			size_t extensionsplitter = title.find_last_of(L".");
			std::wstring extension;
			if (extensionsplitter != std::wstring::npos) {
				extension = title.substr(extensionsplitter + 1);
			}

			if (extension == L"wem") {
				conversion_status = 1;
			}
			else if (extension == L"wav") {
				conversion_status = 2;
			}
			else {
				conversion_status = -1;
			}



			if (conversion_status == 1) {
				std::ifstream file(ofn.lpstrFile, std::ios::binary | std::ios::ate);
				if (file.is_open()) {
					fileData.clear();
					std::streamsize size = file.tellg();
					fileData.resize(size);
					file.seekg(0, std::ios::beg);
					file.read(fileData.data(), size);
					file.close();
					std::cout << "Replaced file! " << std::endl;
				}
				else {
					std::cout << "Error" << std::endl;
				}
			}
			else {
				CTDLog::Log::getInstance().LogError("Failed to replace WEM file (incorrect type or failed conversion)");
			}


		}

		ImGui::CloseCurrentPopup();

		LoadFile();
	}


	MGRVector DecodeNormalWMB0(uint32_t packed) {
		int8_t nz = (int8_t)((packed >> 0) & 0xFF);
		int8_t ny = (int8_t)((packed >> 8) & 0xFF);
		int8_t nx = (int8_t)((packed >> 16) & 0xFF);

		constexpr float scale = 1.0f / 127.0f;

		MGRVector normal;
		normal.x = nx * scale;
		normal.y = ny * scale;
		normal.z = nz * scale;

		return normal;
	}


	void WmbFileNode::LoadModelWMB0(BinaryReader& reader)
	{
		reader.ReadUINT32();
		reader.ReadUINT32();
		uint32_t vertexFormat = reader.ReadUINT32();
		uint32_t numVertexes = reader.ReadUINT32();
		uint8_t numMapping = reader.ReadINT8();
		uint8_t numColor = reader.ReadINT8();
		uint16_t unknownE = reader.ReadINT16();
		uint32_t offsetPositions = reader.ReadUINT32();
		uint32_t offsetVertexes = reader.ReadUINT32();
		uint32_t offsetVertexesExData = reader.ReadUINT32();
		reader.ReadUINT32Array(4); // Unknown G4
		uint32_t numBones = reader.ReadUINT32();
		uint32_t offsetBoneHierarchy = reader.ReadUINT32();
		uint32_t offsetBoneRelativePosition = reader.ReadUINT32();
		uint32_t offsetBonePosition = reader.ReadUINT32();
		uint32_t offsetBoneIndexTranslateTable = reader.ReadUINT32();
		uint32_t numMaterials = reader.ReadINT32();
		uint32_t offsetMaterialsOffset = reader.ReadUINT32();
		uint32_t offsetMaterials = reader.ReadUINT32();
		uint32_t numMeshes = reader.ReadINT32();
		uint32_t offsetMeshesOffsets = reader.ReadUINT32();
		uint32_t offsetMeshes = reader.ReadUINT32();
		uint32_t numPolygons = reader.ReadINT32();
		uint32_t numShaderSettings = reader.ReadINT32();
		uint32_t offsetInverseKinetic = reader.ReadUINT32();
		uint32_t offsetBoneSymmetries = reader.ReadUINT32();
		uint32_t offsetBoneFlags = reader.ReadUINT32();
		uint32_t exMatShaderNamesOffset = reader.ReadUINT32();
		uint32_t exMatSamplers = reader.ReadUINT32();
		std::vector<uint32_t> exMatInfo = reader.ReadUINT32Array(2); // ExMat Info
		if (exMatInfo[0] != 0) {
			wmbVersion = WMB0_BAY2;
		}


		struct WMB0BatchHeader {
			uint16_t batchIdx;
			uint16_t id;
			uint16_t flags;
			uint8_t exMaterialID;
			uint8_t materialID;
			uint8_t hasBoneRefs;
			uint8_t unknownE1;
			uint8_t unknownE2;
			uint32_t vertexStart;
			uint32_t vertexEnd;
			uint32_t primitiveType;
			uint32_t offsetIndices;
			uint32_t numIndices;
			uint32_t vertexOffset;
			uint32_t numBoneRefs;
			std::vector<uint16_t> indices;

			void Read(BinaryReader& br) {
				size_t batchStart = br.Tell();
				batchIdx = br.ReadUINT16();
				id = br.ReadUINT16();
				flags = br.ReadUINT16();
				exMaterialID = br.ReadUINT16();
				materialID = br.ReadINT8();
				hasBoneRefs = br.ReadINT8();
				unknownE1 = br.ReadINT8();
				unknownE2 = br.ReadINT8();
				vertexStart = br.ReadUINT32();
				vertexEnd = br.ReadUINT32();
				primitiveType = br.ReadUINT32();
				offsetIndices = br.ReadUINT32();
				numIndices = br.ReadUINT32();
				vertexOffset = br.ReadUINT32();
				br.ReadUINT32Array(7);
				numBoneRefs = br.ReadUINT32();

				br.Seek(batchStart + offsetIndices);
				indices = br.ReadUINT16Array(numIndices);

			}

		};


		struct WMB0MeshHeader {
			uint16_t id;
			uint16_t numBatch;
			uint16_t unknownA1;
			uint16_t boundingBoxInfos;
			uint32_t offsetBatchOffsets;
			int32_t flags;
			uint32_t unknown_c_0;
			uint32_t unknown_c_1;
			uint32_t unknown_c_2;
			uint32_t unknown_c_3;
			std::string name;
			WMBVector center;
			float height;
			WMBVector corner1;
			WMBVector corner2;
			float unknownD;
			float unknownE;
			std::vector<WMB0BatchHeader> batches;

			void Read(BinaryReader& br) {
				size_t meshStart = br.Tell();
				id = br.ReadUINT16();
				numBatch = br.ReadUINT16();
				unknownA1 = br.ReadUINT16();
				boundingBoxInfos = br.ReadUINT16();
				offsetBatchOffsets = br.ReadUINT32();
				flags = br.ReadUINT32();
				unknown_c_0 = br.ReadUINT32();
				unknown_c_1 = br.ReadUINT32();
				unknown_c_2 = br.ReadUINT32();
				unknown_c_3 = br.ReadUINT32();
				name = br.ReadString(32);
				center.x = br.ReadFloat();
				center.y = br.ReadFloat();
				center.z = br.ReadFloat();
				height = br.ReadFloat();
				corner1.x = br.ReadFloat();
				corner1.y = br.ReadFloat();
				corner1.z = br.ReadFloat();
				corner2.x = br.ReadFloat();
				corner2.y = br.ReadFloat();
				corner2.z = br.ReadFloat();
				unknownD = br.ReadFloat();
				unknownE = br.ReadFloat();

				br.Seek(meshStart + offsetBatchOffsets);
				std::vector<uint32_t> batchOffsets = br.ReadUINT32Array(numBatch);
				for (uint32_t batch : batchOffsets) {
					br.Seek(meshStart + offsetBatchOffsets + batch);
					WMB0BatchHeader batch;
					batch.Read(br);
					batches.push_back(batch);
				}

			}
		};

		std::vector<CUSTOMVERTEX> vertexPool;

		reader.Seek(offsetVertexes);
		for (uint32_t i = 0; i < numVertexes; i++) {
			CUSTOMVERTEX cvtx = CUSTOMVERTEX();

			if (vertexFormat != 0x4000001F) {
				cvtx.x = reader.ReadFloat();
				cvtx.y = reader.ReadFloat();
				cvtx.z = reader.ReadFloat();


				cvtx.u = HelperFunction::HalfToFloat(reader.ReadUINT16());
				cvtx.v = HelperFunction::HalfToFloat(reader.ReadUINT16());

				MGRVector normals = DecodeNormalWMB0(reader.ReadUINT32());
				cvtx.nx = normals.x;
				cvtx.ny = normals.y;
				cvtx.nz = normals.z;
				glm::vec4 tangents = HelperFunction::DecodeTangent(reader.ReadUINT32());
				cvtx.tx = tangents.x;
				cvtx.ty = tangents.y;
				cvtx.tz = tangents.z;
				cvtx.tw = tangents.w;

				reader.ReadUINT32();
				reader.ReadUINT32();

				cvtx.color = 0xFFFFFFFF;
			}



			vertexPool.push_back(cvtx);

		}



		reader.Seek(offsetMeshesOffsets);
		std::vector<uint32_t> offsetMeshArray = reader.ReadUINT32Array(numMeshes);
		std::vector<WMB0MeshHeader> meshHeaders;
		for (uint32_t i = 0; i < numMeshes; i++) {
			reader.Seek(offsetMeshes + offsetMeshArray[i]);
			WMB0MeshHeader mshHeader;
			mshHeader.Read(reader);

			meshHeaders.push_back(mshHeader);
			
		}

		reader.Seek(offsetMaterialsOffset);
		std::vector<uint32_t> offsetMaterialsArray = reader.ReadUINT32Array(numMaterials);

		for (uint32_t i = 0; i < numMaterials; i++) {
			CTDMaterial mat;
			reader.Seek(offsetMaterials + offsetMaterialsArray[i]);
			uint32_t materialID = reader.ReadUINT16();
			reader.ReadUINT16();

			mat.texture_data[0] = reader.ReadUINT32();
			mat.texture_data[2] = reader.ReadUINT32();

			materials.push_back(mat);
		}

		for (WMB0MeshHeader& mesh : meshHeaders) {
			CruelerMesh* cmesh = new CruelerMesh();
			cmesh->name = mesh.name;
			
			for (WMB0BatchHeader& batch : mesh.batches) {
				CruelerBatch* ctdbatch = new CruelerBatch();
				std::vector<uint16_t> remappedIndices;
				if (batch.primitiveType == 5) {
					ctdbatch->drawMethod = GL_TRIANGLE_STRIP;

					
					remappedIndices.reserve(batch.indices.size());

					for (uint16_t idx : batch.indices) {
						remappedIndices.push_back(idx - batch.vertexStart);
					}
				}
				else {
					remappedIndices = batch.indices;
				}

				ctdbatch->materialID = batch.materialID;

				ctdbatch->indexCount = batch.numIndices;



				glGenBuffers(1, &ctdbatch->indexBuffer);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ctdbatch->indexBuffer);
				glBufferData(GL_ELEMENT_ARRAY_BUFFER, remappedIndices.size() * sizeof(unsigned short), remappedIndices.data(), GL_STATIC_DRAW);

				// Vertexes
				std::vector<CUSTOMVERTEX> batchSubPool(
					vertexPool.begin() + batch.vertexStart,
					vertexPool.begin() + batch.vertexEnd);

				if (ctdbatch->vertexBuffer == 0)
					glGenBuffers(1, &ctdbatch->vertexBuffer);

				glBindBuffer(GL_ARRAY_BUFFER, ctdbatch->vertexBuffer);
				glBufferData(GL_ARRAY_BUFFER, batchSubPool.size() * sizeof(CUSTOMVERTEX), batchSubPool.data(), GL_STATIC_DRAW);

				ctdbatch->vertexes = batchSubPool;



				glGenVertexArrays(1, &ctdbatch->vao);

				glBindVertexArray(ctdbatch->vao);

				// Rebind VBO/IBO inside VAO scope
				glBindBuffer(GL_ARRAY_BUFFER, ctdbatch->vertexBuffer);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ctdbatch->indexBuffer);

				size_t curOffset = 0;
				// Position
				glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(CUSTOMVERTEX), (void*)curOffset);
				glEnableVertexAttribArray(0);
				curOffset += 3 * sizeof(float);

				// Color
				glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(CUSTOMVERTEX), (void*)curOffset);
				glEnableVertexAttribArray(1);
				curOffset += 4 * sizeof(byte);

				// UV map
				glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(CUSTOMVERTEX), (void*)curOffset);
				glEnableVertexAttribArray(2);
				curOffset += 2 * sizeof(float);

				// Normals
				glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(CUSTOMVERTEX), (void*)curOffset);
				glEnableVertexAttribArray(3);
				curOffset += 3 * sizeof(float);

				// Tangents
				glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(CUSTOMVERTEX), (void*)curOffset);
				glEnableVertexAttribArray(4);
				curOffset += 4 * sizeof(float);

				// Lightmap UV
				glVertexAttribPointer(5, 2, GL_FLOAT, GL_FALSE, sizeof(CUSTOMVERTEX), (void*)curOffset);
				glEnableVertexAttribArray(2);
				curOffset += 2 * sizeof(float);

				glBindVertexArray(0);

				cmesh->batches.push_back(ctdbatch);
			}




			displayMeshes.push_back(cmesh);
		}



		return;
	}

	void WmbFileNode::LoadModelWMB3(BinaryReader& reader)
	{
		struct WMB3Header {
			uint32_t offsetBones;
			uint32_t numBones;
			uint32_t offsetBoneIndexTranslateTable;
			uint32_t boneTranslateTableSize;
			uint32_t offsetVertexGroups;
			uint32_t numVertexGroups;
			uint32_t offsetBatches;
			uint32_t numBatches;
			uint32_t offsetLods;
			uint32_t numLods;
			uint32_t offsetColTreeNodes;
			uint32_t numColTreeNodes;
			uint32_t offsetBoneMap;
			uint32_t boneMapSize;
			uint32_t offsetBoneSets;
			uint32_t numBoneSets;
			uint32_t offsetMaterials;
			uint32_t numMaterials;
			uint32_t offsetMeshes;
			uint32_t numMeshes;
			uint32_t offsetMeshMaterial;
			uint32_t numMeshMaterial;
		};

		struct WMB3VertexGroup {
			uint32_t vertexOffset;
			uint32_t vertexExOffset;
			uint32_t u_0;
			uint32_t u_1;
			uint32_t vertexSize;
			uint32_t vertexExDataSize;
			uint32_t u_2;
			uint32_t u_3;
			uint32_t numVertexes;
			uint32_t vertexFlags;
			uint32_t indexBufferOffset;
			uint32_t numIndexes;

			void Read(BinaryReader& br) {
				vertexOffset = br.ReadUINT32();
				vertexExOffset = br.ReadUINT32();
				u_0 = br.ReadUINT32();
				u_1 = br.ReadUINT32();
				vertexSize = br.ReadUINT32();
				vertexExDataSize = br.ReadUINT32();
				u_2 = br.ReadUINT32();
				u_3 = br.ReadUINT32();
				numVertexes = br.ReadUINT32();
				vertexFlags = br.ReadUINT32();
				indexBufferOffset = br.ReadUINT32();
				numIndexes = br.ReadUINT32();
			}

		};

		struct WMB3Batch {
			uint32_t vertexGroupIndex;
			int32_t boneSetIndex;
			uint32_t vertexStart;
			uint32_t indexStart;
			uint32_t numVertexes;
			uint32_t numIndexes;
			uint32_t numPrimitives;

			void Read(BinaryReader& br) {
				vertexGroupIndex = br.ReadUINT32();
				boneSetIndex = br.ReadINT32();
				vertexStart = br.ReadUINT32();
				indexStart = br.ReadUINT32();
				numVertexes = br.ReadUINT32();
				numIndexes = br.ReadUINT32();
				numPrimitives = br.ReadUINT32();
			}
		};

		struct WMB3Mesh {
			uint32_t offsetName;
			WMBBoundingBox boundingBox;
			uint32_t offsetMaterials;
			uint32_t numMaterials;
			uint32_t offsetBones;
			uint32_t numBones;


			void Read(BinaryReader& br) {
				offsetName = br.ReadUINT32();
				boundingBox.Read(br);
				offsetMaterials = br.ReadUINT32();
				numMaterials = br.ReadUINT32();
				offsetBones = br.ReadUINT32();
				numBones = br.ReadUINT32();
			}
		};

		struct WMB3LODBatchInfo {
			uint32_t vertexGroupIdx;
			uint32_t meshIdx;
			uint32_t materialIdx;
			int32_t colTreeNodeIdx;
			uint32_t meshMatPairIndex;
			int32_t indexToUnknown;
			uint32_t batchIdx;

			void Read(BinaryReader& br) {
				vertexGroupIdx = br.ReadUINT32();
				meshIdx = br.ReadUINT32();
				materialIdx = br.ReadUINT32();
				colTreeNodeIdx = br.ReadINT32();
				meshMatPairIndex = br.ReadUINT32();
				indexToUnknown = br.ReadINT32();
			};

		};


		WMB3Header header = WMB3Header();
		reader.Seek(40);
		header.offsetBones = reader.ReadUINT32();
		header.numBones = reader.ReadUINT32();
		header.offsetBoneIndexTranslateTable = reader.ReadUINT32();
		header.boneTranslateTableSize = reader.ReadUINT32();
		header.offsetVertexGroups = reader.ReadUINT32();
		header.numVertexGroups = reader.ReadUINT32();
		header.offsetBatches = reader.ReadUINT32();
		header.numBatches = reader.ReadUINT32();
		header.offsetLods = reader.ReadUINT32();
		header.numLods = reader.ReadUINT32();
		header.offsetColTreeNodes = reader.ReadUINT32();
		header.numColTreeNodes = reader.ReadUINT32();
		header.offsetBoneMap = reader.ReadUINT32();
		header.boneMapSize = reader.ReadUINT32();
		header.offsetBoneSets = reader.ReadUINT32();
		header.numBoneSets = reader.ReadUINT32();
		header.offsetMaterials = reader.ReadUINT32();
		header.numMaterials = reader.ReadUINT32();
		header.offsetMeshes = reader.ReadUINT32();
		header.numMeshes = reader.ReadUINT32();
		header.offsetMeshMaterial = reader.ReadUINT32();
		header.numMeshMaterial = reader.ReadUINT32();
		
		reader.Seek(header.offsetVertexGroups);
		std::vector<WMB3VertexGroup> vertexGroups;
		for (uint32_t i = 0; i < header.numVertexGroups; i++) {
			WMB3VertexGroup vtxGroup;
			vtxGroup.Read(reader);
			vertexGroups.push_back(vtxGroup);
		}
		reader.Seek(header.offsetBatches);
		std::vector<WMB3Batch> batches;
		for (uint32_t i = 0; i < header.numBatches; i++) {
			WMB3Batch itm;
			itm.Read(reader);
			batches.push_back(itm);
		}

		reader.Seek(header.offsetMaterials);
		for (uint32_t i = 0; i < header.numMaterials; i++) {
			size_t nextMaterialPosition = reader.Tell() + 0x30;
			CTDMaterial cmat = CTDMaterial();
			reader.ReadUINT32Array(2);
			uint32_t offsetName = reader.ReadUINT32();
			uint32_t offsetShaderName = reader.ReadUINT32();
			uint32_t offsetTechniqueName = reader.ReadUINT32();
			reader.ReadUINT32();
			uint32_t offsetTextures = reader.ReadUINT32();
			uint32_t numTextures = reader.ReadUINT32();

			reader.Seek(offsetShaderName);
			cmat.shader_name = reader.ReadNullTerminatedString();
			
			reader.Seek(offsetTextures);

			for (int j = 0; j < numTextures; j++) {
				uint32_t offsetTexName = reader.ReadUINT32();
				uint32_t texId = reader.ReadUINT32();
				uint32_t nextTexPosition = reader.Tell();
				reader.Seek(offsetTexName);
				uint32_t falseTexFlag = 0;
				std::string texName = reader.ReadNullTerminatedString();
				if (texName == "g_AlbedoMap") {
					falseTexFlag = 0;
				}
				else if (texName == "g_NormalMap") {
					falseTexFlag = 2;
				}
				cmat.texture_data[falseTexFlag] = texId;

				reader.Seek(nextTexPosition);
			}


			materials.push_back(cmat);

			reader.Seek(nextMaterialPosition);
		}

		reader.Seek(header.offsetMeshes);
		std::vector<WMB3Mesh> meshes;
		for (uint32_t i = 0; i < header.numMeshes; i++) {
			WMB3Mesh itm;
			itm.Read(reader);
			meshes.push_back(itm);
		}

		reader.Seek(header.offsetLods);
		std::vector<WMB3LODBatchInfo> lodBatchInfos;
		reader.ReadUINT32Array(3);
		uint32_t offsetBatchInfos = reader.ReadUINT32();
		uint32_t numBatchInfos = reader.ReadUINT32();
		for (uint32_t i = 0; i < numBatchInfos; i++) {
			WMB3LODBatchInfo itm;
			itm.Read(reader);
			itm.batchIdx = i;
			lodBatchInfos.push_back(itm);
		}

		uint32_t meshID = 0;
		for (WMB3Mesh& mesh : meshes) {
			CruelerMesh* ctdmesh = new CruelerMesh();
			ctdmesh->vtxfmt = 0;
			reader.Seek(mesh.offsetName);
			ctdmesh->name = reader.ReadNullTerminatedString();

			reader.Seek(mesh.offsetMaterials);
			for (uint32_t x = 0; x < mesh.numMaterials; x++) {
				ctdmesh->materials.push_back(&materials[reader.ReadUINT16()]);
			}
			
			std::vector<uint32_t> batchIDs;
			for (WMB3LODBatchInfo& info : lodBatchInfos) {
				if (info.meshIdx == meshID) {
					batchIDs.push_back(info.batchIdx);
				}
			}
			
			for (uint32_t meshBatchID : batchIDs) {
				CruelerBatch* ctdbatch = new CruelerBatch();

				WMB3Batch& activeBatch = batches[meshBatchID];
				WMB3VertexGroup& activeVtxGroup = vertexGroups[activeBatch.vertexGroupIndex];

				ctdbatch->vertexCount = activeVtxGroup.numVertexes;
				ctdbatch->indexCount = activeVtxGroup.numIndexes;
				ctdbatch->materialID = 0; // TODO: Fix


				reader.Seek(activeVtxGroup.indexBufferOffset + (sizeof(int) * activeBatch.indexStart));
				std::vector<unsigned int> indices;
				for (uint32_t x = 0; x < activeBatch.numIndexes; x++) {
					indices.push_back(reader.ReadUINT32());
				}

				ctdbatch->indexCount = static_cast<int>(indices.size());
				ctdbatch->indexes_wmb3 = indices;
				glGenBuffers(1, &ctdbatch->indexBuffer);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ctdbatch->indexBuffer);
				glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

				if (activeVtxGroup.vertexFlags == 0x8) {
					reader.Seek(activeVtxGroup.vertexOffset + activeBatch.vertexStart * 28);
					ctdmesh->structSize = sizeof(CUSTOMVERTEX);

					std::vector<WMB3Vertex> vertexes;
					for (uint32_t i = 0; i < activeBatch.numVertexes; i++) {
						WMB3Vertex vtx;
						vtx.Read(reader);
						vertexes.push_back(vtx);
					}

					std::vector<CUSTOMVERTEX> convertedVtx;
					for (WMB3Vertex& vertex : vertexes) {
						CUSTOMVERTEX cvtx;
						cvtx.x = vertex.position.x;
						cvtx.y = vertex.position.y;
						cvtx.z = vertex.position.z;

						glm::vec4 tangents = HelperFunction::DecodeTangent(vertex.tangents);
						cvtx.tx = tangents.x;
						cvtx.ty = tangents.y;
						cvtx.tz = tangents.z;
						cvtx.tw = tangents.w;

						cvtx.u = HelperFunction::HalfToFloat(vertex.uv.u);
						cvtx.v = HelperFunction::HalfToFloat(vertex.uv.v);
						cvtx.color = 0xFFFFFFFF;
						convertedVtx.push_back(cvtx);
					}

					if (ctdbatch->vertexBuffer == 0)
						glGenBuffers(1, &ctdbatch->vertexBuffer);

					glBindBuffer(GL_ARRAY_BUFFER, ctdbatch->vertexBuffer);
					glBufferData(GL_ARRAY_BUFFER, convertedVtx.size() * sizeof(CUSTOMVERTEX), convertedVtx.data(), GL_STATIC_DRAW);

					ctdbatch->vertexes = convertedVtx;
				}
				else {
					CTDLog::Log::getInstance().LogError("Unsupported vertex format! (Bayonetta 1: " + std::to_string(activeVtxGroup.vertexFlags) + ")");
					ctdbatch->isValid = false;
				}

				glGenVertexArrays(1, &ctdbatch->vao);

				glBindVertexArray(ctdbatch->vao);

				// Rebind VBO/IBO inside VAO scope
				glBindBuffer(GL_ARRAY_BUFFER, ctdbatch->vertexBuffer);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ctdbatch->indexBuffer);

				size_t curOffset = 0;
				// Position
				glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(CUSTOMVERTEX), (void*)curOffset);
				glEnableVertexAttribArray(0);
				curOffset += 3 * sizeof(float);

				// Color
				glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(CUSTOMVERTEX), (void*)curOffset);
				glEnableVertexAttribArray(1);
				curOffset += 4 * sizeof(byte);

				// UV map
				glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(CUSTOMVERTEX), (void*)curOffset);
				glEnableVertexAttribArray(2);
				curOffset += 2 * sizeof(float);

				// Normals
				glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(CUSTOMVERTEX), (void*)curOffset);
				glEnableVertexAttribArray(3);
				curOffset += 3 * sizeof(float);

				// Tangents
				glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(CUSTOMVERTEX), (void*)curOffset);
				glEnableVertexAttribArray(4);
				curOffset += 4 * sizeof(float);

				// Lightmap UV
				glVertexAttribPointer(5, 2, GL_FLOAT, GL_FALSE, sizeof(CUSTOMVERTEX), (void*)curOffset);
				glEnableVertexAttribArray(2);
				curOffset += 2 * sizeof(float);

				glBindVertexArray(0);

				ctdmesh->batches.push_back(ctdbatch);

			}


			displayMeshes.push_back(ctdmesh);
			meshID++;
		}

		return;
	}

	WmbFileNode::WmbFileNode(std::string fName) : FileNode(fName) {
		fileIcon = ICON_CI_SYMBOL_METHOD;
		TextColor = { 1.0f, 0.671f, 0.0f, 1.0f };
		nodeType = WMB;
		fileFilter = L"Platinum Model File(*.wmb)\0*.wmb;\0";
	}

	void WmbFileNode::GetBoneNames() {
		std::ifstream cfgfile("Assets/WMB/bone_config.json");
		boneNameSourceFile = "Assets/WMB/bone_names.json";
		if (cfgfile.is_open()) {
			nlohmann::json j;
			cfgfile >> j;
			for (auto& [key, value] : j.items()) {
				std::string targetfilename = key;
				std::string jsonname = value.get<std::string>();

				if (targetfilename == fileName) {
					boneNameSourceFile =  "Assets/WMB/" + jsonname;
					break;
				}

			}
		}

		cfgfile.close();

		std::ifstream file(boneNameSourceFile);
		if (file.is_open()) {
			nlohmann::json j;
			file >> j;

			for (auto& [key, value] : j.items()) {
				int id = std::stoi(key);            // JSON keys are strings
				std::string name = value.get<std::string>();
				boneNames[id] = name;
			}
			
		}

		file.close();

	}

	std::string WmbFileNode::GetBoneNameFromID(int boneID) {
		if (boneNames.find(boneID) != boneNames.end()) {
			return boneNames[boneID];
		}
	


		std::stringstream ss;
		ss << std::setw(4) << std::setfill('0') << boneID;

		return "bone" + ss.str();
	}

	void WmbFileNode::LoadFile() {
		BinaryReader reader(fileData, true);
		reader.SetEndianess(fileIsBigEndian);

		if (wmbVersion == WMB3_BAY3) {
			LoadModelWMB3(reader);
			return;
		}
		else if (wmbVersion == WMB0_BAY1) {
			LoadModelWMB0(reader);
			return;
		}


		GetBoneNames();

		WMBHeader header = WMBHeader();
		header.Read(reader);

		if (header.cutdataOffset != 0) {
			hasCutdata = true;
		}

		reader.Seek(header.offsetVertexGroups);
		std::vector<WMBVertexGroup> vertexGroups;
		for (uint32_t i = 0; i < header.numVertexGroups; i++) {
			WMBVertexGroup vtxGroup;
			vtxGroup.Read(reader);
			vertexGroups.push_back(vtxGroup);
		}

		reader.Seek(header.offsetBatches);
		std::vector<WMBBatch> batches;
		for (uint32_t i = 0; i < header.numBatches; i++) {
			WMBBatch itm;
			itm.Read(reader);
			batches.push_back(itm);
		}

		reader.Seek(header.offsetBatchDescription);
		unsigned int offsetBatchData = reader.ReadUINT32();

		reader.Seek(offsetBatchData);
		std::vector<WMBBatchData> batchDatas;
		for (uint32_t i = 0; i < header.numBatches; i++) {
			WMBBatchData itm;
			itm.Read(reader);
			batchDatas.push_back(itm);
		}

		reader.Seek(header.offsetBones);
		for (uint32_t i = 0; i < header.numBones; i++) {
			CruelerBone bone = CruelerBone();
			bone.boneID = reader.ReadINT16();
			reader.ReadINT16();
			bone.parentIndex = reader.ReadINT16();
			reader.ReadUINT16(); // Unknown
			bone.localPosition = { reader.ReadFloat(), reader.ReadFloat(), reader.ReadFloat() };
			bone.worldPosition = { reader.ReadFloat(), reader.ReadFloat(), reader.ReadFloat() };

			bone.localTransform = glm::translate(glm::mat4(1.0f), bone.localPosition);

			bones.push_back(bone);
		}
		for (uint32_t i = 0; i < header.numBones; i++) {
			if (bones[i].parentIndex >= 0) {
				bones[i].combinedTransform = bones[bones[i].parentIndex].combinedTransform * bones[i].localTransform;
			}
			else {
				bones[i].combinedTransform = bones[i].localTransform;
			}
		}
		for (uint32_t i = 0; i < header.numBones; i++) {
			bones[i].offsetMatrix = glm::inverse(bones[i].combinedTransform);
		}


		reader.Seek(header.offsetMeshes);
		std::vector<WMBMesh> meshes;
		for (uint32_t i = 0; i < header.numMeshes; i++) {
			WMBMesh itm;
			itm.Read(reader);
			meshes.push_back(itm);
		}

		reader.Seek(header.offsetTextures);
		std::vector<WMBTexture> textures;
		for (uint32_t i = 0; i < header.numTextures; i++) {
			WMBTexture itm;
			itm.Read(reader);
			textures.push_back(itm);
		}


		reader.Seek(header.offsetMaterials);
		for (uint32_t i = 0; i < header.numMaterials; i++) {
			WMBMaterial mat;
			mat.Read(reader);
			size_t place = reader.Tell();
			CTDMaterial cmat = CTDMaterial();
			
			reader.Seek(mat.offsetShaderName);
			cmat.shader_name = reader.ReadString(16);
			cmat.shader_name.erase(std::remove(cmat.shader_name.begin(), cmat.shader_name.end(), '\0'), cmat.shader_name.end()); // sanatize
			reader.Seek(mat.offsetParameters);
			cmat.materialDataOffset = mat.offsetParameters;
			for (int j = 0; j < mat.numParameters / 4; j++) {
				cmat.parameters.push_back({reader.ReadFloat(), reader.ReadFloat(), reader.ReadFloat(), reader.ReadFloat()});
			}

			
			reader.Seek(mat.offsetTextures);
			std::vector<WMBTexture> textureMappings;
			for (int j = 0; j < mat.numTextures; j++) {
				WMBTexture itm;
				itm.Read(reader);
				textureMappings.push_back(itm);
			}

			for (WMBTexture& tex : textureMappings) {

				cmat.texture_data[tex.flag] = textures[tex.id].id;

			}

			materials.push_back(cmat);

			reader.Seek(place);
		}


		for (WMBMesh& mesh : meshes) {
			CruelerMesh* ctdmesh = new CruelerMesh();
			ctdmesh->vtxfmt = header.vertexFormat;
			reader.Seek(mesh.offsetName);
			ctdmesh->name = reader.ReadNullTerminatedString();
			// Materials
			reader.Seek(mesh.offsetMaterials);
			for (uint32_t x = 0; x < mesh.numMaterials; x++) {
				ctdmesh->materials.push_back(&materials[reader.ReadUINT16()]);
			}
			std::vector<unsigned short> batchIDs;

			reader.Seek(mesh.offsetBatches); // this format sucks :fire:
			batchIDs = reader.ReadUINT16Array(mesh.numBatches);


			for (unsigned short meshBatchID : batchIDs) {
				CruelerBatch* ctdbatch = new CruelerBatch();

				WMBBatch& activeBatch = batches[meshBatchID];
				WMBBatchData& activeBatchData = batchDatas[meshBatchID];
				WMBVertexGroup& activeVtxGroup = vertexGroups[activeBatch.vertexGroupIndex];

				ctdbatch->vertexCount = activeVtxGroup.numVertexes;
				ctdbatch->indexCount = activeVtxGroup.numIndexes;
				
				ctdbatch->materialID = activeBatchData.materialIndex;


				reader.Seek(activeVtxGroup.offsetIndexes + (sizeof(short) * activeBatch.indexStart));
				std::vector<unsigned short> indices;
				for (uint32_t x = 0; x < activeBatch.numIndices; x++) {
					indices.push_back(reader.ReadUINT16());
				}

				if (fileIsBigEndian) { // what the actual fuck platinum
					std::vector<unsigned short> chain;
					std::vector<unsigned short> newIndices;
					bool reverse = false;
					for (unsigned short& indice : indices) {
						if (indice == 0xFFFF) {
							chain.clear();
							reverse = false;
							continue;
						}
						chain.push_back(indice);
						if (chain.size() > 3) {
							chain.erase(chain.begin());
						}
						if (chain.size() == 3) {
							if (reverse) {
								std::reverse(chain.begin(), chain.end());
							}
							newIndices.insert(newIndices.end(), chain.begin(), chain.end());
							if (reverse) {
								std::reverse(chain.begin(), chain.end());
							}
							reverse = !reverse;

						}
					}

					indices = newIndices;
				}
				ctdbatch->indexCount = static_cast<int>(indices.size());
				ctdbatch->indexes = indices;
				glGenBuffers(1, &ctdbatch->indexBuffer);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ctdbatch->indexBuffer);
				glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned short), indices.data(), GL_STATIC_DRAW);



				// vertex format lore
				if (header.vertexFormat == 263) {
					reader.Seek(activeVtxGroup.offsetVertexes + activeBatch.vertexStart * sizeof(WMBVertexA));
					ctdmesh->structSize = sizeof(CUSTOMVERTEX);

					std::vector<WMBVertexA> vertexes;
					for (uint32_t i = 0; i < activeBatch.numVertices; i++) {
						WMBVertexA vtx;
						vtx.Read(reader);
						vertexes.push_back(vtx);
					}

					std::vector<CUSTOMVERTEX> convertedVtx;
					for (WMBVertexA& vertex : vertexes) {
						CUSTOMVERTEX cvtx;
						cvtx.x = vertex.position.x;
						cvtx.y = vertex.position.y;
						cvtx.z = vertex.position.z;
						MGRVector normals = HelperFunction::DecodeNormal(vertex.normals);	
						cvtx.nx = normals.x;
						cvtx.ny = normals.y;
						cvtx.nz = normals.z;
						glm::vec4 tangents = HelperFunction::DecodeTangent(vertex.tangents);
						cvtx.tx = tangents.x;
						cvtx.ty = tangents.y;
						cvtx.tz = tangents.z;
						cvtx.tw = tangents.w;

						cvtx.u = HelperFunction::HalfToFloat(vertex.uv.u);
						cvtx.v = HelperFunction::HalfToFloat(vertex.uv.v);
						cvtx.color = 0xFFFFFFFF;
						convertedVtx.push_back(cvtx);
					}

					if (ctdbatch->vertexBuffer == 0)
						glGenBuffers(1, &ctdbatch->vertexBuffer);

					glBindBuffer(GL_ARRAY_BUFFER, ctdbatch->vertexBuffer);
					glBufferData(GL_ARRAY_BUFFER, convertedVtx.size() * sizeof(CUSTOMVERTEX), convertedVtx.data(), GL_STATIC_DRAW);

					ctdbatch->vertexes = convertedVtx;
				}
				else if (header.vertexFormat == 65847 || header.vertexFormat == 66359 || header.vertexFormat == 311) {

					reader.Seek(activeVtxGroup.offsetVertexes + activeBatch.vertexStart * sizeof(WMBVertex65847));

					ctdmesh->structSize = sizeof(CUSTOMVERTEX);

					std::vector<WMBVertex65847> vertexes;
					for (uint32_t i = 0; i < activeBatch.numVertices; i++) {
						WMBVertex65847 vtx;
						vtx.Read(reader);
						vertexes.push_back(vtx);
					}


					std::vector<CUSTOMVERTEX> convertedVtx;
					for (WMBVertex65847& vertex : vertexes) {
						CUSTOMVERTEX cvtx;
						cvtx.x = vertex.position.x;
						cvtx.y = vertex.position.y;
						cvtx.z = vertex.position.z;
						MGRVector normals = HelperFunction::DecodeNormal(vertex.normals);
						cvtx.nx = normals.x;
						cvtx.ny = normals.y;
						cvtx.nz = normals.z;
						glm::vec4 tangents = HelperFunction::DecodeTangent(vertex.tangents);
						cvtx.tx = tangents.x;
						cvtx.ty = tangents.y;
						cvtx.tz = tangents.z;
						cvtx.tw = tangents.w;
						cvtx.u = HelperFunction::HalfToFloat(vertex.uv.u);
						cvtx.v = HelperFunction::HalfToFloat(vertex.uv.v);
						cvtx.color = 0xFFFFFFFF;
						convertedVtx.push_back(cvtx);
					}

					if (ctdbatch->vertexBuffer == 0)
						glGenBuffers(1, &ctdbatch->vertexBuffer);

					glBindBuffer(GL_ARRAY_BUFFER, ctdbatch->vertexBuffer);
					glBufferData(GL_ARRAY_BUFFER, convertedVtx.size() * sizeof(CUSTOMVERTEX), convertedVtx.data(), GL_STATIC_DRAW);

					ctdbatch->vertexes = convertedVtx;
				}
				else if (header.vertexFormat == 66311) {

					reader.Seek(activeVtxGroup.offsetVertexes + activeBatch.vertexStart * sizeof(WMBVertex66311));
					ctdmesh->structSize = sizeof(CUSTOMVERTEX);


				
					std::vector<WMBVertex66311> vertexes;
					for (uint32_t i = 0; i < activeBatch.numVertices; i++) {
						WMBVertex66311 vtx;
						vtx.Read(reader);
						vertexes.push_back(vtx);
					}

					std::vector<CUSTOMVERTEX> convertedVtx;
					for (WMBVertex66311& vertex : vertexes) {
						CUSTOMVERTEX cvtx;




						cvtx.x = vertex.position.x;
						cvtx.y = vertex.position.y;
						cvtx.z = vertex.position.z;
						MGRVector normals = HelperFunction::DecodeNormal(vertex.normals);
						cvtx.nx = normals.x;
						cvtx.ny = normals.y;
						cvtx.nz = normals.z;
						glm::vec4 tangents = HelperFunction::DecodeTangent(vertex.tangents);
						cvtx.tx = tangents.x;
						cvtx.ty = tangents.y;
						cvtx.tz = tangents.z;
						cvtx.tw = tangents.w;
						cvtx.u = HelperFunction::HalfToFloat(vertex.uv.u);
						cvtx.v = HelperFunction::HalfToFloat(vertex.uv.v);
						cvtx.u2 = HelperFunction::HalfToFloat(vertex.uv2.u);
						cvtx.v2 = HelperFunction::HalfToFloat(vertex.uv2.v);
						cvtx.color = 0xFFFFFFFF;
						convertedVtx.push_back(cvtx);
					}

					if (ctdbatch->vertexBuffer == 0)
						glGenBuffers(1, &ctdbatch->vertexBuffer);

					glBindBuffer(GL_ARRAY_BUFFER, ctdbatch->vertexBuffer);
					glBufferData(GL_ARRAY_BUFFER, convertedVtx.size() * sizeof(CUSTOMVERTEX), convertedVtx.data(), GL_STATIC_DRAW);

					ctdbatch->vertexes = convertedVtx;


				}
				else if (header.vertexFormat == 65799) {
					// How platinum games felt after making 2 of the EXACT SAME DAMN VERTEX GROUPS EXCEPT ONE HAS AN EXTRA UV MAP 
					reader.Seek(activeVtxGroup.offsetVertexes + activeBatch.vertexStart * sizeof(WMBVertex65799));
					ctdmesh->structSize = sizeof(CUSTOMVERTEX);


					std::vector<WMBVertex65799> vertexes;
					for (uint32_t i = 0; i < activeBatch.numVertices; i++) {
						WMBVertex65799 vtx;
						vtx.Read(reader);
						vertexes.push_back(vtx);
					}


					std::vector<CUSTOMVERTEX> convertedVtx;
					for (WMBVertex65799& vertex : vertexes) {
						CUSTOMVERTEX cvtx;

						cvtx.x = vertex.position.x;
						cvtx.y = vertex.position.y;
						cvtx.z = vertex.position.z;
						MGRVector normals = HelperFunction::DecodeNormal(vertex.normals);
						cvtx.nx = normals.x;
						cvtx.ny = normals.y;
						cvtx.nz = normals.z;
						glm::vec4 tangents = HelperFunction::DecodeTangent(vertex.tangents);
						cvtx.tx = tangents.x;
						cvtx.ty = tangents.y;
						cvtx.tz = tangents.z;
						cvtx.tw = tangents.w;
						cvtx.u = HelperFunction::HalfToFloat(vertex.uv.u);
						cvtx.v = HelperFunction::HalfToFloat(vertex.uv.v);
						cvtx.color = 0xFFFFFFFF;
						convertedVtx.push_back(cvtx);
					}

					if (ctdbatch->vertexBuffer == 0)
						glGenBuffers(1, &ctdbatch->vertexBuffer);

					glBindBuffer(GL_ARRAY_BUFFER, ctdbatch->vertexBuffer);
					glBufferData(GL_ARRAY_BUFFER, convertedVtx.size() * sizeof(CUSTOMVERTEX), convertedVtx.data(), GL_STATIC_DRAW);

					ctdbatch->vertexes = convertedVtx;


				}
				glGenVertexArrays(1, &ctdbatch->vao);

				glBindVertexArray(ctdbatch->vao);

				// Rebind VBO/IBO inside VAO scope
				glBindBuffer(GL_ARRAY_BUFFER, ctdbatch->vertexBuffer);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ctdbatch->indexBuffer);

				size_t curOffset = 0;
				// Position
				glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(CUSTOMVERTEX), (void*)curOffset);
				glEnableVertexAttribArray(0);
				curOffset += 3 * sizeof(float);

				// Color
				glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(CUSTOMVERTEX), (void*)curOffset);
				glEnableVertexAttribArray(1);
				curOffset += 4 * sizeof(byte);

				// UV map
				glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(CUSTOMVERTEX), (void*)curOffset);
				glEnableVertexAttribArray(2);
				curOffset += 2 * sizeof(float);

				// Normals
				glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(CUSTOMVERTEX), (void*)curOffset);
				glEnableVertexAttribArray(3);
				curOffset += 3 * sizeof(float);

				// Tangents
				glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(CUSTOMVERTEX), (void*)curOffset);
				glEnableVertexAttribArray(4);
				curOffset += 4 * sizeof(float);

				// Lightmap UV
				glVertexAttribPointer(5, 2, GL_FLOAT, GL_FALSE, sizeof(CUSTOMVERTEX), (void*)curOffset);
				glEnableVertexAttribArray(2);
				curOffset += 2 * sizeof(float);

				glBindVertexArray(0);

				ctdmesh->batches.push_back(ctdbatch);

			}
			displayMeshes.push_back(ctdmesh);

		}
		return;

}
	void WmbFileNode::SaveFile() {
		BinaryWriter* writer = new BinaryWriter(false);
		writer->WriteBytes(fileData);

		for (CTDMaterial& cmat : materials) {
			writer->Seek(cmat.materialDataOffset);
			for (std::array<float, 4>&array : cmat.parameters) {
				writer->WriteFloat(array[0]);
				writer->WriteFloat(array[1]);
				writer->WriteFloat(array[2]);
				writer->WriteFloat(array[3]);
			}

		}


		fileData = writer->GetData();
		delete writer;


	}


	void WmbFileNode::PopupOptions(CruelerContext *ctx) {
		auto it = std::find(openFiles.begin(), openFiles.end(), this);
		if (ImGui::BeginPopupContextItem())
		{
			if (ImGui::Button("Export (WMB)", ImVec2(150, 20))) {
				ExportFile();
			}

			/*if (ImGui::Button("Send to MGR2Blender", ImVec2(150, 20))) {



				std::ofstream script("blender_launcher.py");
				script << "import bpy\n";
				script << "bpy.ops.import_scene.wmb_data(filepath=r'" << modelPath << "', reset_blend=True)\n";
				script.close();

				std::string command = "\"" + blenderPath + "\" --python \"" + "blender_launcher.py" + "\"";
				system(command.c_str());
			}*/

			if (parent) {
				if (ImGui::Button("Delete", ImVec2(150, 20))) {

					auto it = std::find(parent->children.begin(), parent->children.end(), this);
					if (it != parent->children.end()) {
						parent->children.erase(it);
					}

					delete this;
				}


			}

			if (it != openFiles.end()) { // Ensure it exists before erasing
				if (ImGui::Button("Close", ImVec2(150, 20))) {
					closeNode(this);
				}
			}
			else {
				if (ImGui::Button("Replace", ImVec2(150, 20))) {
					ReplaceFile();
				}
			}



			//PopupOptionsEx();

			ImGui::EndPopup();
		}
	}

	void WmbFileNode::RenderMesh(CruelerContext *ctx) {
		rotationAngle += 0.01f;
		if (visualizerWireframe) {
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		}
		for (CruelerMesh* mesh : displayMeshes) {
			if (mesh->visibility) {
				for (CruelerBatch* batch : mesh->batches) {
					if (!batch->isValid) continue;

					glm::mat4 model = glm::mat4(1.0f);
					glm::mat4 view = CameraManager::Instance().GetMatrixV();
					glm::mat4 projection = CameraManager::Instance().GetMatrixP();
					glm::vec3 viewPos(0.0f, 0.0f, 10.0f);
					glm::vec3 lightPos(10.0f, 10.0f, 10.0f);


					unsigned int targetShader = 0;
					if (materials.size() > 0) {
						if (isSCR) {
							model = glm::translate(model, glm::vec3(meshOffset.x, meshOffset.y, meshOffset.z));
							glm::quat rotation = glm::quat(glm::vec3(0.0f, 0.0f, 0.0f));
							model *= glm::toMat4(rotation);
							model = glm::scale(model, glm::vec3(scaleOffset.x, scaleOffset.y, scaleOffset.z));

							if (ctx->textureMap.find(materials[batch->materialID].texture_data[0]) != ctx->textureMap.end()) {
								glDepthMask(GL_TRUE);
								glDisable(GL_BLEND);

								glUseProgram(ShaderManager::Instance().stageShader);
								targetShader = ShaderManager::Instance().stageShader;
								glActiveTexture(GL_TEXTURE0);
								glBindTexture(GL_TEXTURE_2D, ctx->textureMap[materials[batch->materialID].texture_data[0]]);
								glUniform1i(glGetUniformLocation(ShaderManager::Instance().stageShader, "diffuseMap"), 0);

								glActiveTexture(GL_TEXTURE1);
								glBindTexture(GL_TEXTURE_2D, ctx->textureMap[materials[batch->materialID].texture_data[2]]);
								glUniform1i(glGetUniformLocation(ShaderManager::Instance().stageShader, "normalMap"), 1);

								glActiveTexture(GL_TEXTURE2);
								glBindTexture(GL_TEXTURE_2D, ctx->textureMap[materials[batch->materialID].texture_data[7]]);
								glUniform1i(glGetUniformLocation(ShaderManager::Instance().stageShader, "lightingMap"), 2);

							}
							else {
								targetShader = ShaderManager::Instance().defaultShader;
								glUseProgram(ShaderManager::Instance().defaultShader);
								glActiveTexture(GL_TEXTURE0);
								glUniform1i(glGetUniformLocation(ShaderManager::Instance().defaultShader, "diffuseMap"), 0);
							}
						}
						else {
							if (ctx->textureMap.find(materials[batch->materialID].texture_data[0]) != ctx->textureMap.end()) {
								glDepthMask(GL_TRUE);
								glDisable(GL_BLEND);


								if (materials[batch->materialID].shader_name == "ois02_sbxeX" || materials[batch->materialID].shader_name == "ois02_xbceX" || materials[batch->materialID].shader_name == "ois02_sbceX" || materials[batch->materialID].shader_name == "siv23_sbxxx" || materials[batch->materialID].shader_name == "sis23_xbxex" || materials[batch->materialID].shader_name == "ois01_xbxeX") {
									glUseProgram(ShaderManager::Instance().decalShader);
									targetShader = ShaderManager::Instance().decalShader;
									glActiveTexture(GL_TEXTURE0);
									glBindTexture(GL_TEXTURE_2D, ctx->textureMap[materials[batch->materialID].texture_data[0]]);
									glUniform1i(glGetUniformLocation(ShaderManager::Instance().decalShader, "diffuseMap"), 0);
									glEnable(GL_BLEND);
									glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
								}
								else if (materials[batch->materialID].shader_name.find("skn") != std::string::npos) {
									glUseProgram(ShaderManager::Instance().skinShader);
									targetShader = ShaderManager::Instance().skinShader;
									glActiveTexture(GL_TEXTURE0);
									glBindTexture(GL_TEXTURE_2D, ctx->textureMap[materials[batch->materialID].texture_data[0]]);
									glUniform1i(glGetUniformLocation(ShaderManager::Instance().skinShader, "diffuseMap"), 0);

									glActiveTexture(GL_TEXTURE1);
									glBindTexture(GL_TEXTURE_2D, ctx->textureMap[materials[batch->materialID].texture_data[2]]);
									glUniform1i(glGetUniformLocation(ShaderManager::Instance().skinShader, "normalMap"), 1);
								}
								else {
									glUseProgram(ShaderManager::Instance().defaultShader);
									targetShader = ShaderManager::Instance().defaultShader;
									glActiveTexture(GL_TEXTURE0);
									glBindTexture(GL_TEXTURE_2D, ctx->textureMap[materials[batch->materialID].texture_data[0]]);
									glUniform1i(glGetUniformLocation(ShaderManager::Instance().defaultShader, "diffuseMap"), 0);

									glActiveTexture(GL_TEXTURE1);
									glBindTexture(GL_TEXTURE_2D, ctx->textureMap[materials[batch->materialID].texture_data[2]]);
									glUniform1i(glGetUniformLocation(ShaderManager::Instance().defaultShader, "normalMap"), 1);


								}
							}
							else {
								targetShader = ShaderManager::Instance().defaultShader;
								glUseProgram(ShaderManager::Instance().defaultShader);
								glActiveTexture(GL_TEXTURE0);
								glBindTexture(GL_TEXTURE_2D, ctx->textureMap[0]);
								glUniform1i(glGetUniformLocation(ShaderManager::Instance().defaultShader, "diffuseMap"), 0);
							}
						}
					}


					glUniformMatrix4fv(glGetUniformLocation(targetShader, "model"), 1, GL_FALSE, glm::value_ptr(model));
					glUniformMatrix4fv(glGetUniformLocation(targetShader, "view"), 1, GL_FALSE, glm::value_ptr(view));
					glUniformMatrix4fv(glGetUniformLocation(targetShader, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
					glm::vec3 lightDir = glm::normalize(glm::vec3(0, -1.0f, -1.0f));
					glUniform3fv(glGetUniformLocation(targetShader, "lightDir"), 1, glm::value_ptr(lightDir));
					glUniform3fv(glGetUniformLocation(targetShader, "viewPos"), 1, glm::value_ptr(viewPos));

					glBindVertexArray(batch->vao); 

					glBindBuffer(GL_ARRAY_BUFFER, batch->vertexBuffer);
					glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, batch->indexBuffer);

					

					glDrawElements(batch->drawMethod, batch->indexCount, GL_UNSIGNED_SHORT, 0);

					if (materials.size() > 0) {
						if (materials[batch->materialID].highlight) {
							targetShader = ShaderManager::Instance().outlineShader;
							glUseProgram(ShaderManager::Instance().outlineShader);
							glEnable(GL_CULL_FACE);
							glCullFace(GL_FRONT);

							glUniformMatrix4fv(glGetUniformLocation(targetShader, "model"), 1, GL_FALSE, glm::value_ptr(model));
							glUniformMatrix4fv(glGetUniformLocation(targetShader, "view"), 1, GL_FALSE, glm::value_ptr(view));
							glUniformMatrix4fv(glGetUniformLocation(targetShader, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
							glUniform3fv(glGetUniformLocation(targetShader, "lightDir"), 1, glm::value_ptr(lightDir));
							glUniform3fv(glGetUniformLocation(targetShader, "viewPos"), 1, glm::value_ptr(viewPos));

							glBindVertexArray(batch->vao);

							glBindBuffer(GL_ARRAY_BUFFER, batch->vertexBuffer);
							glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, batch->indexBuffer);

							glDrawElements(batch->drawMethod, batch->indexCount, GL_UNSIGNED_SHORT, 0);

							glCullFace(GL_BACK);
							glDisable(GL_CULL_FACE);
						}
					}



				}

			}



		}
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		static ImDrawList* drawList = new ImDrawList(ImGui::GetDrawListSharedData());
		drawList->_ResetForNewFrame();
		drawList->PushClipRectFullScreen();
		drawList->PushTextureID(ImGui::GetIO().Fonts->TexID);

		glm::mat4 viewMatrix = CameraManager::Instance().GetMatrixV();
		glm::mat4 projMatrix = CameraManager::Instance().GetMatrixP();
		GLint viewport[4];
		glGetIntegerv(GL_VIEWPORT, viewport);

		for (size_t i = 0; i < bones.size(); ++i)
		{
			int parentIdx = bones[i].parentIndex;
			if (parentIdx < 0)
				continue;

			glm::vec3 bonePos = glm::vec3(bones[i].combinedTransform[3]);
			glm::vec3 parentPos = glm::vec3(bones[parentIdx].combinedTransform[3]);


			glm::vec2 screenA, screenB;
			bool visibleA = HelperFunction::WorldToScreen(glm::vec3(bonePos), screenA, viewMatrix, projMatrix, viewport);
			bool visibleB = HelperFunction::WorldToScreen(glm::vec3(parentPos), screenB, viewMatrix, projMatrix, viewport);

			if (visibleA && visibleB)
			{
				if (bones[i].selected)
				{
					drawList->AddLine(ImVec2(screenA.x, screenA.y + 335), ImVec2(screenB.x, screenB.y + 335), IM_COL32(255, 255, 0, 255), 2.0f);
					drawList->AddCircle(ImVec2(screenA.x, screenA.y + 335), 4.0f, IM_COL32(255, 255, 0, 255));
				}
				else
				{
					if (visualizerShowBones) {
						drawList->AddLine(ImVec2(screenA.x, screenA.y + 335), ImVec2(screenB.x, screenB.y + 335), IM_COL32(255, 0, 0, 255), 2.0f);
					}
					
				}
			}
		}

		ImVector<ImDrawList*> lists;
		lists.push_back(drawList);
		static ImDrawData drawData = ImDrawData();
		drawData.DisplayPos = ImGui::GetMainViewport()->Pos;
		drawData.DisplaySize = ImGui::GetMainViewport()->Size;
		drawData.FramebufferScale = ImVec2(1.0f, 1.0f);
		drawData.CmdLists = lists;
		drawData.CmdListsCount = 1;
		drawData.TotalIdxCount = drawList->IdxBuffer.size();
		drawData.TotalVtxCount = drawList->VtxBuffer.size();
		drawData.Valid = true;

		ImGui_ImplOpenGL3_RenderDrawData(&drawData);
	}

	void WmbFileNode::RenderPreviewMesh(CruelerContext* ctx)
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		for (CruelerMesh* mesh : displayMeshes) {
			if (mesh->visibility) {
				for (CruelerBatch* batch : mesh->batches) {
					if (!batch->isValid) continue;

					glm::mat4 model = glm::mat4(1.0f);
					glm::vec3 camPos = glm::vec3(0.0f, 0.0f, 1.0f);
					glm::vec3 target = glm::vec3(0.0f, 0.0f, 0.0f);  
					glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f); 

					glm::mat4 view = glm::lookAt(camPos, target, up);


					float fov = glm::radians(45.0f);
					float aspectRatio = 1.0f; 
					float nearPlane = 0.1f;
					float farPlane = 100.0f;

					glm::mat4 projection = glm::perspective(fov, aspectRatio, nearPlane, farPlane);
					glm::vec3 viewPos(0.0f, 0.0f, 10.0f);
					glm::vec3 lightPos(10.0f, 10.0f, 10.0f);


					unsigned int targetShader = 0;

					if (isSCR) {
						model = glm::translate(model, glm::vec3(meshOffset.x, meshOffset.y, meshOffset.z));
						glm::quat rotation = glm::quat(glm::vec3(0.0f, 0.0f, 0.0f));
						model *= glm::toMat4(rotation);
						model = glm::scale(model, glm::vec3(scaleOffset.x, scaleOffset.y, scaleOffset.z));

						if (ctx->textureMap.find(materials[batch->materialID].texture_data[0]) != ctx->textureMap.end()) {
							glDepthMask(GL_TRUE);
							glDisable(GL_BLEND);

							glUseProgram(ShaderManager::Instance().stageShader);
							targetShader = ShaderManager::Instance().stageShader;
							glActiveTexture(GL_TEXTURE0);
							glBindTexture(GL_TEXTURE_2D, ctx->textureMap[materials[batch->materialID].texture_data[0]]);
							glUniform1i(glGetUniformLocation(ShaderManager::Instance().stageShader, "diffuseMap"), 0);

							glActiveTexture(GL_TEXTURE1);
							glBindTexture(GL_TEXTURE_2D, ctx->textureMap[materials[batch->materialID].texture_data[2]]);
							glUniform1i(glGetUniformLocation(ShaderManager::Instance().stageShader, "normalMap"), 1);

							glActiveTexture(GL_TEXTURE2);
							glBindTexture(GL_TEXTURE_2D, ctx->textureMap[materials[batch->materialID].texture_data[7]]);
							glUniform1i(glGetUniformLocation(ShaderManager::Instance().stageShader, "lightingMap"), 2);

						}
						else {
							targetShader = ShaderManager::Instance().defaultShader;
							glUseProgram(ShaderManager::Instance().defaultShader);
							glActiveTexture(GL_TEXTURE0);
							glUniform1i(glGetUniformLocation(ShaderManager::Instance().defaultShader, "diffuseMap"), 0);
						}
					}
					else {
						if (ctx->textureMap.find(materials[batch->materialID].texture_data[0]) != ctx->textureMap.end()) {
							glDepthMask(GL_TRUE);
							glDisable(GL_BLEND);


							if (materials[batch->materialID].shader_name == "ois02_sbxeX" || materials[batch->materialID].shader_name == "ois02_xbceX" || materials[batch->materialID].shader_name == "ois02_sbceX" || materials[batch->materialID].shader_name == "siv23_sbxxx" || materials[batch->materialID].shader_name == "sis23_xbxex" || materials[batch->materialID].shader_name == "ois01_xbxeX") {
								glUseProgram(ShaderManager::Instance().decalShader);
								targetShader = ShaderManager::Instance().decalShader;
								glActiveTexture(GL_TEXTURE0);
								glBindTexture(GL_TEXTURE_2D, ctx->textureMap[materials[batch->materialID].texture_data[0]]);
								glUniform1i(glGetUniformLocation(ShaderManager::Instance().decalShader, "diffuseMap"), 0);
								glEnable(GL_BLEND);
								glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
							}
							else if (materials[batch->materialID].shader_name.find("skn") != std::string::npos) {
								glUseProgram(ShaderManager::Instance().skinShader);
								targetShader = ShaderManager::Instance().skinShader;
								glActiveTexture(GL_TEXTURE0);
								glBindTexture(GL_TEXTURE_2D, ctx->textureMap[materials[batch->materialID].texture_data[0]]);
								glUniform1i(glGetUniformLocation(ShaderManager::Instance().skinShader, "diffuseMap"), 0);

								glActiveTexture(GL_TEXTURE1);
								glBindTexture(GL_TEXTURE_2D, ctx->textureMap[materials[batch->materialID].texture_data[2]]);
								glUniform1i(glGetUniformLocation(ShaderManager::Instance().skinShader, "normalMap"), 1);
							}
							else {
								glUseProgram(ShaderManager::Instance().defaultShader);
								targetShader = ShaderManager::Instance().defaultShader;
								glActiveTexture(GL_TEXTURE0);
								glBindTexture(GL_TEXTURE_2D, ctx->textureMap[materials[batch->materialID].texture_data[0]]);
								glUniform1i(glGetUniformLocation(ShaderManager::Instance().defaultShader, "diffuseMap"), 0);

								glActiveTexture(GL_TEXTURE1);
								glBindTexture(GL_TEXTURE_2D, ctx->textureMap[materials[batch->materialID].texture_data[2]]);
								glUniform1i(glGetUniformLocation(ShaderManager::Instance().defaultShader, "normalMap"), 1);


							}
						}
						else {
							targetShader = ShaderManager::Instance().defaultShader;
							glUseProgram(ShaderManager::Instance().defaultShader);
							glActiveTexture(GL_TEXTURE0);
							glBindTexture(GL_TEXTURE_2D, ctx->textureMap[0]);
							glUniform1i(glGetUniformLocation(ShaderManager::Instance().defaultShader, "diffuseMap"), 0);
						}
					}





					glUniformMatrix4fv(glGetUniformLocation(targetShader, "model"), 1, GL_FALSE, glm::value_ptr(model));
					glUniformMatrix4fv(glGetUniformLocation(targetShader, "view"), 1, GL_FALSE, glm::value_ptr(view));
					glUniformMatrix4fv(glGetUniformLocation(targetShader, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
					glm::vec3 lightDir = glm::normalize(glm::vec3(0, -1.0f, -1.0f)); 
					glUniform3fv(glGetUniformLocation(targetShader, "lightDir"), 1, glm::value_ptr(lightDir));
					glUniform3fv(glGetUniformLocation(targetShader, "viewPos"), 1, glm::value_ptr(viewPos));

					glBindVertexArray(batch->vao);

					glBindBuffer(GL_ARRAY_BUFFER, batch->vertexBuffer);
					glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, batch->indexBuffer);



					glDrawElements(GL_TRIANGLES, batch->indexCount, GL_UNSIGNED_SHORT, 0);

				}

			}



		}
	}

	void WmbFileNode::RemoveCuttingDataWMB4()
	{
		BinaryWriter* writer = new BinaryWriter(false);
		writer->WriteBytes(fileData);
		writer->Seek(108);
		writer->WriteUINT32(0);
		hasCutdata = false;
		fileData = writer->GetData();
		delete writer;
	}

	bool WmbFileNode::InitData(int rootBoneID) {
		bonePhysData.rootBone = CruelerBone();
		bonePhysData.rootBoneIndex = 0;
		bool rootFound = false;

		for (int i = 0; i < bones.size(); ++i)
		{
			if (bones[i].boneID == rootBoneID) {
				bonePhysData.rootBone = bones[i];
				rootFound = true;
				bonePhysData.rootBoneIndex = i;
				break;
			}
		}

		if (!rootFound) return false;

		bonePhysData.boneChildren.clear();

		for (int i = 0; i < bones.size(); ++i)
		{
			int parent = bones[i].parentIndex;
			if (parent >= 0)
			{
				bonePhysData.boneChildren[parent].push_back(i);
			}
		}

		bonePhysData.boneRunLength = bonePhysData.boneChildren[bonePhysData.rootBoneIndex][1] - bonePhysData.boneChildren[bonePhysData.rootBoneIndex][0]; // Length of each run
		bonePhysData.boneRunCounts = bonePhysData.boneChildren[bonePhysData.rootBoneIndex].size(); // Number of individual "skirt" runs

		bonePhysData.boneStartIndiceOffset = bonePhysData.boneChildren[bonePhysData.rootBoneIndex][0];


		

		return true;

	}

	void WmbFileNode::GenerateData() {

		int totalCount = bonePhysData.boneRunCounts * bonePhysData.boneRunLength;
		int internalOffset = 0;
		int currentChainIndex = 0;
		int tiebackSideID = 0; // For open jackets
		std::reverse(bonePhysData.boneOrder.begin(), bonePhysData.boneOrder.end());
		for (int boneOrderedIdx : bonePhysData.boneOrder) {
			BoneClothWK hdrWK = BoneClothWK();
			int lastBoneDownIndex = 0;
			hdrWK.no = bones[boneOrderedIdx].boneID;
			if (bonePhysData.boneChildren[boneOrderedIdx].size() > 0) {
				hdrWK.noDown = bones[bonePhysData.boneChildren[boneOrderedIdx][0]].boneID;
				lastBoneDownIndex = bonePhysData.boneChildren[boneOrderedIdx][0];
			}
			else {
				hdrWK.noDown = 4095;
				lastBoneDownIndex = 4095;
			}
			hdrWK.noUp = 4095;
			int hdrwindedID = 4095;
			if (currentChainIndex + 1 < bonePhysData.boneOrder.size()) {
				hdrwindedID = bones[bonePhysData.boneOrder[currentChainIndex + 1]].boneID;
			}
			else {
				hdrwindedID = tiebackSideID;
			}
			hdrWK.noSide = hdrwindedID;
			hdrWK.noPoly = hdrwindedID;
			
			for (int x = 0; x < bonePhysData.boneRunLength - 1; x++) {
				BoneClothWK tmpWK = BoneClothWK();
				


				tmpWK.noUp = bones[lastBoneDownIndex - 1].boneID;
				tmpWK.no = bones[lastBoneDownIndex].boneID;

				if (bonePhysData.boneChildren[lastBoneDownIndex].size() > 0) {
					tmpWK.noDown = bones[bonePhysData.boneChildren[lastBoneDownIndex][0]].boneID;
				}
				else {
					tmpWK.noDown = 4095;
				}
				int chainOffset = hdrWK.no - tmpWK.no;
				int windedID = 4095;
				if (currentChainIndex + 1 < bonePhysData.boneOrder.size()) {
					windedID = bones[bonePhysData.boneOrder[currentChainIndex + 1] + chainOffset].boneID;
					tmpWK.noSide = windedID;
					tmpWK.noPoly = windedID;
					tiebackSideID = tmpWK.noSide;
				}
				else {
					windedID = tiebackSideID + chainOffset;
					tmpWK.noSide = windedID;
					tmpWK.noPoly = windedID;
					tiebackSideID = tmpWK.noSide;
				}
				


				lastBoneDownIndex++;
				bonePhysData.boneClothWorks.push_back(tmpWK);
			}


			bonePhysData.boneClothWorks.push_back(hdrWK);
			currentChainIndex += 1;

		}

		std::ofstream clothWKDbg;
		clothWKDbg.open("clothWKDebug.xml");
		clothWKDbg << "<Root>\n";
		for (BoneClothWK& wk : bonePhysData.boneClothWorks) {
			clothWKDbg << "\t<CLOTH_WK>\n";
			clothWKDbg << "\t\t<no>" << wk.no << "</no>\n";
			clothWKDbg << "\t\t<noUp>" << wk.noUp << "</noUp>\n";
			clothWKDbg << "\t\t<noDown>" << wk.noDown << "</noDown>\n";
			clothWKDbg << "\t\t<noSide>" << wk.noSide << "</noSide>\n";
			clothWKDbg << "\t\t<noPoly>" << wk.noPoly << "</noPoly>\n";
			clothWKDbg << "\t\t<noFix>" << 4095 << "</noFix>\n";
			clothWKDbg << "\t\t<rotLimit>" << wk.rotLimit << "</rotLimit>\n";
			clothWKDbg << "\t\t<offset>" << wk.offset.x << " " << wk.offset.y << " " << wk.offset.z << "< / offset>\n";
			clothWKDbg << "\t\t<m_OriginalRate>" << wk.m_OriginalRate << "</m_OriginalRate>\n";
			clothWKDbg << "\t</CLOTH_WK>\n";
		}
		clothWKDbg << "</Root>\n";


		clothWKDbg.close();

		std::reverse(bonePhysData.boneOrder.begin(), bonePhysData.boneOrder.end());

		return;
	}

	void WmbFileNode::PhysicsPanel() {

		if (arePhysicsInitalized) {
			for (int i = 0; i < bonePhysData.boneRunCounts; i++) {
				bones[bonePhysData.boneStartIndiceOffset + bonePhysData.boneRunLength * i].selected = true;
			}


			if (bonePhysData.boneOrder.empty())
			{
				for (int i = 0; i < bonePhysData.boneRunCounts; i++) {
					bonePhysData.boneOrder.push_back(bonePhysData.boneStartIndiceOffset + bonePhysData.boneRunLength * i);
				}
			}

			for (int i = 0; i < bonePhysData.boneOrder.size(); ++i)
			{
				std::string label = ((std::string(ICON_CI_THREE_BARS) + "bone") + std::to_string(bones[bonePhysData.boneOrder[i]].boneID));

				ImGui::Selectable(label.c_str(), false, ImGuiSelectableFlags_AllowDoubleClick);

				if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
				{
					ImGui::SetDragDropPayload("BONE_DRAG", &i, sizeof(int));
					ImGui::Text("Move %s", label.c_str());
					ImGui::EndDragDropSource();
				}

				// Drop target
				if (ImGui::BeginDragDropTarget())
				{
					if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("BONE_DRAG"))
					{
						int srcIndex = *(const int*)payload->Data;
						if (srcIndex != i)
						{
							std::swap(bonePhysData.boneOrder[i], bonePhysData.boneOrder[srcIndex]);
						}
					}
					ImGui::EndDragDropTarget();
				}
			}



			if (ImGui::Button("Generate Physics")) {
				GenerateData();
			}
			if (bonePhysData.boneClothWorks.size() > 0) {
				if (ImGui::CollapsingHeader("Cloth Works (Debug)")) {
					int i = 0;
					for (BoneClothWK& clothWK : bonePhysData.boneClothWorks) {
						i += 1;
						ImGui::SeparatorText(("Work #" + std::to_string(i)).c_str());
						ImGui::Text("no: %d", clothWK.no);
						ImGui::Text("noDown: %d noUp: %d", clothWK.noDown, clothWK.noUp);
						ImGui::Text("noSide: %d noPoly: %d", clothWK.noSide, clothWK.noPoly);

					}
				}
			}





			if (ImGui::Button("Clear Data")) {
				for (int i = 0; i < bonePhysData.boneRunCounts; i++) {
					ImGui::Text("Run %d: BoneID: %d", i, bones[bonePhysData.boneStartIndiceOffset + bonePhysData.boneRunLength * i].boneID);
					bones[bonePhysData.boneStartIndiceOffset + bonePhysData.boneRunLength * i].selected = false;
				}
				arePhysicsInitalized = false;
			}
		}
		else {

			if (ImGui::Button("Initalize Data")) {
				arePhysicsInitalized = InitData(4093);;
			}
		}

	}

	void WmbFileNode::DrawBoneTree(int boneIdx) {
		std::string boneName = GetBoneNameFromID(bones[boneIdx].boneID);
		bool isSelected = bones[boneIdx].selected;

		ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;
		if (isSelected)
			flags |= ImGuiTreeNodeFlags_Selected;

		bool nodeOpen = ImGui::TreeNodeEx((void*)(intptr_t)boneIdx, flags, boneName.c_str());

		if (ImGui::IsItemClicked()) {
			for (auto& b : bones) b.selected = false;
			bones[boneIdx].selected = true;
		}

		if (nodeOpen) {
			for (int i = 0; i < bones.size(); i++) {
				if (bones[i].parentIndex == boneIdx) {
					DrawBoneTree(i);
				}
			}
			ImGui::TreePop();
		}
	}

	void WmbFileNode::RenderBoneGUI() {
		if (ImGui::BeginTabBar("bone_sub")) {
			if (ImGui::BeginTabItem("Armature")) {
				ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.1f, 0.1f, 0.1f, 0.0f));
				ImGui::BeginChild("BoneSidebar", ImVec2(325, 0));
				ImGui::Text("Bone Names: %s", boneNameSourceFile.c_str());


				for (int i = 0; i < bones.size(); ++i) {
					if (bones[i].parentIndex < 0) {
						DrawBoneTree(i);
					}
				}
				ImGui::EndChild();
				ImGui::PopStyleColor();
				ImGui::EndTabItem();
			}
			if (parent) {
				for (FileNode* fNode : parent->children) {
					if (fNode->nodeType == MOT) {
						if (ImGui::BeginTabItem("Animation")) {
							ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.1f, 0.1f, 0.1f, 0.0f));
							ImGui::BeginChild("AnimSidebar", ImVec2(325, 0));

							for (FileNode* fNode : parent->children) {
								if (fNode->nodeType == MOT) {
									ImGui::Text(fNode->fileName.c_str());
								}
							}



							ImGui::EndChild();
							ImGui::PopStyleColor();
							ImGui::EndTabItem();
						};
						break;
					}
				}
			}



			if (ImGui::BeginTabItem("Physics")) {
				ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.1f, 0.1f, 0.1f, 0.0f));
				ImGui::BeginChild("PhysicsSidebar", ImVec2(325, 0));

				PhysicsPanel();

				ImGui::EndChild();
				ImGui::PopStyleColor();
				ImGui::EndTabItem();
			}



			ImGui::EndTabBar();
		}


	}


	void WmbFileNode::RenderGUI(CruelerContext *ctx) {
		ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.1f, 0.1f, 0.1f, 0.0f));
		ImGui::PushStyleColor(ImGuiCol_ScrollbarBg, ImVec4(0.1f, 0.1f, 0.1f, 0.0f));
		ImGui::PushStyleColor(ImGuiCol_ScrollbarGrab, ImVec4(0.1f, 0.1f, 0.1f, 0.5f));
		ImGui::BeginChild("MeshSidebar", ImVec2(200, 0));
		if (isSCR) {
			ImGui::Text("WMB Meshes - %s", fileName.c_str());
		}
		else {
			ImGui::Text("WMB Meshes");
		}

		int i = 0;
		for (CruelerMesh* mesh : displayMeshes) {
			i += 1;
			ImGui::Text("%s. ", (std::to_string(i)).c_str());
			ImGui::SameLine();
			ImGui::Checkbox((mesh->name).c_str(), &mesh->visibility);



		}
		ImGui::EndChild();
		ImGui::PopStyleColor(3);
	}

	void SCRMesh::Read(BinaryReader& br) {
		offset = br.ReadUINT32();
		name = br.ReadString(64);
		position.x = br.ReadFloat();
		position.y = br.ReadFloat();
		position.z = br.ReadFloat();
		rotation.x = br.ReadFloat();
		rotation.y = br.ReadFloat();
		rotation.z = br.ReadFloat();
		scale.x = br.ReadFloat();
		scale.y = br.ReadFloat();
		scale.z = br.ReadFloat();

	}


	ScrFileNode::ScrFileNode(std::string fName) : FileNode(fName) {
		fileIcon = ICON_CI_LAYERS;
		TextColor = { 1.0f, 0.0f, 1.0f, 1.0f };
		nodeType = MOT;
		fileFilter = L"Platinum Stage File(*.scr)\0*.scr;\0";
	}
	void ScrFileNode::LoadFile() {
		BinaryReader reader(fileData, true);
		reader.SetEndianess(fileIsBigEndian);

		reader.Seek(0x6);

		int modelCount = reader.ReadUINT16();
		int offsetModelDef = reader.ReadUINT32();

		reader.Seek(offsetModelDef);
		std::vector<unsigned int> meshOffsets;

		for (int i = 0; i < modelCount; i++) {
			meshOffsets.push_back(reader.ReadUINT32());
		}

		std::vector<SCRMesh> meshes;
		for (int offset : meshOffsets) {
			reader.Seek(offset);
			SCRMesh mesh;
			mesh.Read(reader);
			meshes.push_back(mesh);
		}
		for (int i = 0; i < modelCount; i++) {
			reader.Seek(meshes[i].offset);
			int size = 0;
			if (i != meshes.size() - 1) {
				size = meshes[i + 1].offset - meshes[i].offset;
			}
			else {
				size = static_cast<int>(fileData.size()) - meshes[i].offset;
			}

			WmbFileNode* node = new WmbFileNode(std::string(meshes[i].name));
			node->isSCR = true;
			node->scrNode = this;
			node->meshOffset = meshes[i].position;
			node->scaleOffset = meshes[i].scale;
			node->meshRotation = meshes[i].rotation;
			node->fileIsBigEndian = fileIsBigEndian;
			node->SetFileData(reader.ReadBytes(size));
			node->LoadFile();
			children.push_back(node);
		}

	}
	void ScrFileNode::SaveFile() {

	}


WWISE::Data002BlobData* Data002Blob = nullptr;

	BnkFileNode::BnkFileNode(std::string fName) : FileNode(fName) {
		fileIcon = ICON_CI_PACKAGE;
		TextColor = { 0.0f, 0.529f, 0.724f, 1.0f };
		nodeType = BNK;
		fileFilter = L"WWISE Audio Bank v72(*.bnk)\0*.bnk;\0";
	}

	void BnkFileNode::LoadFile() {
		if (!Data002Blob) {
			Data002Blob = new WWISE::Data002BlobData();
			Data002Blob->Load();
		}
		

		BinaryReader reader(fileData, fileIsBigEndian);
		reader.Seek(0x4);
		int headerLength = reader.ReadUINT32();
		int wwiseVersion = reader.ReadUINT32();


		if (wwiseVersion != 72) {
			if (wwiseVersion == 1207959552) {
				reader.SetEndianess(true);
				reader.Seek(0x4);
				headerLength = reader.ReadUINT32();
				wwiseVersion = reader.ReadUINT32();
			}
			else {
				loadFailed = true;
				CTDLog::Log::getInstance().LogError("BNK failed version check! (Value at 0x10 isn't 72 but instead is " + std::to_string(wwiseVersion) + ")");
				return;
			}



		}




		reader.Seek(0x8 + headerLength);
		while (!reader.EndOfBuffer()) {
			std::string chunkType = reader.ReadString(4);
			size_t chunkSize = reader.ReadUINT32();
			size_t nextChunkPosition = reader.Tell() + chunkSize;

			if (chunkType == "DIDX") {
				std::vector<int> wemIDs;
				std::vector<int> wemOffsets;
				std::vector<int> wemSizes;
				if (chunkSize > 0) {
					int wemCount = static_cast<int>(chunkSize / 12);
					for (int i = 0; i < wemCount; i++) {
						wemIDs.push_back(reader.ReadUINT32());
						wemOffsets.push_back(reader.ReadUINT32());
						wemSizes.push_back(reader.ReadUINT32());
					}
					reader.ReadUINT32();
					reader.ReadUINT32();
					size_t baseOffset = reader.Tell();
					// The DATA chunk *should* always be after DIDX, if it isn't uhhh... kick sand?

					for (int i = 0; i < wemCount; i++) {
						reader.Seek(wemOffsets[i] + baseOffset);
						std::string wemName = std::to_string(wemIDs[i]) + ".wem";
						FileNode* childNode = HelperFunction::LoadNode(wemName, reader.ReadBytes(wemSizes[i]), false, false);
						if (childNode) {
							children.push_back(childNode);
						}

					}
				}
			}
			if (chunkType == "HIRC") {
				// Chunk lore
				int childrenCount = reader.ReadUINT32();
				for (int i = 0; i < childrenCount; i++) {
					int type = reader.ReadINT8();
					unsigned int size = reader.ReadUINT32();
					size_t nextHircChunkPosition = reader.Tell() + size;
					unsigned int uid = reader.ReadUINT32();
					


					if (type == 0x4) {
						BnkEventObject evnObj{ static_cast<uint8_t>(type), size, uid };
						int eventCounts = reader.ReadUINT32();
						for (int i = 0; i < eventCounts; i++) {
							evnObj.ids.push_back(reader.ReadUINT32());
						}
						
						hircObjects.push_back(evnObj);
					}


					reader.Seek(nextHircChunkPosition);
				}



			}


			reader.Seek(nextChunkPosition);

		}


	}

	std::string BnkFileNode::GetBNKHircID(BnkHircObject obj) {
		if (Data002Blob->wwiseHircObjectIDs.find(obj.uid) != Data002Blob->wwiseHircObjectIDs.end()) {
			return Data002Blob->wwiseHircObjectIDs[obj.uid];
		}
		else {
			return std::to_string(obj.uid);
		}
	}

	void BnkFileNode::RenderGUI(CruelerContext *ctx) {
		if (ImGui::BeginTabBar("bnk_tab")) {
			if (ImGui::BeginTabItem("Audio")) {


				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("Events")) {
				if (ImGui::TreeNode("Events")) {
					if (ImGui::TreeNode("Default Work Unit")) {
						for (BnkHircObject hirc : hircObjects) {
							if (hirc.type == 0x4) {
								if (ImGui::TreeNodeEx((GetBNKHircID(hirc)).c_str(), ImGuiTreeNodeFlags_Bullet)) {
									ImGui::TreePop();
								}
							}
						}
						ImGui::TreePop();
					}
					ImGui::TreePop();
				}




				ImGui::EndTabItem();
			}
			ImGui::EndTabBar();
		}


	}

	void BnkFileNode::SaveFile() {
		bool shouldRepack = false;
		for (FileNode* node : children) {
			if (node->isEdited) {
				shouldRepack = true;
				break;
			}
		}
		if (!shouldRepack) return;

		std::unordered_map<std::string, std::vector<char>> chunk_datas;
		BinaryReader reader(fileData, fileIsBigEndian);
		while (true) {

			if (reader.EndOfBuffer()) break;

			std::string chunkID = reader.ReadString(4);
			uint32_t chunkSize = reader.ReadUINT32();
			chunk_datas[chunkID] = reader.ReadBytes(chunkSize);

		}


		BinaryWriter* writer = new BinaryWriter(fileIsBigEndian);
		uint32_t totalDataChunkSize = 0;
		for (const auto& data : chunk_datas) {
			if (data.first + "\0" != "DATA" && data.first + "\0" != "DIDX") {
				writer->WriteString(data.first);
				writer->WriteUINT32(data.second.size());
				writer->WriteBytes(data.second);
			}
			else if (data.first == "DIDX") {
				writer->WriteString(data.first);
				writer->WriteUINT32(children.size() * 12);
				uint32_t offsetTicker = 0;
				for (FileNode* node : children) {
					uint32_t wemID = 0;
					size_t extensionsplitter = node->fileName.find_last_of(".");
					if (extensionsplitter != std::string::npos) {
						wemID = std::stoi(node->fileName.substr(0, extensionsplitter));
					}
					
					writer->WriteUINT32(wemID);
					writer->WriteUINT32(offsetTicker);
					writer->WriteUINT32(node->fileData.size());
					offsetTicker += node->fileData.size();


				}
				totalDataChunkSize = offsetTicker;
			}
			else if (data.first == "DATA") {
				writer->WriteString(data.first);
				writer->WriteUINT32(totalDataChunkSize);
				for (FileNode* node : children) {
					writer->WriteBytes(node->fileData);
				}

			}
		}
		
		fileData = writer->GetData();
		delete writer;

	}

	DatFileNode::DatFileNode(std::string fName) : FileNode(fName) {
		fileIcon = ICON_CI_FOLDER;
		TextColor = { 0.0f, 0.98f, 0.467f, 1.0f };
		nodeType = DAT;
		fileFilter = L"Platinum File Container(*.dat, *.dtt, *.eff, *.evn, *.eft)\0*.dat;*.dtt;*.eff;*.evn;*.eft;\0";
	}

	void DatFileNode::LoadFile() {
		BinaryReader reader(fileData, false);
		reader.Seek(0x4);
		if (reader.ReadUINT32() > 1000000) {
			std::string logName = fileName;
			logName.pop_back();
			CTDLog::Log::getInstance().LogNote("Crueler has decided that " + logName + " is a big endian file! (Xbox 360/PS3)");
			fileIsBigEndian = true;
			reader.SetEndianess(true);
		}
		reader.Seek(0x4);
		unsigned int FileCount = reader.ReadUINT32();
		unsigned int PositionsOffset = reader.ReadUINT32();
		reader.Skip(sizeof(unsigned int)); // TODO: ExtensionsOffset
		unsigned int NamesOffset = reader.ReadUINT32();
		unsigned int SizesOffset = reader.ReadUINT32();
		reader.Skip(sizeof(unsigned int)); // TODO: HashMapOffset

		reader.Seek(PositionsOffset);
		std::vector<int> offsets;
		for (unsigned int f = 0; f < FileCount; f++) {
			offsets.push_back(reader.ReadUINT32());
		}

		reader.Seek(NamesOffset);
		int nameLength = reader.ReadUINT32();
		std::vector<std::string> names;
		for (unsigned int f = 0; f < FileCount; f++) {
			std::string temp_name = reader.ReadString(nameLength);
			temp_name.erase(std::remove(temp_name.begin(), temp_name.end(), '\0'), temp_name.end());
			names.push_back(temp_name);
		}

		reader.Seek(SizesOffset);
		std::vector<int> sizes;
		for (unsigned int f = 0; f < FileCount; f++) {
			sizes.push_back(reader.ReadUINT32());
		}

		for (unsigned int f = 0; f < FileCount; f++) {
			reader.Seek(offsets[f]);
			FileNode* childNode = HelperFunction::LoadNode(names[f], reader.ReadBytes(sizes[f]), fileIsBigEndian, fileIsBigEndian);
			childNode->parent = this;
			if (childNode) {
				children.push_back(childNode);
			}

		}


	}



	void DatFileNode::SaveFile() {
		std::cout << "Saving DAT file: " << fileName << std::endl;

		auto start_noio = std::chrono::high_resolution_clock::now();




		int longestName = 0;

		for (FileNode* child : children) {
			child->SaveFile();
			if (child->fileName.length() > longestName) {
				longestName = static_cast<int>(child->fileName.length() + 1);
			}
		}

		CRC32 crc32;

		std::vector<std::string> fileNames;
		for (FileNode* node : children) {
			fileNames.push_back(node->fileName);
		}

		int shift = std::min(31, 32 - IntLength(static_cast<int>(fileNames.size())));
		int bucketSize = 1 << (31 - shift);

		std::vector<short> bucketTable(bucketSize, -1);

		std::vector<std::pair<int, short>> hashTuple;
		for (int i = 0; i < fileNames.size(); ++i) {
			//int hashValue = crc32.HashToUInt32(fileNames[i]) & 0x7FFFFFFF;
			int hashValue = ComputeHash(fileNames[i], crc32);
			hashTuple.push_back({ hashValue, static_cast<short>(i) });
		}

		// Sort the hash tuples based on shifted hash values
		std::sort(hashTuple.begin(), hashTuple.end(), [shift](const std::pair<int, short>& a, const std::pair<int, short>& b) {
			return (a.first >> shift) < (b.first >> shift);
			});

		// Populate bucket table with the first unique index for each bucket
		for (int i = 0; i < fileNames.size(); ++i) {
			int bucketIndex = hashTuple[i].first >> shift;
			if (bucketTable[bucketIndex] == -1) {
				bucketTable[bucketIndex] = static_cast<short>(i);
			}
		}

		// Create the result object with the hash data
		HashDataContainer hashData;
		hashData.Shift = shift;
		hashData.Offsets = bucketTable;
		hashData.Hashes.reserve(hashTuple.size());
		hashData.Indices.reserve(hashTuple.size());

		for (const auto& tuple : hashTuple) {
			hashData.Hashes.push_back(tuple.first);
			hashData.Indices.push_back(tuple.second);
		}

		hashData.StructSize = static_cast<int>(4 + 2 * bucketTable.size() + 4 * hashTuple.size() + 2 * hashTuple.size());

		BinaryWriter* writer = new BinaryWriter();
		writer->SetEndianess(fileIsBigEndian);
		writer->WriteString("DAT");
		writer->WriteByteZero();
		int fileCount = static_cast<int>(children.size());

		int positionsOffset = 0x20;
		int extensionsOffset = positionsOffset + 4 * fileCount;
		int namesOffset = extensionsOffset + 4 * fileCount;
		int sizesOffset = namesOffset + (fileCount * longestName) + 6;
		int hashMapOffset = sizesOffset + 4 * fileCount;

		writer->WriteUINT32(fileCount);
		writer->WriteUINT32(positionsOffset);
		writer->WriteUINT32(extensionsOffset);
		writer->WriteUINT32(namesOffset);
		writer->WriteUINT32(sizesOffset);
		writer->WriteUINT32(hashMapOffset);
		writer->WriteUINT32(0);

		for (FileNode* child : children) {
			(void)child; // TODO: If child isn't gonna be used in a future patch, remove it entirely instead of discarding.
			writer->WriteUINT32(0);
		}

		for (FileNode* child : children) {
			writer->WriteString(child->fileExtension);
			writer->WriteByteZero();
		}

		writer->WriteUINT32(longestName);
		for (FileNode* child : children) {
			writer->WriteString(child->fileName);
			for (int i = 0; i < longestName - child->fileName.length(); ++i) {
				writer->WriteByteZero();
			}
		}
		// Pad
		writer->WriteINT16(0);


		for (FileNode* child : children) {
			writer->WriteUINT32(static_cast<uint32_t>(child->fileData.size()));
		}

		// Prepare for hash writing
		writer->WriteUINT32(hashData.Shift);
		writer->WriteUINT32(16);
		writer->WriteUINT32(16 + static_cast<uint32_t>(hashData.Offsets.size()) * 2);
		writer->WriteUINT32(16 + static_cast<uint32_t>(hashData.Offsets.size()) * 2 + static_cast<uint32_t>(hashData.Hashes.size()) * 4);

		for (int i = 0; i < hashData.Offsets.size(); i++)
			writer->WriteINT16(hashData.Offsets[i]);

		for (int i = 0; i < fileCount; i++)
			writer->WriteINT32(hashData.Hashes[i]);

		for (int i = 0; i < fileCount; i++)
			writer->WriteINT16(hashData.Indices[i]);

		std::vector<int> offsets;
		for (FileNode* child : children) {
			(void)child; // TODO: If child isn't gonna be used in a future patch, remove it entirely instead of discarding.

			int targetPosition = HelperFunction::Align(static_cast<int>(writer->Tell()), 1024);
			int padding = targetPosition - static_cast<int>(writer->GetData().size());
			if (padding > 0) { // TODO: Replace this with an skip function
				std::vector<char> zeroPadding(padding, 0);
				writer->WriteBytes(zeroPadding);
			}


			offsets.push_back(static_cast<int>(writer->Tell()));
			writer->WriteBytes(child->fileData);

		}

		writer->Seek(positionsOffset);
		for (int i = 0; i < fileCount; i++) {
			writer->WriteUINT32(offsets[i]);
		}

		fileData = writer->GetData();

		auto finish = std::chrono::high_resolution_clock::now();
		auto millisecondsnoio = std::chrono::duration_cast<std::chrono::milliseconds>(finish - start_noio);

		CTDLog::Log::getInstance().LogNote(fileIcon + " Rebuilt in " + std::to_string(millisecondsnoio.count()) + "ms: " + fileName);
	}

	TrgFileNode::TrgFileNode(std::string fName) : FileNode(fName) {
		fileIcon = ICON_CI_CIRCUIT_BOARD;
		nodeType = TRG;
		fileFilter = L"Trigger Files (*.trg)\0*.trg;\0";
	}

	void TrgFileNode::LoadFile()
	{
		BinaryReader reader(fileData, false);
		reader.Seek(0x4);
		if (reader.ReadUINT32() == 17) {
			uint32_t triggerCount = reader.ReadUINT32();


		}


	}

	void TrgFileNode::SaveFile()
	{
	}

	EstFileNode::EstFileNode(std::string fName) : FileNode(fName) {
		fileIcon = ICON_CI_STAR;
		nodeType = EST;
		fileFilter = L"Effect Files (*.est)\0*.est;\0";
	}

	void EstFileNode::LoadFile()
	{
		BinaryReader reader(fileData, true);
		reader.SetEndianess(fileIsBigEndian);
		reader.Seek(0x4);
		uint32_t recordCount = reader.ReadUINT32();
		uint32_t recordOffsetTable = reader.ReadUINT32();
		uint32_t typeOffset = reader.ReadUINT32();
		uint32_t typeEndOffset = reader.ReadUINT32();
		uint32_t typeSize = reader.ReadUINT32();
		uint32_t typeNumber = reader.ReadUINT32();

		reader.Seek(recordOffsetTable);
		std::vector<uint32_t> offsets = reader.ReadUINT32Array(recordCount);

		reader.Seek(typeOffset);
		for (int x = 0; x < recordCount; x++) {
			EstRecord rec = EstRecord();
			for (int y = 0; y < typeNumber; y++) {
				
				reader.ReadUINT32();
				EstItem* item = nullptr;

				std::string type = reader.ReadString(4);
				if (fileIsBigEndian) std::reverse(type.begin(), type.end());
				

				uint32_t dataSize = reader.ReadUINT32();
				uint32_t dataOffset = reader.ReadUINT32();

				if (type == "MOVE") item = new EstMove();
				else if (type == "PART") item = new EstPart();
				else if (type == "EMIF") item = new EstEmif();
				else if (type == "TEX ") item = new EstTex();
				else if (type == "SZSA") item = new EstSzsa();
				else if (type == "PSSA") item = new EstPssa();
				else if (type == "RTSA") item = new EstRtsa();
				else if (type == "FVWK") item = new EstFvwk();
				else if (type == "FWK ") item = new EstFwk();
				else if (type == "EMMV") item = new EstEmmv();
				else if (type == "EMSA") item = new EstEmsa();
				else if (type == "EMFV") item = new EstEmfv();
				else if (type == "EMPA") item = new EstEmpa();
				else if (type == "EMRA") item = new EstEmra();
				else if (type == "EMVF") item = new EstEmvf();
				else if (type == "EMFW") item = new EstEmfw();
				else if (type == "MJSG") item = new EstMjsg();
				else if (type == "MJCM") item = new EstMjcm();
				else if (type == "MJNM") item = new EstMjnm();
				else if (type == "MJMM") item = new EstMjmm();
				else if (type == "MJDT") item = new EstMjdt();
				else if (type == "MJFN") item = new EstMjfn();
				else if (type == "MJVA") item = new EstMjva();
				else {
					CTDLog::Log::getInstance().LogError("Unknown EST Type! (" + type + ")");
					item = new EstItem();
					item->isValid = false;
				}

				if (dataSize == 0 || dataOffset == 0) {
					item->isValid = false;
				}
				else {
					uint32_t nextRecordPosition = reader.Tell();
					reader.Seek(offsets[x] + dataOffset);


					item->Read(reader);

					reader.Seek(nextRecordPosition);
				}
				rec.types.push_back(item);
			}

			records.push_back(rec);
		}

		return;
	}

	void EstFileNode::RenderGUI(CruelerContext* ctx)
	{
		if (records.size() == 0) {
			ImGui::Text("There are no records in this EST.");
		}

		int i = 0;
		for (EstRecord& rec : records) {
			ImGui::SeparatorText(("Record " + std::to_string(i)).c_str());
			for (EstItem* item : rec.types) {
				if (item->isValid) {
					
					ImGui::PushID(i);
					ImGui::Checkbox(("###" + std::to_string(item->type)).c_str(), &item->shouldRepack);
					ImGui::SameLine();
					if (ImGui::TreeNode((EstItemTypeToString.at(item->type)).c_str())) {
						item->Draw();

						ImGui::TreePop();

					}
					

					ImGui::PopID();
				}
				
			}

			i += 1;
		}
	}

	void EstFileNode::SaveFile()
	{
	}

	BayoEffNode::BayoEffNode(std::string fName) : FileNode(fName)
	{
		fileIcon = ICON_CI_FOLDER;
		TextColor = { 0.0f, 0.98f, 0.467f, 1.0f };
		nodeType = B1EFF;
		fileFilter = L"Platinum Effect Container(*.eff)\0*.eff;\0";
	}

	void BayoEffNode::LoadFile()
	{
		BinaryReader reader(fileData, true);
		reader.SetEndianess(fileIsBigEndian);
		reader.Seek(0x4);
		uint32_t dataNumber = reader.ReadUINT32();

		struct BayoEffDataInfo {
			int32_t index;
			uint32_t offset;
		};

		std::vector<BayoEffDataInfo> infos;

		for (uint32_t i = 0; i < dataNumber; i++) {
			BayoEffDataInfo datInfo;
			datInfo.index = reader.ReadINT32();
			datInfo.offset = reader.ReadUINT32();
			infos.push_back(datInfo);
		}

		for (BayoEffDataInfo info : infos) {
			reader.Seek(info.offset);
			std::string name = reader.ReadString(4) + ".est";
			uint32_t recordNumber = reader.ReadUINT32();
			FileNode* childNode = HelperFunction::LoadNode(name, reader.ReadBytes(0), fileIsBigEndian, fileIsBigEndian);
			childNode->parent = this;
			if (childNode) {
				children.push_back(childNode);
			}
		}


	}

	void BayoEffNode::SaveFile()
	{
	}

	BayoClpClhClwFileNode::BayoClpClhClwFileNode(std::string fName) : FileNode(fName)
	{
		fileIcon = ICON_CI_TYPE_HIERARCHY;
		TextColor = { 0, 1, 0.722, 1.0f };
		nodeType = B1PHYS;
		fileFilter = L"Platinum Physics (*.*)\0*.*;\0";
	}

	void BayoClpClhClwFileNode::LoadFile()
	{
		BinaryReader reader(fileData, true);
		reader.SetEndianess(fileIsBigEndian);
		if (type == B1_CLP) {
			fileFilter = L"Platinum CLP (*.clp)\0*.clp;\0";
			clpHeader = Bayo1ClpHeader();
			clpHeader.m_Num = reader.ReadUINT32();
			clpHeader.m_LimitSpringRate = reader.ReadFloat();
			clpHeader.m_SpdRate = reader.ReadFloat();
			clpHeader.m_Stretchy = reader.ReadFloat();
			clpHeader.m_BundleNum = reader.ReadUINT16();
			clpHeader.m_BundleNum2 = reader.ReadUINT16();
			clpHeader.m_Thick = reader.ReadFloat();
			clpHeader.m_gravityVec.x = reader.ReadFloat();
			clpHeader.m_gravityVec.y = reader.ReadFloat();
			clpHeader.m_gravityVec.z = reader.ReadFloat();
			clpHeader.m_GravityPartsNo = reader.ReadUINT32();
			clpHeader.m_FirstBundleRate = reader.ReadFloat();
			clpHeader.m_WindVec.x = reader.ReadFloat();
			clpHeader.m_WindVec.y = reader.ReadFloat();
			clpHeader.m_WindVec.z = reader.ReadFloat();
			clpHeader.m_WindPartsNo = reader.ReadUINT32();
			clpHeader.m_WindOffset.x = reader.ReadFloat();
			clpHeader.m_WindOffset.y = reader.ReadFloat();
			clpHeader.m_WindOffset.z = reader.ReadFloat();
			clpHeader.m_WindSin = reader.ReadFloat();
			clpHeader.m_HitAdjustRate = reader.ReadFloat();

			for (uint32_t i = 0; i < clpHeader.m_Num; i++) {
				Bayo1ClpUnit unit = Bayo1ClpUnit();
				unit.no = reader.ReadINT16();
				unit.noUp = reader.ReadINT16();
				unit.noDown = reader.ReadINT16();
				unit.noSide = reader.ReadINT16();
				unit.noPoly = reader.ReadINT16();
				unit.noFix = reader.ReadINT16();
				unit.rotLimit = reader.ReadFloat();
				unit.offset.x = reader.ReadFloat();
				unit.offset.y = reader.ReadFloat();
				unit.offset.z = reader.ReadFloat();
				clpHeader.works.push_back(unit);
			}

		}
	}

	void BayoClpClhClwFileNode::SaveFile()
	{
		if (type == B1_CLP) {
			BinaryWriter* writer = new BinaryWriter(fileIsBigEndian);

			writer->WriteUINT32(clpHeader.works.size());
			writer->WriteFloat(clpHeader.m_LimitSpringRate);
			writer->WriteFloat(clpHeader.m_SpdRate);
			writer->WriteFloat(clpHeader.m_Stretchy);
			writer->WriteUINT16(clpHeader.m_BundleNum);
			writer->WriteUINT16(clpHeader.m_BundleNum2);
			writer->WriteFloat(clpHeader.m_Thick);
			writer->WriteFloat(clpHeader.m_gravityVec.x);
			writer->WriteFloat(clpHeader.m_gravityVec.y);
			writer->WriteFloat(clpHeader.m_gravityVec.z);
			writer->WriteUINT32(clpHeader.m_GravityPartsNo);
			writer->WriteFloat(clpHeader.m_FirstBundleRate);
			writer->WriteFloat(clpHeader.m_WindVec.x);
			writer->WriteFloat(clpHeader.m_WindVec.y);
			writer->WriteFloat(clpHeader.m_WindVec.z);
			writer->WriteUINT32(clpHeader.m_WindPartsNo);
			writer->WriteFloat(clpHeader.m_WindOffset.x);
			writer->WriteFloat(clpHeader.m_WindOffset.y);
			writer->WriteFloat(clpHeader.m_WindOffset.z);
			writer->WriteFloat(clpHeader.m_WindSin);
			writer->WriteFloat(clpHeader.m_HitAdjustRate);

			for (Bayo1ClpUnit& unit : clpHeader.works) {
				writer->WriteUINT16(unit.no);
				writer->WriteUINT16(unit.noUp);
				writer->WriteUINT16(unit.noDown);
				writer->WriteUINT16(unit.noSide);
				writer->WriteUINT16(unit.noPoly);
				writer->WriteUINT16(unit.noFix);
				writer->WriteFloat(unit.rotLimit);
				writer->WriteFloat(unit.offset.x);
				writer->WriteFloat(unit.offset.y);
				writer->WriteFloat(unit.offset.z);
			}


			fileData = writer->GetData();
			delete writer;

		}



	}

	void BayoClpClhClwFileNode::RenderGUI(CruelerContext* ctx)
	{
		if (type == B1_CLP) {
			if (ImGui::BeginTabBar("clp")) {
				if (ImGui::BeginTabItem("Header")) {
					ImGui::InputFloat("Limit Spring Rate", &clpHeader.m_LimitSpringRate);
					ImGui::InputFloat("Speed Rate", &clpHeader.m_SpdRate);
					ImGui::InputFloat("Stretchy", &clpHeader.m_Stretchy);
					ImGui::InputInt("Bundle Number", &clpHeader.m_BundleNum);
					ImGui::InputInt("Bundle Number 2", &clpHeader.m_BundleNum2);
					ImGui::InputFloat("Thick", &clpHeader.m_Thick);
					ImGui::InputFloat3("Gravity Vector", clpHeader.m_gravityVec);
					ImGui::InputInt("Gravity Parts Number", &clpHeader.m_GravityPartsNo);
					ImGui::InputFloat("First Bundle Rate", &clpHeader.m_FirstBundleRate);
					ImGui::InputFloat3("Wind Vector", clpHeader.m_WindVec);
					ImGui::InputInt("Wind Parts Number", &clpHeader.m_WindPartsNo);
					ImGui::InputFloat3("Wind Offset", clpHeader.m_WindOffset);
					ImGui::InputFloat("Wind Sine", &clpHeader.m_WindSin);
					ImGui::InputFloat("Hit Adjust Rate", &clpHeader.m_HitAdjustRate);

					ImGui::EndTabItem();
				}
				if (ImGui::BeginTabItem("Works")) {
					if (ImGui::Button("Export CSV")) {
						OPENFILENAME ofn;
						wchar_t szFile[260] = { 0 };
						LPWSTR pFile = szFile;
						std::string csvName = fileName + ".csv";
						mbstowcs_s(0, pFile, csvName.length() + 1, csvName.c_str(), _TRUNCATE);

						ZeroMemory(&ofn, sizeof(OPENFILENAME));

						ofn.lpstrFile = pFile;
						ofn.lStructSize = sizeof(OPENFILENAME);
						ofn.hwndOwner = NULL;
						ofn.nMaxFile = 260;
						ofn.lpstrFilter = L"CSV Table (*.csv)\0*.csv;\0";

						ofn.nFilterIndex = 1;
						ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;

						if (GetSaveFileName(&ofn) == TRUE) {
							std::ofstream outputFile(ofn.lpstrFile); // Open in binary mode!
							if (outputFile.is_open()) {
								outputFile << "no,noUp,noDown,noSide,noPoly,noFix,rotLimit,offset\n";
								for (Bayo1ClpUnit& unit : clpHeader.works) {
									outputFile << std::to_string(unit.no) + ",";
									outputFile << std::to_string(unit.noUp) + ",";
									outputFile << std::to_string(unit.noDown) + ",";
									outputFile << std::to_string(unit.noSide) + ",";
									outputFile << std::to_string(unit.noPoly) + ",";
									outputFile << std::to_string(unit.noFix) + ",";
									outputFile << std::to_string(unit.rotLimit) + ",";
									outputFile << std::to_string(unit.offset.x) + " " + std::to_string(unit.offset.y) + " " + std::to_string(unit.offset.z) + "\n";
								}
								
							}

						}
					}
					ImGui::SameLine();
					if (ImGui::Button("Import CSV")) {
						OPENFILENAME ofn;
						wchar_t szFile[260] = { 0 };
						LPWSTR pFile = szFile;

						mbstowcs_s(0, pFile, fileName.length() + 1, fileName.c_str(), _TRUNCATE);

						ZeroMemory(&ofn, sizeof(OPENFILENAME));

						ofn.lpstrFile = pFile;
						ofn.lStructSize = sizeof(OPENFILENAME);
						ofn.hwndOwner = NULL;
						ofn.nMaxFile = 260;
						ofn.lpstrFilter = L"CSV Table (*.csv)\0*.csv;\0";

						ofn.nFilterIndex = 1;
						ofn.Flags = OFN_PATHMUSTEXIST;

						if (GetOpenFileName(&ofn) == TRUE) {
							std::ifstream file(ofn.lpstrFile);
							if (file.is_open()) {
								std::vector<Bayo1ClpUnit> units;
								std::string line;
								std::getline(file, line); // HDR Skip

								while (std::getline(file, line)) {
									std::stringstream ss(line);
									std::string token;

									Bayo1ClpUnit unit;

									std::getline(ss, token, ','); unit.no = std::stoi(token);
									std::getline(ss, token, ','); unit.noUp = std::stoi(token);
									std::getline(ss, token, ','); unit.noDown = std::stoi(token);
									std::getline(ss, token, ','); unit.noSide = std::stoi(token);
									std::getline(ss, token, ','); unit.noPoly = std::stoi(token);
									std::getline(ss, token, ','); unit.noFix = std::stoi(token);
									std::getline(ss, token, ','); unit.rotLimit = std::stof(token);

									std::getline(ss, token, ',');
									std::stringstream offsetStream(token);
									offsetStream >> unit.offset.x >> unit.offset.y >> unit.offset.z;

									units.push_back(unit);
								}

								clpHeader.works = units;

							}
						}
					}


					ImGui::BeginTable("clp_works", 8);
					ImGui::TableSetupColumn("no");
					ImGui::TableSetupColumn("noUp");
					ImGui::TableSetupColumn("noDown");
					ImGui::TableSetupColumn("noSide");
					ImGui::TableSetupColumn("noPoly");
					ImGui::TableSetupColumn("noFix");
					ImGui::TableSetupColumn("rotLimit");
					ImGui::TableSetupColumn("offset");
					ImGui::TableHeadersRow();

					for (int i = 0; i < clpHeader.works.size(); i++) {
						ImGui::TableNextRow();


						//ImGui::SeparatorText(("Work " + std::to_string(i)).c_str());
						ImGui::PushID(i);
						ImGui::TableSetColumnIndex(0);
						ImGui::InputInt("###No", &clpHeader.works[i].no, 0);
						ImGui::TableSetColumnIndex(1);
						ImGui::InputInt("###NoUp", &clpHeader.works[i].noUp, 0);
						ImGui::TableSetColumnIndex(2);
						ImGui::InputInt("###NoDown", &clpHeader.works[i].noDown, 0);
						ImGui::TableSetColumnIndex(3);
						ImGui::InputInt("###NoSide", &clpHeader.works[i].noSide, 0);
						ImGui::TableSetColumnIndex(4);
						ImGui::InputInt("###NoPoly", &clpHeader.works[i].noPoly, 0);
						ImGui::TableSetColumnIndex(5);
						ImGui::InputInt("###NoFix", &clpHeader.works[i].noFix, 0);
						ImGui::TableSetColumnIndex(6);
						ImGui::InputFloat("###Rotation Limit", &clpHeader.works[i].rotLimit);
						ImGui::TableSetColumnIndex(7);
						ImGui::InputFloat3("###Offset", clpHeader.works[i].offset);
						ImGui::PopID();

					}

					ImGui::EndTable();

					if (ImGui::Button("+")) {
						clpHeader.works.push_back(Bayo1ClpUnit());
					}

					ImGui::EndTabItem();
				}
				ImGui::EndTabBar();
			}

		}

	}

	Ct2FileNode::Ct2FileNode(std::string fName) : FileNode(fName)
	{
		fileIcon = ICON_CI_PACKAGE;
		TextColor = { 0.529f, 0, 0.724f, 1.0f };
		nodeType = CT2;
		fileFilter = L"Cubemap Texture Package (*.ctx)\0*.ctx;\0";
	}

	void Ct2FileNode::LoadFile()
	{
		BinaryReader reader(fileData, true);
		reader.SetEndianess(fileIsBigEndian);

		reader.Seek(0x4);
		uint32_t texture_count = reader.ReadUINT32();
		std::vector<uint32_t> offsets;
		for (uint32_t i = 0; i < texture_count; i++) {
			offsets.push_back(reader.ReadUINT32());
		}
		
		int i = 0;
		for (uint32_t offset : offsets) {

			
			reader.Seek(offset);
			WtbFileNode* wtbNode = new WtbFileNode(std::to_string(i) + ".wtb");
			wtbNode->SetFileData(reader.ReadBytes(512000));
			wtbNode->LoadFile();
			children.push_back(wtbNode);
			i += 1;
		}


	}

	void Ct2FileNode::SaveFile()
	{
	}
