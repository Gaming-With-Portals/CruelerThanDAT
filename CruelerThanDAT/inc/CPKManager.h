#pragma once
#include <iostream>
#include <filesystem>
#include <string>
#include <vector>
#include <imgui.h>
#include <windows.h>
#include <shobjidl.h> 
#include "BinaryHandler.h"
#include "FileUtils.h"
#include <variant>
#include <unordered_map>
#include <../CruelerThanDAT.h>

namespace fs = std::filesystem;


namespace CRILAYLA {
	int get_next_bits(const std::vector<char>& input, int& input_offset, uint8_t& bit_pool, int& bits_left, int num_bits) {
		int out_bits = 0;

		for (int i = 0; i < num_bits; i++) {
			if (bits_left == 0) {
				// Read next byte into bit_pool, assuming input_offset moves from end backwards
				// The C# code processes input from the end backwards, so we adjust input_offset accordingly.
				bit_pool = static_cast<uint8_t>(input[input_offset--]);
				bits_left = 8;
			}

			out_bits = (out_bits << 1) | ((bit_pool >> 7) & 1); // Get the most significant bit
			bit_pool <<= 1; // Shift left to discard the read bit
			bits_left--;
		}
		return out_bits;
	}

	std::vector<char> DecompressCRILAYLA(std::vector<char> input, int USize) {
		std::vector<char> result;

		BinaryReader br(input, false); 

		br.Seek(8);
		int uncompressed_size = br.ReadINT32();
		int uncompressed_header_offset = br.ReadINT32();

		result.resize(uncompressed_size + 0x100);

		std::copy(input.begin() + uncompressed_header_offset + 0x10,
			input.begin() + uncompressed_header_offset + 0x10 + 0x100,
			result.begin());

		int input_end = input.size() - 0x100 - 1; 
		int input_offset = input_end;
		int output_end = 0x100 + uncompressed_size - 1;
		uint8_t bit_pool = 0;
		int bits_left = 0, bytes_output = 0;
		std::vector<int> vle_lens = { 2, 3, 5, 8 };

		while (bytes_output < uncompressed_size) {
			if (get_next_bits(input, input_offset, bit_pool, bits_left, 1) > 0) {

				int backreference_offset = output_end - bytes_output + get_next_bits(input, input_offset, bit_pool, bits_left, 13) + 3;
				int backreference_length = 3;
				int vle_level;

				for (vle_level = 0; vle_level < vle_lens.size(); vle_level++) {
					int this_level = get_next_bits(input, input_offset, bit_pool, bits_left, vle_lens[vle_level]);
					backreference_length += this_level;
					if (this_level != ((1 << vle_lens[vle_level]) - 1)) break;
				}

				if (vle_level == vle_lens.size()) {
					int this_level;
					do {
						this_level = get_next_bits(input, input_offset, bit_pool, bits_left, 8);
						backreference_length += this_level;
					} while (this_level == 255);
				}

				for (int i = 0; i < backreference_length; i++) {
					if (output_end - bytes_output < 0 || backreference_offset < 0 || backreference_offset >= result.size()) {
					}
					result[output_end - bytes_output] = result[backreference_offset--];
					bytes_output++;
				}
			}
			else {

				if (output_end - bytes_output < 0) {

				}
				result[output_end - bytes_output] = static_cast<uint8_t>(get_next_bits(input, input_offset, bit_pool, bits_left, 8));
				bytes_output++;
			}
		}

		return result;
	}
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
	> GetValue() const {
		switch (type) {
		case 0:
		case 1: return uint8;
		case 2:
		case 3: return uint16;
		case 4:
		case 5: return uint32;
		case 6:
		case 7: return uint64;
		case 8:     return ufloat;
		case 0xA:   return str;
		case 0xB:   return data;
		default:    return std::monostate{};
		}
	}

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
	std::string name;
	uint64_t location;
	uint64_t size;
};

struct CRIFOLDER {
	std::string name;
	std::unordered_map<std::string, CRIFOLDER> subfolders;
	std::vector<CRIFILE> files;
};

enum COLUMN_FLAGS : uint8_t {
	STORAGE_MASK = 0xF0,
	STORAGE_NONE = 0x00,
	STORAGE_PERROW = 0x20,
	STORAGE_CONSTANT = 0x30,
	STORAGE_ZERO = 0x10,

	TYPE_MASK = 0x0f,
	TYPE_DATA = 0x0b,
	TYPE_STRING = 0x0a,
	TYPE_FLOAT = 0x08,
	TYPE_8BYTE2 = 0x07,
	TYPE_8BYTE = 0x06,
	TYPE_4BYTE2 = 0x05,
	TYPE_4BYTE = 0x04,
	TYPE_2BYTE2 = 0x03,
	TYPE_2BYTE = 0x02,
	TYPE_1BYTE2 = 0x01,
	TYPE_1BYTE = 0x00,
};

class CPKManager {
	char searchBuf[128] = "";
	CPKMGMTFile activeFile;
	CPKMGMTFile baseFile;
	CRIFOLDER cpkBase;
	bool isCPKLoaded = false;
	int totalFileCount = 0;
	int processedFileCount = 0;
	std::vector<char> fileData;
	void MakeTree(const fs::path& dirPath, CPKMGMTFile& node) {
		if (!fs::exists(dirPath)) {
			return;
		}

		for (const auto& entry : fs::directory_iterator(dirPath)) {
			if (entry.is_directory()) {
				CPKMGMTFile folder;
				folder.name = entry.path().filename().string();
				folder.type = 1;
				MakeTree(entry.path(), folder);

				if (!folder.children.empty())
					node.children.push_back(std::move(folder));
			}
			else if (entry.path().has_extension()) {
				if (entry.path().extension() == ".cpk") {
					CPKMGMTFile file;
					file.name = entry.path().filename().string();
					file.path = entry.path().string();
					file.type = 0;
					node.children.push_back(std::move(file));
				}

			}
		}
	}

	void RenderTree(const CPKMGMTFile& file) {
		if (file.type == 1) {
			if (ImGui::TreeNode(file.name.c_str())) {
				for (const auto& child : file.children) {
					RenderTree(child);
				}
				ImGui::TreePop();
			}
		}
		else if (file.type == 8) {
			for (const auto& child : file.children) {
				RenderTree(child);
			}
		}
		else {
			if (ImGui::TreeNodeEx(file.name.c_str(), ImGuiTreeNodeFlags_Leaf)) {

				if (ImGui::IsItemClicked()) {
					activeFile = file;
				}

				ImGui::TreePop();
			}
		}
	}

	CRIUTFTable CriUTF(BinaryReader& br) {
		size_t UTFOffset = br.Tell();
		CRIUTFTable table = CRIUTFTable();
		if (br.ReadString(4) != "@UTF") {
			return table;
		}
		br.SetEndianess(true);

		

		uint32_t table_size = br.ReadUINT32();
		uint32_t rows_offset = br.ReadUINT32();
		uint32_t strings_offset = br.ReadUINT32();
		uint32_t data_offset = br.ReadUINT32();
		uint32_t table_name = br.ReadUINT32();
		uint16_t column_count = br.ReadUINT16();
		uint16_t row_length = br.ReadUINT16();
		uint16_t row_count = br.ReadUINT32();

		for (int i = 0; i < column_count; i++) {
			CRICOLUMN col = CRICOLUMN();

			uint8_t flag = br.ReadINT8();
			if (flag == 0) {
				br.ReadBytes(3);
				flag = br.ReadINT8();
			}

			col.flag = flag;
			uint32_t title_offset = br.ReadUINT32();
			size_t columnOffset = br.Tell();
			br.Seek(title_offset + UTFOffset + 8 + strings_offset);
			col.name = br.ReadNullTerminatedString();
			br.Seek(columnOffset);
			table.columns.push_back(col);

		}

		for (int j = 0; j < row_count; j++) {
			br.Seek(rows_offset + UTFOffset + 8 + (j * row_length));

			CRIROWS current_entry = CRIROWS();

			for (int i = 0; i < column_count; i++) {
				CRIROW current_row = CRIROW();

				uint8_t flags = table.columns[i].flag;
				uint8_t storage_flag = flags & STORAGE_MASK;

				if (storage_flag == STORAGE_NONE ||
					storage_flag == STORAGE_ZERO ||
					storage_flag == STORAGE_CONSTANT)
				{
					current_entry.rows.push_back(current_row);
					continue;
				}

				current_row.type = table.columns[i].flag & TYPE_MASK;
				current_row.position = br.Tell();

				switch (current_row.type) {
				case 0: // fallthrough
				case 1:
					current_row.uint8 = br.ReadINT8();
					break;
				case 2: // fallthrough
				case 3:
					current_row.uint16 = br.ReadUINT16();
					break;
				case 4: // fallthrough
				case 5:
					current_row.uint32 = br.ReadUINT32();
					break;
				case 6: // fallthrough
				case 7:
					current_row.uint64 = br.ReadUINT64();
					break;
				case 8:
					current_row.ufloat = br.ReadFloat();
					break;
				case 0xA: {
					long position = br.ReadINT32() + strings_offset + UTFOffset + 8;
					long original = br.Tell();
					br.Seek(position);
					current_row.str = br.ReadNullTerminatedString();
					br.Seek(original);
					break;
				}
				case 0xB: {
					long position = br.ReadINT32() + data_offset + UTFOffset + 8;
					current_row.position = position;
					current_row.data = br.ReadBytes(br.ReadINT32());
					break;
				}
				default:
					current_row.position = 0;
					std::cout << "ERROR!" << std::endl;

				}
				current_entry.rows.push_back(current_row);

			}
			table.rows.push_back(current_entry);
		}


		return table;
	}

	void AddFile(CRIFOLDER& root, const std::string& dirPath, const std::string& fileName, uint64_t location, uint64_t size) {
		CRIFOLDER* current = &root;

		std::istringstream ss(dirPath);
		std::string segment;

		while (std::getline(ss, segment, '/')) {
			current = &current->subfolders[segment];
			current->name = segment;
		}

		current->files.push_back({ fileName, location, size });
		totalFileCount += 1;
	}

	void LoadCPK() {
		totalFileCount = 0;
		ReadFileIntoVector(activeFile.path, fileData);
		BinaryReader br = BinaryReader(fileData, false);
		if (br.ReadString(4) != "CPK ") {
			return;
		}

		cpkBase.files.clear();
		cpkBase.subfolders.clear();

		int unk1 = br.ReadUINT32();
		long utf_size = br.ReadUINT64();

		CRIUTFTable cpkutf = CriUTF(br);

		std::unordered_map <std::string, CRIROW> datatable;
		
		for (int i = 0; i < cpkutf.columns.size(); i++) {
			datatable[cpkutf.columns[i].name] = cpkutf.rows[0].rows[i];
		}

		uint64_t tocOffset = std::get<uint64_t>(datatable["TocOffset"].GetValue());
		br.Seek(tocOffset);
		if (br.ReadString(4) == "TOC ") {
			br.ReadUINT32();
			br.ReadUINT64();
			CRIUTFTable tocutf = CriUTF(br);
			std::unordered_map <std::string, CRIROW> toctable;

			

			for (int i = 0; i < tocutf.rows.size(); i++) {
				toctable.clear();
				for (int j = 0; j < tocutf.columns.size(); j++) {
					toctable[tocutf.columns[j].name] = tocutf.rows[i].rows[j];
				}

				auto val = toctable["FileOffset"].GetValue();
				size_t offset = 0;

				if (std::holds_alternative<uint32_t>(val)) {
					offset = static_cast<size_t>(std::get<uint32_t>(val));
				}
				else if (std::holds_alternative<uint64_t>(val)) {
					offset = static_cast<size_t>(std::get<uint64_t>(val));
				}
				else {
					std::cerr << "Invalid type in FileOffset!" << std::endl;
				}

				uint32_t size = std::get<uint32_t>(toctable["FileSize"].GetValue());
				std::string path = toctable["DirName"].str;
				if (toctable["DirName"].str == "") {
					path = "";
				}

 				AddFile(cpkBase, path, std::get<std::string>(toctable["FileName"].GetValue()), offset + tocOffset, size);

			}



		}


	}

	std::vector<char> GetCPKFileData(CRIFILE file) {
		if (!file.size > 0) {
			return {};
		}

		BinaryReader br = BinaryReader(fileData, false);
		br.Seek(file.location);
		std::string compressionAlg = br.ReadString(8);
		if (compressionAlg == "CRILAYLA") {

			br.Seek(file.location);
			return CRILAYLA::DecompressCRILAYLA(br.ReadBytes(file.size), file.size);
		}
		else {
			br.Seek(file.location);
			return br.ReadBytes(file.size);
		}
	}

	FileNode* GetFileNodeFromCPK(CRIFILE file, CRIFOLDER folder) {
		// TODO: Use GetCPKFileData instead.
		BinaryReader br = BinaryReader(fileData, false);
		br.Seek(file.location);
		std::string compressionAlg = br.ReadString(8);
		FileNode* node = nullptr;
		if (compressionAlg == "CRILAYLA") {

			br.Seek(file.location);
			node = HelperFunction::LoadNode(file.name, CRILAYLA::DecompressCRILAYLA(br.ReadBytes(file.size), file.size), false, false);
		}
		else {
			br.Seek(file.location);
			node = HelperFunction::LoadNode(file.name, br.ReadBytes(file.size), false, false);
		}

		if (node->fileExtension == "dat") {
			fs::path dttPath = file.name;
			dttPath.replace_extension(".dtt");
			FileNode* wtaNode = nullptr;
			FileNode* wtpNode = nullptr;
			for (CRIFILE& dfile : folder.files) {


				if (dfile.name == dttPath.string()) {
					for (FileNode* node : node->children) {

						if (node->fileExtension == "wta") {
							wtaNode = node;
						}
					}



					FileNode* dttNode = GetFileNodeFromCPK(dfile, folder);
					for (FileNode* node : dttNode->children) {
						if (node->fileExtension == "wtp") {
							wtpNode = node;
						}
						else if (node->fileExtension == "wtb") {
							wtpNode = node;
							wtaNode = node;
						}
					}
				}


			}

			BinaryReader wta = BinaryReader(wtaNode->fileData);
			BinaryReader wtp = BinaryReader(wtpNode->fileData);
			if (wtaNode && wtpNode) {
				DX9WTAWTPLoad(wta, wtp);
			}
			
			

		}

		

		return node;
	}

	bool FolderMatchesSearch(const CRIFOLDER& folder, const std::string& searchQuery) {
		if (folder.name.find(searchQuery) != std::string::npos)
			return true;

		for (const auto& file : folder.files) {
			if (file.name.find(searchQuery) != std::string::npos)
				return true;
		}

		for (const auto& sub : folder.subfolders) {
			if (FolderMatchesSearch(sub.second, searchQuery))
				return true;
		}

		return false;
	}

	void DrawSubitems(const CRIFOLDER& folder, const std::string& searchQuery) {
		bool showFolder = searchQuery.empty() || folder.name.find(searchQuery) != std::string::npos;

		for (const auto& sub : folder.subfolders) {
			if (FolderMatchesSearch(sub.second, searchQuery)) {
				DrawFolder(sub.second, searchQuery);
			}
		}

		for (const auto& file : folder.files) {
			if (searchQuery.empty() || file.name.find(searchQuery) != std::string::npos) {
				if (ImGui::TreeNodeEx(file.name.c_str(), ImGuiTreeNodeFlags_Leaf)) {
					if (ImGui::IsItemClicked()) {
						isCPKLoaded = false;
						activeFile.type = 9;
						openFiles.push_back(GetFileNodeFromCPK(file, folder));
						fileData.clear();
					}
					ImGui::TreePop();
				}
			}
		}
	}
	void DrawFolder(const CRIFOLDER& folder, const std::string& searchQuery, bool isFirst = false) {

		if (isFirst) {
			DrawSubitems(folder, searchQuery);
		}
		else {
			if (searchQuery.empty() || folder.name.find(searchQuery) != std::string::npos || FolderMatchesSearch(folder, searchQuery)) {
				if (ImGui::TreeNode(folder.name.c_str())) {
					DrawSubitems(folder, searchQuery);
					ImGui::TreePop();
				}
			}
		}
	}

	void RipCPK_Folders(CRIFOLDER cri, fs::path basePath) {

		// cri about it :laughing emoji:
		for (CRIFILE file : cri.files) {
			fs::path outputPath = basePath / file.name;
			std::ofstream outFile(outputPath, std::ios::binary);
			outFile.write(GetCPKFileData(file).data(), GetCPKFileData(file).size());
			outFile.close();
			processedFileCount += 1;

			std::cout << cri.name << "/" << file.name
				<< std::format(" ({:.2f} MB)", (file.size / (1024.0 * 1024.0)))
				<< std::format(" | {:.2f}%", ((float)processedFileCount / (float)totalFileCount) * 100.0f)
				<< std::endl;
		}


		for (const auto& [name, crifolder] : cri.subfolders) {
			fs::path fullPath = basePath / name;
			fs::create_directory(fullPath);
			RipCPK_Folders(crifolder, fullPath);


		}

	}


	void RipCPKToDisk(std::wstring path) {
		processedFileCount = 0;
		::ShowWindow(::GetConsoleWindow(), SW_SHOW);
		std::cout << "Ripping CPK..." << std::endl;
		RipCPK_Folders(cpkBase, path);
		::ShowWindow(::GetConsoleWindow(), SW_HIDE);
	}


public:
	void Init(const std::string& dir) {
		activeFile = CPKMGMTFile();
		activeFile.type = 9; // special type
		baseFile = CPKMGMTFile();
		baseFile.type = 8; // special type
		MakeTree(dir, baseFile);
	}




	void Render() {
		if (activeFile.type == 9) {
			if (ImGui::Button("Select CPK Directory", ImVec2(334, 20))) {
				HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
				if (SUCCEEDED(hr))
				{
					IFileDialog* pFileDialog = nullptr;
					hr = CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pFileDialog));

					if (SUCCEEDED(hr))
					{
						DWORD dwOptions;
						pFileDialog->GetOptions(&dwOptions);
						pFileDialog->SetOptions(dwOptions | FOS_PICKFOLDERS | FOS_FORCEFILESYSTEM);
						hr = pFileDialog->Show(nullptr);
						if (SUCCEEDED(hr))
						{
							IShellItem* pItem;
							hr = pFileDialog->GetResult(&pItem);
							if (SUCCEEDED(hr))
							{
								PWSTR pszFilePath = nullptr;
								hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);
								if (SUCCEEDED(hr))
								{
									std::wstring outputPath = pszFilePath;

									MakeTree(outputPath, baseFile);


									CoTaskMemFree(pszFilePath);
								}

								pItem->Release();
							}
						}
						pFileDialog->Release();
					}
					CoUninitialize();
				}
			}


			RenderTree(baseFile);
		}
		else {
			ImGui::SeparatorText(activeFile.name.c_str());
			ImGui::InputText("Search", searchBuf, sizeof(searchBuf));
			if (ImGui::Button("Unload")) {
				isCPKLoaded = false;
				activeFile.type = 9;
				fileData.clear();
			}
			ImGui::SameLine();
			if (ImGui::Button("Rip")) {
				HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
				if (SUCCEEDED(hr))
				{
					IFileDialog* pFileDialog = nullptr;
					hr = CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pFileDialog));

					if (SUCCEEDED(hr))
					{
						DWORD dwOptions;
						pFileDialog->GetOptions(&dwOptions);
						pFileDialog->SetOptions(dwOptions | FOS_PICKFOLDERS | FOS_FORCEFILESYSTEM);
						hr = pFileDialog->Show(nullptr);
						if (SUCCEEDED(hr))
						{
							IShellItem* pItem;
							hr = pFileDialog->GetResult(&pItem);
							if (SUCCEEDED(hr))
							{
								PWSTR pszFilePath = nullptr;
								hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);
								if (SUCCEEDED(hr))
								{
									std::wcout << L"Selected folder: " << pszFilePath << std::endl;
									std::wstring outputPath = pszFilePath;

									RipCPKToDisk(outputPath);


									CoTaskMemFree(pszFilePath);
								}

								pItem->Release();
							}
						}
						pFileDialog->Release();
					}
					CoUninitialize();
				}



			}

			if (!isCPKLoaded) {
				LoadCPK();
				isCPKLoaded = true;
			}

			DrawFolder(cpkBase, std::string(searchBuf), true);

		}

	}

};