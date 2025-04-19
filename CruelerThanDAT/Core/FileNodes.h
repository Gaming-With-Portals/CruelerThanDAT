#pragma once
#ifndef FILENODE_H
#define FILENODE_H
#define NOMINMAX
#include <string>
#include <vector>
#include "../globals.h"
#include "../imgui.h"
#include "BinaryHandler.h"
#include "../Assets/CodIcons.h"
#include <windows.h>
#include <fstream> 
#include "Log.h"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <functional>
#include "Utility/CRC32.h"
#include "Utility/HashDataContainer.h"
#include <sstream>
#include <format>
#include "Utility/ImGuiExtended.h"
#include "Utility/WMB.h"
#include <d3dx9.h>
/**/





class FileNode;  // Forward declaration

extern std::vector<FileNode*> openFiles;


namespace BXMInternal {
	struct XMLAttribute {
		std::string value = "";
		std::string name = "";
	};

	struct XMLNode {
		std::string name = "";
		std::string value = "";
		std::vector<XMLNode*> childNodes;
		std::vector<XMLAttribute*> childAttributes;
		
	};

}
void closeNode(FileNode* target);

namespace HelperFunction {
	FileNode* LoadNode(std::string fileName, const std::vector<char>& data, bool forceEndianess = false, bool bigEndian = false);

	int Align(int value, int alignment);
}

enum FileNodeTypes {
	DEFAULT,
	UNKNOWN,
	MOT,
	BXM,
	WEM,
	WMB,
	BNK,
	DAT,
	WTB,
	LY2
};

int IntLength(int);


class FileNode {
public:
	std::string fileName;
	std::string fileExtension;
	std::vector<FileNode*> children;
	std::vector<char> fileData;
	FileNodeTypes nodeType;
	const wchar_t* fileFilter = L"All Files(*.*)\0*.*;\0";
	bool loadFailed = false;
	bool isEdited = false;

	static FileNode* selectedNode;


	std::string fileIcon = ICON_CI_QUESTION;
	bool fileIsBigEndian;
	ImVec4 TextColor = { 1.0f, 1.0f, 1.0f, 1.0f };

	FileNode(std::string fName) {
		fileName = fName;
		fileExtension = fileName.substr(fileName.find_last_of(".") + 1);
	}

	virtual ~FileNode() {

	}

	virtual void PopupOptionsEx() {

	}

	virtual void ExportFile() {
		OPENFILENAME ofn;
		wchar_t szFile[260] = { 0 };

		mbstowcs_s(0, szFile, fileName.length() + 1, fileName.c_str(), _TRUNCATE);

		ZeroMemory(&ofn, sizeof(OPENFILENAME));
		ofn.lpstrFile = szFile;
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

	virtual void ReplaceFile() {
		OPENFILENAME ofn;
		wchar_t szFile[260] = { 0 };

		mbstowcs_s(0, szFile, fileName.length() + 1, fileName.c_str(), _TRUNCATE);

		ZeroMemory(&ofn, sizeof(OPENFILENAME));
		ofn.lpstrFile = szFile;
		ofn.lStructSize = sizeof(OPENFILENAME);
		ofn.hwndOwner = NULL;
		ofn.nMaxFile = 260;
		ofn.lpstrFilter = fileFilter;
		ofn.nFilterIndex = 1;
		ofn.Flags = OFN_PATHMUSTEXIST;

		if (GetOpenFileName(&ofn) == TRUE) {
			std::ifstream file(ofn.lpstrFile, std::ios::binary | std::ios::ate); // Open in binary mode!
			if (file.is_open()) {
				fileData.clear();
				std::streamsize size = file.tellg(); // Get file size
				fileData.resize(size);         // Resize vector to file size
				file.seekg(0, std::ios::beg);       // Go back to beginning
				file.read(fileData.data(), size); // Read data
				file.close();
				std::cout << "Replaced file! " << std::endl;
			} else {
				std::cout << "Error" << std::endl;
			}

		}

		ImGui::CloseCurrentPopup();

	}

	virtual void PopupOptions() {
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
				}
			}



			PopupOptionsEx();

			ImGui::EndPopup();
		}
	}

	
	virtual void Render() {
		std::string displayName = fileIcon + fileName;
		


		if (fileIsBigEndian) {
			displayName += " (X360/PS3)";
		}
		if (children.size() == 0) {

			ImGui::PushStyleColor(ImGuiCol_Text, TextColor);
			if (ImGui::TreeNodeEx(displayName.c_str(), ImGuiTreeNodeFlags_Leaf)) {
				PopupOptions();

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

				PopupOptions();

				if (ImGui::IsItemClicked()) {
					selectedNode = this; // Update selected node
				}
				ImGui::PopStyleColor();
				for (FileNode* node : children) {
					node->Render();
				}

				ImGui::TreePop();
			}
			else {
				ImGui::PopStyleColor();
			}
		}
		
	}

	virtual void LoadFile() = 0;
	virtual void SaveFile() = 0;

	void SetFileData(const std::vector<char>& data) {
		fileData = data;
	}

	const std::vector<char>& GetFileData() const {
		return fileData;
	}
};



class UnkFileNode : public FileNode {
public:
	UnkFileNode(std::string fName) : FileNode(fName) {}
	void LoadFile() override {

	}
	void SaveFile() override {

	}
};

struct LY2ExData {
	unsigned short a;
	unsigned short b;
	unsigned int c;
	unsigned int d;
};

struct LY2Instance {
	float pos[3] = { 0, 0, 0 };
	float scale[3] = { 1, 1, 1 };
	float rot[3] = { 0, 0, 0 };
	unsigned int unknownB4C = 0;
	unsigned int unknownB4E = 0;
	unsigned int unknownB4F = 0;
	int unknownB4G = 0;
};


struct LY2Node {
	int flag_a;
	int flag_b;
	std::string prop_category;
	unsigned short prop_id;
	int offset;
	int count;
	std::vector<LY2Instance> instances;
};



class LY2FileNode : public FileNode {
public:

	int ly2Flags;
	int propTypeCount;
	int mysteryPointer;
	int mysteryCount;
	std::vector<LY2Node> nodes;
	std::vector<LY2ExData> extradata;

	char input_id[7];
	

	LY2FileNode(std::string fName) : FileNode(fName) {
		fileIcon = ICON_CI_ARRAY;
		TextColor = { 1.0f, 0.0f, 0.376f, 1.0f };
		nodeType = LY2;
		fileFilter = L"Layout Files (*.ly2)\0*.ly2;\0";
		ly2Flags = 0;
		propTypeCount = 0;
		mysteryPointer = 0;
		mysteryCount = 0;
	}
	void LoadFile() override {
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

			int pos = reader.Tell();
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
	void SaveFile() override {
		if (!isEdited) {
			return;
		}
		BinaryWriter* writer = new BinaryWriter();

		writer->WriteString("LY2");
		writer->WriteByteZero();
		writer->WriteINT32(4);
		writer->WriteUINT32(nodes.size());

		int extradataoffset = 20;
		int mainchunkend = 20;
		for (LY2Node& node : nodes) {
			extradataoffset += (20 + (40 * node.instances.size()));
			mainchunkend += 20;

		}
		writer->WriteUINT32(extradataoffset);
		writer->WriteUINT32(extradata.size());
		int offset = 0;
		for (LY2Node& node : nodes) {
			writer->WriteUINT32(node.flag_a);
			writer->WriteUINT32(node.flag_b);
			writer->WriteString(node.prop_category);
			writer->WriteUINT16(node.prop_id);
			writer->WriteUINT32(mainchunkend + offset);
			writer->WriteUINT32(node.instances.size());
			offset += 40 * node.instances.size();
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

	void RenderGUI() {
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
					node.prop_id = std::stoi(hexStr, nullptr, 16);
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
						ImGui::Text("Item Count %d", node.instances.size());
						ImGui::InputInt("Flag A", &node.flag_a);
						ImGui::InputInt("Flag B", &node.flag_b);
					}
					ImGui::PopID();
					i += 1;
				}
				
				if (ImGui::Button("Optimize Groups")) {
					// Basically, every group with equal IDs and flags can be combined into one.
					for (size_t i = 0; i < nodes.size(); ++i) {
						if (nodes[i].instances.empty()) continue;

						for (size_t j = i + 1; j < nodes.size(); ++j) {
							if (nodes[j].flag_a == nodes[i].flag_a &&
								nodes[j].flag_b == nodes[i].flag_b &&
								nodes[j].prop_id == nodes[i].prop_id &&
								nodes[j].prop_category == nodes[i].prop_category) {

								nodes[i].instances.insert(nodes[i].instances.end(), nodes[j].instances.begin(), nodes[j].instances.end());

								nodes[j].instances.clear(); 
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



};




class MotFileNode : public FileNode {
public:

	MotFileNode(std::string fName) : FileNode(fName) {
		fileIcon = ICON_CI_RECORD;
		TextColor = { 1.0f, 0.0f, 0.376f, 1.0f };
		nodeType = MOT;
		fileFilter = L"XSI Motion Files(*.mot)\0*.mot;\0";
	}
	void LoadFile() override {



	}
	void SaveFile() override {

	}
};

class BxmFileNode : public FileNode {
public:
	std::string ConvertToXML(BXMInternal::XMLNode* node, int indentLevel = 0) {
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


	BXMInternal::XMLNode* baseNode;
	int infoOffset;
	int dataOffset;
	int stringOffset;

	std::string xmlData = "";

	std::vector<char> xmlBuffer;

	BxmFileNode(std::string fName) : FileNode(fName) {
		fileIcon = ICON_CI_CODE;
		TextColor = { 1.0f, 0.0f, 0.914f, 1.0f };
		fileIsBigEndian = false;
		nodeType = BXM;		
		fileFilter = L"Binary XML Files(*.bxm)\0*.bxm;\0";
	}

	void RenderXMLTree(BXMInternal::XMLNode* node) {

		/*if (ImGui::TreeNode((node->name + " (XMLNode)").c_str())) {
			if (ImGui::TreeNode("Attributes")) {
				for (BXMInternal::XMLAttribute* attrib : node->childAttributes) {

					std::string text = attrib->value;  // Copy the value so it's modifiable
					if (ImGui::InputText(attrib->name.c_str(), &text[0], text.size() + 1)) {
						attrib->value = text;
					}


				}
				ImGui::TreePop();
			}
			if (node->value != "") {
				std::string text = node->value;  // Copy the value so it's modifiable
				if (ImGui::InputText("", &text[0], text.size() + 1)) {
					node->value = text;
				}
			}

			if (ImGui::TreeNode("Children")) {
				for (BXMInternal::XMLNode* node2 : node->childNodes) {
					RenderXMLTree(node2);
				}
				ImGui::TreePop();
			}
			ImGui::TreePop();


		}*/

		std::string currentLine;
		std::stringstream ss(xmlData);
		while (std::getline(ss, currentLine)) {
			for (size_t i = 0; i < currentLine.size(); ++i) {
				if (currentLine[i] == '<') {
					ImGui::TextColored(ImColor(255, 0, 0), "<");
					ImGui::SameLine();
				}
				else if (currentLine[i] == '>') {
					ImGui::TextColored(ImColor(255, 0, 0), ">");
					ImGui::SameLine();
				}
				else if (currentLine[i] == '\"') {
					ImGui::TextColored(ImColor(0, 255, 0), "\"");
					ImGui::SameLine();
				}
				else if (currentLine[i] == '\n') {

				}
				else {
					ImGui::TextColored(ImColor(255, 255, 255), std::string(1, currentLine[i]).c_str());
					ImGui::SameLine();
				}
			}
		}


		/*if (ImGui::InputTextMultiline("XML Editor", xmlBuffer.data(), xmlBuffer.size())) {
			xmlData = std::string(xmlBuffer.data());

			// If needed, expand buffer to accommodate more text
			if (xmlData.length() + 1 > xmlBuffer.size()) {
				xmlBuffer.resize(xmlData.length() + 512);
			}
		}*/

	}


	BXMInternal::XMLNode* ReadXMLNode(BinaryReader reader) {

		BXMInternal::XMLNode* node = new BXMInternal::XMLNode();

		int childCount = reader.ReadUINT16();
		int firstChildIndex = reader.ReadUINT16();
		int attributeNumber = reader.ReadUINT16();
		int dataIndex = reader.ReadUINT16();

		int currentPos = reader.Tell();

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
			node->childNodes.push_back(ReadXMLNode(reader));

		}

		return node;
	}

	void LoadFile() override {
		BinaryReader reader(fileData, true);
		reader.Seek(0x8);
		int nodeCount = reader.ReadUINT16();
		int dataCount = reader.ReadUINT16();
		int dataSize = reader.ReadUINT32();

		infoOffset = 16;
		dataOffset = 16 + 8 * nodeCount;
		stringOffset = 16 + 8 * nodeCount + dataCount * 4;


		baseNode = ReadXMLNode(reader);
		xmlData = ConvertToXML(baseNode);
		xmlBuffer.resize(xmlData.size() + 1); // +1 for null termination
		std::copy(xmlData.begin(), xmlData.end(), xmlBuffer.begin()); // Copy new data into buffer
		xmlBuffer[xmlBuffer.size() - 1] = '\0';


		return;
	}
	void SaveFile() override {

	}
};

class WtbFileNode : public FileNode {
public:
	int textureCount = 0;
	std::vector<int> textureOffsets;
	std::vector<int> textureSizes;
	std::vector<int> textureFlags;
	std::vector<int> textureIdx;
	std::vector<std::vector<char>> textureData;

	WtbFileNode(std::string fName) : FileNode(fName) {
		fileIcon = ICON_CI_FILE_MEDIA;
		nodeType = WTB;
		TextColor = { 1.0f, 0.0f, 0.7f, 1.0f };
		fileFilter = L"Texture Container(*.wtb)\0*.wtb;\0";
	}
	void LoadFile() override {
		BinaryReader reader(fileData, false);
		reader.SetEndianess(fileIsBigEndian);

		reader.Seek(0x4);
		if (reader.ReadUINT32() == 1) {
			textureCount = reader.ReadUINT32();
			int offsetTextureOffsets = reader.ReadUINT32();
			int offsetTextureSizes = reader.ReadUINT32();
			int offsetTextureFlags = reader.ReadUINT32();
			int offsetTextureIdx = reader.ReadUINT32();
			int offsetTextureInfo = reader.ReadUINT32();




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


			for (int x = 0; x < textureCount; x++) {
				reader.Seek(textureOffsets[x]);
				textureData.push_back(reader.ReadBytes(textureSizes[x]));

			}




		}


	}
	void SaveFile() override {

	}

};

class WemFileNode : public FileNode {
public:
	WemFileNode(std::string fName) : FileNode(fName) {
		fileIcon = ICON_CI_MUSIC;
		nodeType = WEM;
		fileFilter = L"WWISE Audio Container(*.wem)\0*.wem;\0";
	}
	void LoadFile() override {

	}
	void SaveFile() override {

	}

};

class CruelerMesh {
public:

	std::string name = "N/A";
	bool visibility = true;
	int vtxfmt = 0;
	LPDIRECT3DVERTEXBUFFER9 vertexBuffer;
	int vertexCount;
	LPDIRECT3DINDEXBUFFER9 indexBuffer;
	int indexCount;
	int structSize;


};

class WmbFileNode : public FileNode {
public:
	std::vector<CruelerMesh*> displayMeshes;

	float rotationAngle = 0;;

	WmbFileNode(std::string fName) : FileNode(fName) {
		fileIcon = ICON_CI_SYMBOL_METHOD;
		TextColor = { 1.0f, 0.671f, 0.0f, 1.0f };
		nodeType = WMB;
		fileFilter = L"Platinum Model File(*.wmb)\0*.wmb;\0";
	}
	void LoadFile() override {
		BinaryReader reader(fileData, true);
		reader.SetEndianess(fileIsBigEndian);

		WMBHeader header = WMBHeader();
		header = reader.ReadStruct<WMBHeader>();

		reader.Seek(header.offsetVertexGroups);
		std::vector<WMBVertexGroup> vertexGroups = reader.ReadStructs<WMBVertexGroup>(header.numVertexGroups);
		reader.Seek(header.offsetBatches);
		std::vector<WMBBatch> batches = reader.ReadStructs<WMBBatch>(header.numBatches);
		reader.Seek(header.offsetMeshes);
		std::vector<WMBMesh> meshes = reader.ReadStructs<WMBMesh>(header.numMeshes);

		for (WMBMesh& mesh : meshes) {
			CruelerMesh* ctdmesh = new CruelerMesh();
			ctdmesh->vtxfmt = header.vertexFormat;
			reader.Seek(mesh.offsetName);
			ctdmesh->name = reader.ReadNullTerminatedString();
			reader.Seek(mesh.offsetBatches); // this format sucks :fire:
			int meshBatchID = reader.ReadUINT16();
			WMBBatch& activeBatch = batches[meshBatchID];
			WMBVertexGroup& activeVtxGroup = vertexGroups[activeBatch.vertexGroupIndex];

			ctdmesh->vertexCount = activeVtxGroup.numVertexes;
			ctdmesh->indexCount = activeVtxGroup.numIndexes;

			reader.Seek(activeVtxGroup.offsetIndexes + (sizeof(short) * activeBatch.indexStart));
			std::vector<unsigned short> indices;
			for (int x = 0; x < activeBatch.numIndices; x++) {
				indices.push_back(reader.ReadUINT16());
			}

			g_pd3dDevice->CreateIndexBuffer(activeBatch.numIndices * sizeof(short), 0,
				D3DFMT_INDEX16, D3DPOOL_MANAGED,
				&ctdmesh->indexBuffer, nullptr);
			void* pIndexData = nullptr;
			ctdmesh->indexBuffer->Lock(0, 0, &pIndexData, 0);

			std::memcpy(pIndexData, indices.data(), indices.size() * sizeof(short));
			ctdmesh->indexBuffer->Unlock();

			

			// vertex format lore
			if (header.vertexFormat == 263){
				reader.Seek(activeVtxGroup.offsetVertexes + activeBatch.vertexStart * sizeof(WMBVertexA));
				D3DVERTEXELEMENT9 WMBVertexADecl[] = {
					{ 0, offsetof(WMBVertexA, position), D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
					{ 0, offsetof(WMBVertexA, u), D3DDECLTYPE_SHORT4N, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
					{ 0, offsetof(WMBVertexA, normals), D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0 },
					{ 0, offsetof(WMBVertexA, tangents), D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TANGENT, 0 },
					D3DDECL_END() // This marks the end of the vertex declaration
				};
				ctdmesh->structSize = sizeof(WMBVertexA);

				IDirect3DVertexDeclaration9* vertexDecl;
				g_pd3dDevice->CreateVertexDeclaration(WMBVertexADecl, &vertexDecl);
				g_pd3dDevice->SetVertexDeclaration(vertexDecl);

				std::vector<WMBVertexA> vertexes = reader.ReadStructs<WMBVertexA>(activeBatch.numVertices);

				g_pd3dDevice->CreateVertexBuffer(activeBatch.numVertices * sizeof(WMBVertexA),
					0, 0,
					D3DPOOL_DEFAULT, &ctdmesh->vertexBuffer, NULL);

				void* pVertexData = nullptr;
				HRESULT hr = ctdmesh->vertexBuffer->Lock(0, 0, &pVertexData, 0);
				std::memcpy(pVertexData, vertexes.data(), activeBatch.numVertices * sizeof(WMBVertexA));
				ctdmesh->vertexBuffer->Unlock();


			}
			else if (header.vertexFormat == 65847 || header.vertexFormat == 66359) {

				reader.Seek(activeVtxGroup.offsetVertexes + activeBatch.vertexStart * sizeof(WMBVertex65847));
				D3DVERTEXELEMENT9 WMBVertexADecl[] = {
					{ 0, offsetof(WMBVertex65847, position),   D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
					{ 0, offsetof(WMBVertex65847, u),          D3DDECLTYPE_SHORT4N, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
					{ 0, offsetof(WMBVertex65847, normals),    D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0 },
					{ 0, offsetof(WMBVertex65847, tangents),   D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TANGENT, 0 },
					{ 0, offsetof(WMBVertex65847, boneIndexes), D3DDECLTYPE_FLOAT1, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_BLENDINDICES, 0 },
					{ 0, offsetof(WMBVertex65847, boneWeights), D3DDECLTYPE_FLOAT1, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_BLENDWEIGHT, 0 },
					D3DDECL_END()
				};
				ctdmesh->structSize = sizeof(WMBVertex65847);
				IDirect3DVertexDeclaration9* vertexDecl;
				g_pd3dDevice->CreateVertexDeclaration(WMBVertexADecl, &vertexDecl);
				g_pd3dDevice->SetVertexDeclaration(vertexDecl);

				std::vector<WMBVertex65847> vertexes = reader.ReadStructs<WMBVertex65847>(activeBatch.numVertices);

				g_pd3dDevice->CreateVertexBuffer(activeBatch.numVertices * sizeof(WMBVertex65847),
					0, 0,
					D3DPOOL_DEFAULT, &ctdmesh->vertexBuffer, NULL);

				void* pVertexData = nullptr;
				HRESULT hr = ctdmesh->vertexBuffer->Lock(0, 0, &pVertexData, 0);
				std::memcpy(pVertexData, vertexes.data(), activeBatch.numVertices * sizeof(WMBVertex65847));
				ctdmesh->vertexBuffer->Unlock();
			}



			displayMeshes.push_back(ctdmesh);
		}

	}
	void SaveFile() override {

	}


	void RenderMesh() {
		rotationAngle += 0.01f;
		for (CruelerMesh* mesh : displayMeshes) {
			if (mesh->visibility) {
				g_pd3dDevice->SetIndices(mesh->indexBuffer);
				g_pd3dDevice->SetStreamSource(0, mesh->vertexBuffer, 0, mesh->structSize);

				D3DXMATRIX matWorld, matView, matProj, matScale, matRotate, matWVP;


				D3DXMatrixIdentity(&matView);
				D3DXMatrixIdentity(&matProj);

				// Rotate the mesh
				D3DXMatrixRotationY(&matRotate, rotationAngle);
				float scaleFactor = 0.4f;

				D3DXMatrixScaling(&matScale, scaleFactor, scaleFactor, scaleFactor);
				matWorld =  matScale * matRotate;
				// Final WVP matrix
				matWVP = matWorld * matView * matProj;


				// Send it to the shader

				g_pd3dDevice->SetVertexShader(pSolidShaderVTX);
				g_pd3dDevice->SetPixelShader(pSolidShaderPX);

				g_pd3dDevice->SetVertexShaderConstantF(0, (float*)&matWVP, 4);

				g_pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, mesh->vertexCount, 0, mesh->indexCount / 3);
			}



		}

	}


	void RenderGUI() {
		ImGui::Text("WMB Meshes");
		int i = 0;
		for (CruelerMesh* mesh : displayMeshes) {
			i += 1;
			ImGui::Text((std::to_string(i) + ". ").c_str());
			ImGui::SameLine();
			ImGui::Checkbox((mesh->name).c_str(), &mesh->visibility);

			

		}
	}

};

class BnkFileNode : public FileNode {
public:
	BnkFileNode(std::string fName) : FileNode(fName) {
		fileIcon = ICON_CI_PACKAGE;
		TextColor = { 0.0f, 0.529f, 0.724f, 1.0f };
		nodeType = BNK;
		fileFilter = L"WWISE Audio Bank v72(*.bnk)\0*.bnk;\0";
	}
	void LoadFile() override {
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
		
		std::string tmpHeader = "";
		int tmpLength = headerLength;
		reader.Seek(0x8);
		while (tmpHeader != "DIDX") {
			int offset = reader.Tell();
			reader.Seek(offset + headerLength);
			tmpHeader = reader.ReadString(4);
			tmpLength = reader.ReadUINT32();
		}

		std::vector<int> wemIDs;
		std::vector<int> wemOffsets;
		std::vector<int> wemSizes;


		// Checking the final header is actually DIDX
		if (tmpHeader == "DIDX") {
			if (tmpLength > 0) {
				int wemCount = (tmpLength / 12);
				for (int i = 0; i < wemCount; i++) {
					wemIDs.push_back(reader.ReadUINT32());
					wemOffsets.push_back(reader.ReadUINT32());
					wemSizes.push_back(reader.ReadUINT32());
				}
				reader.ReadUINT32();
				reader.ReadUINT32();
				int baseOffset = reader.Tell();


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


	}
	void SaveFile() override {
		
	}
};



class DatFileNode : public FileNode {
public:
	DatFileNode(std::string fName) : FileNode(fName) {
		fileIcon = ICON_CI_FOLDER;
		TextColor = { 0.0f, 0.98f, 0.467f, 1.0f };
		nodeType = DAT;
		fileFilter = L"Platinum File Container(*.dat, *.dtt, *.eff, *.evn, *.eft)\0*.dat;*.dtt;*.eff;*.evn;*.eft;\0";
	}

	void LoadFile() override {
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
		unsigned int ExtensionsOffset = reader.ReadUINT32();
		unsigned int NamesOffset = reader.ReadUINT32();
		unsigned int SizesOffset = reader.ReadUINT32();
		unsigned int HashMapOffset = reader.ReadUINT32();
		
		reader.Seek(PositionsOffset);
		std::vector<int> offsets;
		for (int f = 0; f < FileCount; f++) {
			offsets.push_back(reader.ReadUINT32());
		}
		
		reader.Seek(NamesOffset);
		int nameLength = reader.ReadUINT32();
		std::vector<std::string> names;
		for (int f = 0; f < FileCount; f++) {
			std::string temp_name = reader.ReadString(nameLength);
			temp_name.erase(std::remove(temp_name.begin(), temp_name.end(), '\0'), temp_name.end());
			names.push_back(temp_name);
		}
		
		reader.Seek(SizesOffset);
		std::vector<int> sizes;
		for (int f = 0; f < FileCount; f++) {
			sizes.push_back(reader.ReadUINT32());
		}

		for (int f = 0; f < FileCount; f++) {
			reader.Seek(offsets[f]);
			FileNode* childNode = HelperFunction::LoadNode(names[f], reader.ReadBytes(sizes[f]), fileIsBigEndian, fileIsBigEndian);
			if (childNode) {
				children.push_back(childNode);
			}

		}


	}



	void SaveFile() override {
		std::cout << "Saving DAT file: " << fileName << std::endl;
		
		CTDLog::Log::getInstance().LogNote(fileIcon + " Recompiling " + fileName);

		int longestName = 0;

		for (FileNode* child : children) {
			child->SaveFile();
			if (child->fileName.length() > longestName) {
				longestName = child->fileName.length() + 1;
			}
		}

		CRC32 crc32;

		std::vector<std::string> fileNames;
		for (FileNode* node : children) {
			fileNames.push_back(node->fileName);
		}
		
		int shift = std::min(31, 32 - IntLength(fileNames.size()));
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

		hashData.StructSize = 4 + 2 * bucketTable.size() + 4 * hashTuple.size() + 2 * hashTuple.size();

		BinaryWriter* writer = new BinaryWriter();
		writer->WriteString("DAT");
		writer->WriteByteZero();
		int fileCount = children.size();
		
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
			writer->WriteUINT32(child->fileData.size());
		}

		// Prepare for hash writing
		writer->WriteUINT32(hashData.Shift);
		writer->WriteUINT32(16);
		writer->WriteUINT32(16 + hashData.Offsets.size() * 2);
		writer->WriteUINT32(16 + hashData.Offsets.size() * 2 + hashData.Hashes.size() * 4);

		for (int i = 0; i < hashData.Offsets.size(); i++)
			writer->WriteINT16(hashData.Offsets[i]);

		for (int i = 0; i < fileCount; i++)
			writer->WriteINT32(hashData.Hashes[i]);

		for (int i = 0; i < fileCount; i++)
			writer->WriteINT16(hashData.Indices[i]);
		
		std::vector<int> offsets;
		for (FileNode* child : children) {

			int targetPosition = HelperFunction::Align(writer->Tell(), 1024);
			int padding = targetPosition - writer->GetData().size();
			if (padding > 0) {
				std::vector<char> zeroPadding(padding, 0);
				writer->WriteBytes(zeroPadding);
			}


			offsets.push_back(writer->Tell());
			writer->WriteBytes(child->fileData);

		}

		writer->Seek(positionsOffset);
		for (int i = 0; i < fileCount; i++) {
			writer->WriteUINT32(offsets[i]);
		}

		fileData = writer->GetData();


	}
};



#endif