#pragma once
#include "pch.hpp"

#ifndef FILENODE_H
#define FILENODE_H
#define NOMINMAX
#include "globals.h"
#include "BinaryHandler.h"
#include "CodIcons.h"
#include "Log.h"
#include "CRC32.h"
#include "HashDataContainer.h"
#include "ImGuiExtended.h"
#include "WMB.h"
#include "CTDModel.h"
#include "UID.h"
#include "UVD.h"

class FileNode;  // Forward declaration
class UvdFileNode;  // Forward declaration
extern std::vector<FileNode*> openFiles;

namespace BXMInternal {
	static const std::vector<std::string> possibleParams = {
		"RoomNo",
		"PlayerPos",
		"isRestartPoint",
		"CameraYaw",
		"CameraEnable",
		"CameraXEnable",
		"CameraYEnable",
		"isPlWaitPayment",
	};

	std::vector<std::string> SplitString(const std::string& str, char delimiter);

	struct XMLAttribute {
		std::string value = "";
		std::string name = "";
	};

	struct XMLNode {
		std::string name = "";
		std::string value = "";
		XMLNode* parent;
		std::vector<XMLNode*> childNodes;
		std::vector<XMLAttribute*> childAttributes;

	};
}

void closeNode(FileNode* target);

namespace HelperFunction {
	FileNode* LoadNode(std::string fileName, const std::vector<char>& data, bool forceEndianess = false, bool bigEndian = false);

	int Align(int value, int alignment);

	float HalfToFloat(uint16_t h);

	bool WriteVectorToFile(const std::vector<char> dataVec, const std::string& filename);
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
	LY2,
	UID,
	UVD
};

int IntLength(int);

class FileNode {
public:
	std::string fileName;
	std::string fileExtension;
	std::vector<FileNode*> children;
	std::vector<char> fileData;
	FileNodeTypes nodeType;
	FileNode* parent;
	LPCWSTR fileFilter = L"All Files(*.*)\0*.*;\0";
	bool loadFailed = false;
	bool isEdited = false;

	static FileNode* selectedNode;


	std::string fileIcon = ICON_CI_QUESTION;
	bool fileIsBigEndian = false;
	ImVec4 TextColor = { 1.0f, 1.0f, 1.0f, 1.0f };

	FileNode(std::string fName);

	virtual ~FileNode();

	virtual void PopupOptionsEx();

	virtual void ExportFile();

	virtual void ReplaceFile();

	virtual void PopupOptions();


	virtual void Render();

	virtual void LoadFile() = 0;
	virtual void SaveFile() = 0;

	void SetFileData(const std::vector<char>& data);

	const std::vector<char>& GetFileData() const;
};

class UnkFileNode : public FileNode {
public:
	UnkFileNode(std::string fName);
	void LoadFile() override;
	void SaveFile() override;
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

	LY2FileNode(std::string fName);
	void LoadFile() override;
	void SaveFile() override;
	void RenderGUI();
};

class UvdFileNode : public FileNode {
public:
	std::vector<UvdTexture> uvdTextures;
	std::vector<UvdEntry> uvdEntries;

	UvdFileNode(std::string fName);
	void LoadFile() override;
	void RenderGUI();
	void SaveFile() override;
};

class UidFileNode : public FileNode {
public:
	UIDHeader uidHeader;
	UvdFileNode* pairedUVD = nullptr;
	std::vector<UIDEntry1> UIDEntry1List;
	std::vector<UIDEntry2> UIDEntry2List;
	std::vector<UIDEntry3> UIDEntry3List;

	bool UIDVisualize = false;
	UidFileNode(std::string fName);
	void LoadFile() override;
	void SaveFile() override;
	void RenderGUI();
};

class MotFileNode : public FileNode {
public:
	MotFileNode(std::string fName);
	void LoadFile() override;
	void SaveFile() override;
};

class BxmFileNode : public FileNode {
public:
	BXMInternal::XMLNode* baseNode;
	int infoOffset;
	int dataOffset;
	int stringOffset;

	std::string xmlData = "";
	std::vector<std::array<char, 32>> roomBuffers;
	std::vector<char> xmlBuffer;

	BxmFileNode(std::string fName);
	std::string ConvertToXML(BXMInternal::XMLNode* node, int indentLevel = 0);
	BXMInternal::XMLNode* ReadXMLNode(BinaryReader reader, BXMInternal::XMLNode* parent);
	std::vector<BXMInternal::XMLNode*> FlattenXMLTree(BXMInternal::XMLNode* root);
	void RenderGUI();
	void LoadFile() override;
	void SaveFile() override;
};

class WtbFileNode : public FileNode {
public:
	int textureCount = 0;
	std::vector<int> textureOffsets;
	std::vector<int> textureSizes;
	std::vector<int> textureFlags;
	std::vector<int> textureIdx;
	std::vector<std::vector<char>> textureData;

	WtbFileNode(std::string fName);
	void LoadFile() override;
	void SaveFile() override;
};

class WemFileNode : public FileNode {
public:
	WemFileNode(std::string fName);
	void LoadFile() override;
	void SaveFile() override;
};

class CruelerBatch {
	// i hate everything
	// I can count how many seconds until I kill myself on one hand

	// sounds like skill issue - Chloe
public:
	LPDIRECT3DVERTEXBUFFER9 vertexBuffer;
	LPDIRECT3DINDEXBUFFER9 indexBuffer;
	std::vector<CUSTOMVERTEX> vertexes;
	std::vector<unsigned short> indexes;
	int indexCount;
	int vertexCount;
	int materialID;
};

class CruelerMesh {
public:
	std::vector<CruelerBatch*> batches;
	std::string name = "N/A";
	bool visibility = true;
	int vtxfmt = 0;
	int structSize;

	std::vector<CTDMaterial*> materials;
};

class WmbFileNode : public FileNode {
public:
	WMBVector meshOffset;
	WMBVector scaleOffset;
	WMBVector meshRotation;
	std::vector<CruelerMesh*> displayMeshes;
	std::vector<CTDMaterial> materials;
	FileNode* scrNode;
	bool isSCR = false;
	float rotationAngle = 0;;
	float scaleFactor = 0.4f;

	WmbFileNode(std::string fName);
	void LoadFile() override;
	void SaveFile() override;
	void PopupOptions() override;
	void RenderMesh();
	void RenderGUI();
};

struct SCRMesh {
	unsigned int offset;
	std::string name;
	WMBVector position;
	WMBVector rotation;
	WMBVector scale;

	void Read(BinaryReader& br);
};

class ScrFileNode : public FileNode {
public:
	ScrFileNode(std::string fName);
	void LoadFile() override;
	void SaveFile() override;
};

class BnkFileNode : public FileNode {
public:
	BnkFileNode(std::string fName);
	void LoadFile() override;
	void SaveFile() override;
};

class DatFileNode : public FileNode {
public:
	DatFileNode(std::string fName);
	void LoadFile() override;
	void SaveFile() override;
};

#endif
