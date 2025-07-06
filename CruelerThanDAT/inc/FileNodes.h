#pragma once
#include "pch.hpp"

#define NOMINMAX
#include "CTDSettings.h"
#include "globals.h"
#include "tinyxml2.h"
#include "imgui_impl_opengl3.h"
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
/**/

class CPKManager;
class ThemeManager;

struct CruelerContext {
	bool hasHandledArguments = false;
	std::vector<std::string> args;

	std::string downloadURL;

	CTDSettings config;

	SDL_Window *window;
	glm::uvec2 winSize;
	float progress;

	bool dragging;
	int dragOffsetX;
	int dragOffsetY;

	bool viewportShow;
	glm::uvec2 viewportLastSize;
	uint32_t viewportFrameBuffer;
	uint32_t viewportDepthRenderBuffer;
	uint32_t viewportColorTexture;

	glm::mat4 projMatrix; // TODO: Delete
	glm::mat4 viewMatrix;
	glm::vec3 target;
	glm::vec3 cameraPos;
	// TODO: Better camera controls
	float yaw;
	float pitch;
	float radius;
	
	std::unordered_map<unsigned int, unsigned int> textureMap;
	std::unordered_map<unsigned int, std::vector<char>> rawTextureInfo;

	nlohmann::json plNames;

	bool showAllScrMeshes = false;
	bool cruelerLog = true;

	ThemeManager *themeManager;
	CPKManager *cpkManager;
};



class FileNode;  // Forward declaration
class UvdFileNode;  // Forward declaration
extern std::vector<FileNode*> openFiles;

namespace BXMInternal {
	extern const std::vector<std::string> possibleParams;

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
	bool WorldToScreen(const glm::vec3& worldPos, glm::vec2& screenPos, const glm::mat4& view, const glm::mat4& proj, const GLint* viewport);

	FileNode* LoadNode(std::string fileName, const std::vector<char>& data, bool forceEndianess = false, bool bigEndian = false);

	int Align(int value, int alignment);


	float HalfToFloat(uint16_t h);
	MGRVector DecodeNormal(uint32_t packed);
	glm::vec4 DecodeTangent(uint32_t packedTangent);

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
	FileNode* parent = nullptr;
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

	virtual void PopupOptions(CruelerContext *ctx);


	virtual void Render(CruelerContext *ctx);

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

	void RenderGUI(CruelerContext *ctx);



};

class UvdFileNode : public FileNode {
public:
	std::vector<UvdTexture> uvdTextures;
	std::vector<UvdEntry> uvdEntries;

	UvdFileNode(std::string fName);
	void LoadFile() override;

	void RenderGUI(CruelerContext *ctx);

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

	void RenderGUI(CruelerContext *ctx);

};






class MotFileNode : public FileNode {
public:

	MotFileNode(std::string fName);
	void LoadFile() override;
	void SaveFile() override;
};

class BxmFileNode : public FileNode {
public:
	std::string ConvertToXML(BXMInternal::XMLNode* node, int indentLevel = 0);

	TextEditor ImEditor;
	BXMInternal::XMLNode* baseNode;
	int infoOffset;
	int dataOffset;
	int stringOffset;

	std::string xmlData = "";
	std::vector<std::array<char, 32>> roomBuffers;
	std::vector<char> xmlBuffer;

	BxmFileNode(std::string fName);

	void RenderGUI(CruelerContext *ctx);


	BXMInternal::XMLNode* ReadXMLNode(BinaryReader reader, BXMInternal::XMLNode* parent);

	void LoadFile() override;

	std::vector<BXMInternal::XMLNode*> FlattenXMLTree(BXMInternal::XMLNode* root);

	BXMInternal::XMLNode* ConvertXML(tinyxml2::XMLElement* element, BXMInternal::XMLNode* parent = nullptr);

	int TinyXMLToGwpBXM();

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

struct CruelerBone {
	int parentIndex;
	int boneID;
	bool selected = false;
	glm::vec3 localPosition;
	glm::vec3 worldPosition;
	glm::mat4 localTransform;
	glm::mat4 combinedTransform;
	glm::mat4 offsetMatrix;
};

class CruelerBatch {
	// i hate everything
	// I can count how many seconds until I kill myself on one hand
public:
	//LPDIRECT3DVERTEXBUFFER9 vertexBuffer;
	//LPDIRECT3DINDEXBUFFER9 indexBuffer;
	unsigned int vertexBuffer;
	unsigned int indexBuffer;
	unsigned int vao;
	std::vector<CUSTOMVERTEX> vertexes;
	std::vector<unsigned short> indexes;
	std::vector<unsigned int> indexes_wmb3;
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

enum WmbVersionFormat {
	WMB4_MGRR,
	WMB3_BAY3,
	WMB0_BAY1
};

class WmbFileNode : public FileNode {
private:
	void LoadModelWMB3(BinaryReader& br);
public:
	WmbVersionFormat wmbVersion = WMB4_MGRR;
	WMBVector meshOffset;
	WMBVector scaleOffset;
	WMBVector meshRotation;
	std::vector<CruelerMesh*> displayMeshes;
	std::vector<CTDMaterial> materials;
	std::vector<CruelerBone> bones;
	std::unordered_map<int, std::string> boneNames;
	FileNode* scrNode;
	std::string boneNameSourceFile = "";
	bool isSCR = false;
	float rotationAngle = 0;;
	float scaleFactor = 0.4f;
	bool visualizerPopout = false;
	bool visualizerShowBones = false;
	bool visualizerWireframe = false;

	WmbFileNode(std::string fName);

	void GetBoneNames();

	std::string GetBoneNameFromID(int boneID);

	void LoadFile() override;
	void SaveFile() override;


	void PopupOptions(CruelerContext *ctx) override;

	void RenderMesh(CruelerContext *ctx);


	struct BoneClothWK {
		int no = 0;
		int noUp = 0;
		int noDown = 0;
		int noSide = 0;
		int noPoly = 0;
		float rotLimit = 1.0f;
		MGRVector offset = { 0, -0.10000000149011612f ,0 };
		float m_OriginalRate = 0.0f;
	};

	struct BonePhysicsContainer {
		CruelerBone rootBone;
		int rootBoneIndex = 0;
		std::unordered_map<int, std::vector<int>> boneChildren;
		int boneRunLength;
		int boneRunCounts;
		int boneStartIndiceOffset;
		std::vector<int> boneOrder;
		std::vector<BoneClothWK> boneClothWorks;
	};

	bool arePhysicsInitalized = false;
	BonePhysicsContainer bonePhysData;

	bool InitData(int rootBoneID);

	void GenerateData();

	void PhysicsPanel();

	void DrawBoneTree(int boneIdx);

	void RenderBoneGUI();


	void RenderGUI(CruelerContext *ctx);

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

struct BnkHircObject {
	uint8_t type;
	uint32_t size;
	uint32_t uid;
};

struct BnkEventObject : BnkHircObject {
	std::vector<unsigned int> ids;
};

extern WWISE::Data002BlobData* Data002Blob; // TODO: Do we need to expose this?

class BnkFileNode : public FileNode {
public:
	BnkFileNode(std::string fName);

	std::vector<BnkHircObject> hircObjects;

	void LoadFile() override;

	std::string GetBNKHircID(BnkHircObject obj);

	void RenderGUI(CruelerContext *ctx);

	void SaveFile() override;
};



class DatFileNode : public FileNode {
public:
	DatFileNode(std::string fName);

	void LoadFile() override;



	void SaveFile() override;
};
