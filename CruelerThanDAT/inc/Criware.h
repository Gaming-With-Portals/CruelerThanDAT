#pragma once
#include "BinaryHandler.h"



struct CriRow {
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

struct CriField {
	uint8_t flag;
	std::string name;
	uint32_t default_value_offset;
	CriRow row;
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

class CriTOC {
public:
	size_t size = 0;
	uint8_t encoding = 0;
	std::vector<CriField> fields;
	std::unordered_map<uint32_t, std::unordered_map<std::string, CriRow>> row_map;
	uint16_t field_count = 0;
	uint16_t row_count = 0;


	CriField* GetField(std::string id) {
		for (CriField& field : fields) {
			if (field.name == id) {
				return &field;
			}
		}
		return nullptr;
	}

	void Load(BinaryReader& br) {
		br.SetEndianess(true);
		// i love how this is so different than critoc in CPKManager, thank god i can use this bt instead of the stupid criware extractor thing
		if (br.ReadString(4) == "@UTF") {
			size = br.ReadUINT32();
			br.ReadINT8();
			encoding = br.ReadINT8();
			uint16_t rows_offset = br.ReadUINT16() + 8;
			uint32_t string_pool_offset = br.ReadUINT32() + 8;
			uint32_t data_pool_offset = br.ReadUINT32() + 8;
			uint32_t table_name = br.ReadUINT32();
			field_count = br.ReadUINT16();
			uint16_t row_size = br.ReadUINT16();
			row_count = br.ReadUINT32();

			for (uint16_t i = 0; i < field_count; i++) {
				CriField field;
				field.flag = br.ReadINT8();

				uint32_t name_of = br.ReadUINT32();
				size_t tmp = br.Tell();
				br.Seek(string_pool_offset + name_of);
				field.name = br.ReadNullTerminatedString();

				br.Seek(tmp);
				if (field.flag & 32) { // Default Value
					field.default_value_offset = br.ReadUINT32();
				}

				fields.push_back(field);
			}

			for (uint32_t x = 0; x < row_count; x++) {
				br.Seek(rows_offset + (x * row_size));
				for (uint32_t y = 0; y < field_count; y++) {
					CriRow row;
					CriField& field = fields[y];

					uint8_t flags = field.flag;
					uint8_t storage_flag = flags & STORAGE_MASK;
					row.type = flags & TYPE_MASK;
					row.position = br.Tell();

					if (flags & 0x20) {
						continue;
					}

					if (flags == 0) {
						continue;
					}

					switch (row.type) {
					case 0: // fallthrough
					case 1:
						row.uint8 = br.ReadINT8();
						break;
					case 2: // fallthrough
					case 3:
						row.uint16 = br.ReadUINT16();
						break;
					case 4: // fallthrough
					case 5:
						row.uint32 = br.ReadUINT32();
						break;
					case 6: // fallthrough
					case 7:
						row.uint64 = br.ReadUINT64();
						break;
					case 8:
						row.ufloat = br.ReadFloat();
						break;
					case 0xA: {
						long position = br.ReadINT32() + string_pool_offset;
						long original = br.Tell();
						br.Seek(position);
						row.str = br.ReadNullTerminatedString();
						br.Seek(original);
						break;
					}
					case 0xB: {
						long position = br.ReadINT32() + data_pool_offset;
						long data_size = br.ReadINT32();
						long original = br.Tell();
						br.Seek(position);
						row.position = position;
						row.data = br.ReadBytes(data_size);
						br.Seek(original);
						break;
					}
					default:
						row.position = 0;
						std::cout << "ERROR!" << std::endl;
					}

					row_map[x][field.name] = row;
				}
			}
		}
		else {
			return; // ts is NOT a critoc table please die thank you
		}
		return;
	}
};