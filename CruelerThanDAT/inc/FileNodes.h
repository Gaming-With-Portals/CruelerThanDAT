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
	UVD,
	TRG,
	EST,
	B1EFF,
	B1PHYS
};

enum TextureStorageMode {
	OLD, // Old ID System, Pre MGR:R
	NEW // New ID System, Post MGR:R

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

enum EstItemType {
	NONE,
	MOVE,
	PART,
	EMIF,
	TEX,
	SZSA,
	PSSA,
	RTSA,
	FVWK,
	FWK,
	EMMV,
	EMSA,
	EMPA,
	EMRA,
	EMVF,
	EMFW,
	EMFV,
	MJSG,
	MJNM,
	MJMM,
	MJDT,
	MJFN,
	MJCM,
	MJVA
};



const std::map<EstItemType, std::string> EstItemTypeToString = {
	{ NONE, "????" },
	{ MOVE, "MOVE" },
	{ PART, "PART" },
	{ EMIF, "EMIF" },
	{ TEX, "TEX" },
	{ SZSA, "SZSA" },
	{ PSSA, "PSSA" },
	{ RTSA, "RTSA" },
	{ FVWK, "FVWK" },
	{ FWK, "FWK" },
	{ EMMV, "EMMV" },
	{ EMSA, "EMSA" },
	{ EMPA, "EMPA" },
	{ EMRA, "EMRA" },
	{ EMVF, "EMVF" },
	{ EMFW, "EMFW" },
	{ EMFV, "EMFV" },
	{ MJSG, "MJSG" },
	{ MJNM, "MJNM" },
	{ MJMM, "MJMM" },
	{ MJDT, "MJDT" },
	{ MJFN, "MJFN" },
	{ MJCM, "MJCM" },
	{ MJVA, "MJVA" }
};

struct EstItem {
	EstItemType type = NONE;
	bool isValid = true;
	bool shouldRepack = true;
	virtual void Read(BinaryReader& br) {}
	virtual void Draw() {}

	virtual ~EstItem() = default;
};

struct EstMove : EstItem {
	EstMove() { type = MOVE; }

	uint32_t u_a;
	MGRVector offset;
	float unk_1;
	float top_pos_1;
	float right_pos_1;
	MGRVector move_speed;
	MGRVector move_small_speed;
	std::vector<float> u_1; // 6
	float angle;
	std::vector<float> u_2; // 13
	float scale;
	std::vector<float> u_3; // 16
	MGRColor color;
	std::vector<float> u_4; // 4
	uint16_t unk_2;
	uint16_t smooth_appearance;
	MGRColor effect_size_limit;
	float smooth_disappear;
	std::vector<float> u_5; // 32

	void Read(BinaryReader& br) override {
		u_a = br.ReadUINT32();
		offset.x = br.ReadFloat();
		offset.y = br.ReadFloat();
		offset.z = br.ReadFloat();
		unk_1 = br.ReadFloat();
		top_pos_1 = br.ReadFloat();
		right_pos_1 = br.ReadFloat();
		move_speed.x = br.ReadFloat();
		move_speed.y = br.ReadFloat();
		move_speed.z = br.ReadFloat();
		move_small_speed.x = br.ReadFloat();
		move_small_speed.y = br.ReadFloat();
		move_small_speed.z = br.ReadFloat();
		u_1 = br.ReadFloatArray(6);
		angle = br.ReadFloat();
		u_2 = br.ReadFloatArray(13);
		scale = br.ReadFloat();
		u_3 = br.ReadFloatArray(16);
		color.r = br.ReadFloat();
		color.g = br.ReadFloat();
		color.b = br.ReadFloat();
		color.a = br.ReadFloat();
		u_4 = br.ReadFloatArray(4);
		unk_2 = br.ReadUINT16();
		smooth_appearance = br.ReadUINT16();
		effect_size_limit.r = br.ReadFloat();
		effect_size_limit.g = br.ReadFloat();
		effect_size_limit.b = br.ReadFloat();
		effect_size_limit.a = br.ReadFloat();
		smooth_disappear = br.ReadFloat();
		u_5 = br.ReadFloatArray(32);
	}

	void Draw() override {
		ImGui::InputFloat3("Offset", offset);
		ImGui::InputFloat3("Move Speed", move_speed);
		ImGui::InputFloat3("Move Small Speed", move_small_speed);
		ImGui::InputFloat("Rotation", &angle);
		ImGui::InputFloat("Scale", &scale);
		ImGui::ColorEdit4("Color", color);
		ImGui::InputFloat4("Effect Size Limit", effect_size_limit);
	}

};

struct EstPart : EstItem {
	EstPart() { type = PART; }

	int16_t anchorBone;
	int16_t u_b;
	uint32_t u_c;
	uint32_t u_d;
	std::vector<uint16_t> u_1; // 8
	std::vector<uint32_t> u_2; // 9

	void Read(BinaryReader& br) override {
		anchorBone = br.ReadINT16();
		u_b = br.ReadINT16();
		u_c = br.ReadUINT32();
		u_d = br.ReadUINT32();
		u_1 = br.ReadUINT16Array(8);
		u_2 = br.ReadUINT32Array(9);
	}
};

struct EstEmif : EstItem {
	EstEmif() { type = EMIF; }
};

struct EstTex : EstItem {
	EstTex() { type = TEX; }
};

struct EstSzsa : EstItem {
	EstSzsa() { type = SZSA; }
};

struct EstPssa : EstItem {
	EstPssa() { type = PSSA; }
};

struct EstRtsa : EstItem {
	EstRtsa() { type = RTSA; }
};

struct EstFvwk : EstItem {
	EstFvwk() { type = FVWK; }
};

struct EstFwk : EstItem {
	EstFwk() { type = FWK; }
};

struct EstEmmv : EstItem {
	EstEmmv() { type = EMMV; }
};

struct EstEmsa : EstItem {
	EstEmsa() { type = EMSA; }
};

struct EstEmpa : EstItem {
	EstEmpa() { type = EMPA; }
};

struct EstEmra : EstItem {
	EstEmra() { type = EMRA; }
};

struct EstEmvf : EstItem {
	EstEmvf() { type = EMVF; }
};

struct EstEmfv : EstItem {
	EstEmfv() { type = EMFV; }
};

struct EstEmfw : EstItem {
	EstEmfw() { type = EMFW; }
};

struct EstMjsg : EstItem {
	EstMjsg() { type = MJSG; }
};

struct EstMjcm : EstItem {
	EstMjcm() { type = MJCM; }
};

struct EstMjnm : EstItem {
	EstMjnm() { type = MJNM; }
};

struct EstMjmm : EstItem {
	EstMjmm() { type = MJMM; }
};

struct EstMjdt : EstItem {
	EstMjdt() { type = MJDT; }
};

struct EstMjfn : EstItem {
	EstMjfn() { type = MJFN; }
};

struct EstMjva : EstItem {
	EstMjva() { type = MJVA; }
};


struct EstRecord {
	std::vector<EstItem*> types;
};

class EstFileNode : public FileNode {
public:
	std::vector<EstRecord> records;

	EstFileNode(std::string fName);
	void LoadFile() override;

	void RenderGUI(CruelerContext* ctx);

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
	bool isValid = true;
	unsigned int vertexBuffer;
	unsigned int indexBuffer;
	unsigned int vao;
	std::vector<CUSTOMVERTEX> vertexes;
	std::vector<unsigned short> indexes;
	std::vector<unsigned int> indexes_wmb3;
	int indexCount;
	int vertexCount;
	int materialID;
	unsigned int drawMethod = GL_TRIANGLES;
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
	WMB0_BAY1,
	WMB0_BAY2
};

class TrgFileNode : public FileNode {
public:
	TrgFileNode(std::string fName);
	void LoadFile() override;
	void SaveFile() override;

};


class WmbFileNode : public FileNode {
private:
	void LoadModelWMB0(BinaryReader& br);
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
	bool hasCutdata = false;

	WmbFileNode(std::string fName);

	void GetBoneNames();

	std::string GetBoneNameFromID(int boneID);

	void LoadFile() override;
	void SaveFile() override;


	void PopupOptions(CruelerContext *ctx) override;

	void RenderMesh(CruelerContext *ctx);
	void RenderPreviewMesh(CruelerContext* ctx);

	void RemoveCuttingDataWMB4();

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
	std::unordered_map<unsigned int, unsigned int> textureInfo;
	TextureStorageMode mode;

	DatFileNode(std::string fName);

	void LoadFile() override;



	void SaveFile() override;
};

class BayoEffNode : public FileNode {
public:
	BayoEffNode(std::string fName);

	void LoadFile() override;
	void SaveFile() override;

};

enum BayoPhysicsFileType {
	B1_CLP,
	B1_CLH,
	B1_CLW,
	B1_NONE
};

struct Bayo1ClpUnit {
	int no = 0;
	int noUp = 0;
	int noDown = 0;
	int noSide = 0;
	int noPoly = 0;
	int noFix = 0;
	float rotLimit = 0;
	MGRVector offset = { 0, 0, 0 };
};

struct Bayo1ClpHeader {
	std::vector<Bayo1ClpUnit> works;

	uint32_t m_Num;
	float m_LimitSpringRate;
	float m_SpdRate;
	float m_Stretchy;
	int m_BundleNum;
	int m_BundleNum2;
	float m_Thick;
	MGRVector m_gravityVec;
	int m_GravityPartsNo;
	float m_FirstBundleRate;
	MGRVector m_WindVec;
	int m_WindPartsNo;
	MGRVector m_WindOffset;
	float m_WindSin;
	float m_HitAdjustRate;
};


class BayoClpClhClwFileNode : public FileNode {
public:
	BayoPhysicsFileType type = B1_NONE;

	Bayo1ClpHeader clpHeader;

	BayoClpClhClwFileNode(std::string fName);

	void LoadFile() override;
	void SaveFile() override;

	void RenderGUI(CruelerContext* ctx);
	
};