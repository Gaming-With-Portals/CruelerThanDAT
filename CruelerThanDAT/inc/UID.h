#pragma once
#include "pch.hpp"

#include "CommonTypes.h"
#include "BinaryHandler.h"

namespace MGRUI {
	enum UIDData1Type {
		MCD,
		UVD,
		UNK
	};

	extern std::string Entry1TypeFriendlyFormatted[10];
}

enum Entry1Type {
	UIDrawLocator = 0,
	UIDrawImage = 1,
	UIDraw9Grid = 2,
	UIDrawMessage = 3,
	UIDrawString = 4,
	UIDrawMask = 5,
	UIDraw3Grid = 8,
	UIDrawHit = 9
};

struct UIDData1Image {
	float spriteWidth;
	float spriteHeight;
	uint32_t u_0;
	uint32_t u_1;
	uint32_t u_2;
	uint32_t u_3;
	uint32_t u_4;
	uint32_t textureID;
	uint32_t uvdID;
	uint32_t bools;
	uint32_t unused_2;
	float f_0;
	float f_1;
	float f_2;
	float f_3;

	void Read(BinaryReader& br);

	void Write(BinaryWriter* bw);

	void Render();
};

struct UIDData1Message {
	float f_0;
	float f_1;
	uint32_t u_0;
	uint32_t u_1;
	const char* mcdFile;
	uint32_t mcdHash;
	uint32_t u_2;
	float f_2;
	float f_3;
	float f_4;
	uint32_t bools;
	uint32_t u_3;
	uint32_t u_4;
	uint32_t u_5;
	float f_5;
	float f_6;
	float f_7;
	float f_8;
	float f_9;
	float f_10;
	float f_11;
	float f_12;
	float f_13;
	uint16_t u_6;
	uint16_t u_7;
	float f_14;
	float f_15;

	void Read(BinaryReader& br);

	void Write(BinaryWriter* bw);

	void Render();
};

union UIDData1Structures {
	UIDData1Image img;
	UIDData1Message mcd;
};

// Thank you savior RaiderB
struct UIDData1 {
	Entry1Type dataType;
	UIDData1Structures dataStructure;
	std::vector<char> data;

	UIDData1();

	void Read(BinaryReader& br, int size, Entry1Type type);

	void Write(BinaryWriter* bw);
};

struct UIDData2 {
	std::vector<char> data;
	void Read(BinaryReader& br, int size);

	void Write(BinaryWriter* bw);
};

struct Data3Header {
	uint32_t beginOffset;
	uint32_t size1;
	uint32_t propertyIndex;
	float minValue;
	float maxValue;
	float startTime;
	float endTime;
	uint32_t u_3;

	void Read(BinaryReader& br);

	void Write(BinaryWriter* bw);
};

struct Data3Entry {
	float time;
	uint16_t value;
	uint16_t u_0;

	void Read(BinaryReader& br);

	void Write(BinaryWriter* bw);
};

struct UIDData3 {
	std::vector<char> data;
	Data3Header d3Header;

	void Read(BinaryReader& br);

	void Write(BinaryWriter* bw);
};




struct UIDHeader {
	int size1;
	int size2;
	int size3;
	int u0;
	int offset1;
	int offset2;
	int offset3;
	int u1;
	float frameLength;
	float width;
	float height;
	int u2;

	void Read(BinaryReader& br);

	void Write(BinaryWriter* bw);
};

struct UIDEntry1 {
	MGRVector position;
	MGRVector rotation;
	MGRVector scale;
	MGRColor rgb;
	float f0;
	float f1; 
	float f2; 
	float f3;
	int entry2_index;
	int entry2_id;
	int uint_2;
	int uint_3;
	float float_4;
	int uint_5;
	float float_6;
	int int_7;
	int bool_8;
	int bool_9;
	int bool_10;
	int bool_11;
	int bool_12;
	int bool_13;
	int bool_14;
	int bool_15;
	int null_16;
	int bool_17;
	int bool_18;
	float float_19;
	int uint_20;
	float float_21;
	int bool_22;
	float float_23;
	int uint_24;
	float float_25;
	int bool_26;
	float float_27;
	int bool_28;
	float float_29;
	int bool_30;
	float float_31;
	int uint_32;
	float float_33;
	int bool_34;
	float float_35;
	int uint_36;
	float float_37;
	int bool_38;
	float float_39;
	int uint_40;
	float float_41;
	int null_42;
	float float_43;
	int uint_44;
	float float_45;
	int null_46;
	float float_47;
	int uint_48;
	float float_49;
	int bool_50;
	int null_51;
	int null_52;
	int null_53;
	int null_54;
	int null_55;
	int null_56;
	int null_57;
	int null_58;
	int null_59;
	int null_60;
	int null_61;
	int null_62;
	float float_63;
	int uint_64;
	float float_65;
	int bool_66;
	int null_67;
	int null_68;
	int null_69;
	int null_70;
	float float_71;
	int uint_72;
	float float_73;
	int bool_74;
	float float_75;
	int uint_76;
	float float_77;
	int bool_78;
	float float_79;
	int uint_80;
	float float_81;
	int uint_82;
	Entry1Type data1Flag;
	int uint_84;
	int uint_85;
	int uint_86;
	int data1Offset;
	int data2Offset;
	int data3Offset;
	int u4;

	UIDData1 data1;
	UIDData2 data2;
	UIDData3 data3;

	std::vector<int> getOffsets();


	void Read(BinaryReader& br);

	void Write(BinaryWriter* bw);

	void readAdditionalData(BinaryReader& reader, std::unordered_map<int, int> offsetSizes);

	void writeAdditionalData(BinaryWriter* writer);
};

struct UIDEntry2 {
	int id;
	int entry1_index;

	void Read(BinaryReader& br);

	void Write(BinaryWriter* bw);
};

struct UIDEntry3 {
	int entry2_id;
	float f0;
	float f1;
	int u1, u2, u3, u4, u5, u6, u7, u8, u9, u10, u11, u12, u13, u14; // lore

	void Read(BinaryReader& br);

	void Write(BinaryWriter* bw);
};