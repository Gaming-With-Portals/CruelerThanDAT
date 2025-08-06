#pragma once

#include "pch.hpp"
#include "Criware.h"
#include <imgui.h>
#include <thread>
#include <future>
#include "BinaryHandler.h"
#include "FileUtils.h"
#include "FileNodes.h"

namespace fs = std::filesystem;


namespace CRILAYLA {
	int get_next_bits(const std::vector<char>& input, int& input_offset, uint8_t& bit_pool, int& bits_left, int num_bits);

	std::vector<char> DecompressCRILAYLA(std::vector<char> input, int USize);
}

struct CPKMGMTFile {
	int type = -1;
	std::string path;
	std::string name;
	std::vector<CPKMGMTFile> children;
};

struct CRIROW {
	uint8_t  uint8;
	uint16_t uint16;
	uint32_t uint32;
	uint64_t uint64;
	float    ufloat;
	std::string str;
	std::vector<char> data;
	uint8_t type;
	size_t position;

	std::variant<
		std::monostate,  
		uint8_t,
		uint16_t,
		uint32_t,
		uint64_t,
		float,
		std::string,
		std::vector<char>
	> GetValue() const;

};


struct CRIROWS {
	std::vector<CRIROW> rows;
};

struct CRICOLUMN {
	uint8_t flag;
	std::string name;
};

struct CRIUTFTable {
	size_t offset;
	std::vector<CRICOLUMN> columns;
	std::vector<CRIROWS> rows;
};

struct CRIFILE {
	std::string name = "";
	uint64_t location = 0;
	uint64_t size = 0;
};

struct CRIFOLDER {
	std::string name = "";
	std::unordered_map<std::string, CRIFOLDER> subfolders;
	std::vector<CRIFILE> files;
};

class CPKManager {
	std::future<void> cpkLoadFuture;
	std::atomic<bool> isLoading = false;
	char searchBuf[128] = "";
	CPKMGMTFile activeFile;
	CPKMGMTFile baseFile;
	CRIFOLDER cpkBase;
	bool isCPKLoaded = false;
	int totalFileCount = 0;
	int processedFileCount = 0;
	std::string lastLoadedFilePath;
	std::vector<char> fileData;
	float progressThrobber = 0.0f;
	bool hasResetProgressBar = true;
	void MakeTree(const fs::path& dirPath, CPKMGMTFile& node);

	void RenderTree(const CruelerContext *ctx, const CPKMGMTFile& file);

	CRIUTFTable CriUTF(BinaryReader& br);

	void AddFile(CRIFOLDER& root, const std::string& dirPath, const std::string& fileName, uint64_t location, uint64_t size);

	void LoadCPK();

	std::vector<char> GetCPKFileData(CRIFILE file);

	FileNode* GetFileNodeFromCPK(CruelerContext *ctx, CRIFILE file, CRIFOLDER folder);

	bool FolderMatchesSearch(const CRIFOLDER& folder, const std::string& searchQuery);

	void DrawSubitems(CruelerContext *ctx, const CRIFOLDER& folder, const std::string& searchQuery);
	void DrawFolder(CruelerContext *ctx, const CRIFOLDER& folder, const std::string& searchQuery, bool isFirst = false);

	float GetProgress() const;

	void RipCPK_Folders(CRIFOLDER cri, fs::path basePath);


	void RipCPKToDisk(std::wstring path);


public:
	void Init(const std::string& dir);




	void Render(CruelerContext *ctx);

};