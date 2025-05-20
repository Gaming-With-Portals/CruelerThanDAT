#pragma once
#include "pch.hpp"
#include "CommonTypes.h"
namespace MGRUI {
	enum UIDData1Type {
		MCD,
		UVD,
		UNK
	};
}


struct UIDData1MCD {

};

struct UIDData1UVD {
	float width;
	float height;
	uint32_t texID;
	uint32_t uvdID;
	bool Read(BinaryReader& br) {
		width = br.ReadFloat();
		height = br.ReadFloat();
		texID = br.ReadUINT32();
		uvdID = br.ReadUINT32();

		if (floor(width) == ceil(width) && floor(height) == ceil(height)) { // Weird method but if it works I can't blame em
			return true;
		}
		else {
			return false;
		}


	}
};

// Thank you savior RaiderB
struct UIDData1 {
	MGRUI::UIDData1Type dataType = MGRUI::UNK;
	UIDData1MCD mcdData;
	UIDData1UVD uvdData;

	std::vector<char> data;
	void Read(BinaryReader& br, int size) {
		size_t startPos = br.Tell();
		data = br.ReadBytes(size);
		br.Seek(startPos);
		// Type Checking
		if (size > 36) {
			dataType = MGRUI::UNK;

			if (uvdData.Read(br)) {
				dataType = MGRUI::UVD;
			}


		}



	}

	void Write(BinaryWriter* bw) {
		bw->WriteBytes(data);
	}

};

struct UIDData2 {
	std::vector<char> data;
	void Read(BinaryReader& br, int size) {
		data = br.ReadBytes(size);
	}

	void Write(BinaryWriter* bw) {
		bw->WriteBytes(data);
	}

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

	void Read(BinaryReader& br) {
		beginOffset = br.ReadUINT32();
		size1 = br.ReadUINT32();
		propertyIndex = br.ReadUINT32();
		minValue = br.ReadFloat();
		maxValue = br.ReadFloat();
		startTime = br.ReadFloat();
		endTime = br.ReadFloat();
		u_3 = br.ReadUINT32();
	}
	void Write(BinaryWriter* bw) {
		bw->WriteUINT32(beginOffset);
		bw->WriteUINT32(size1);
		bw->WriteUINT32(propertyIndex);
		bw->WriteFloat(minValue);
		bw->WriteFloat(maxValue);
		bw->WriteFloat(startTime);
		bw->WriteFloat(endTime);
		bw->WriteUINT32(u_3);
	}
	

};

struct Data3Entry {
	float time;
	uint16_t value;
	uint16_t u_0;

	void Read(BinaryReader& br) {
		time = br.ReadFloat();
		value = br.ReadUINT16();
		u_0 = br.ReadUINT16();
	}

	void Write(BinaryWriter* bw) {
		bw->WriteFloat(time);
		bw->WriteUINT16(value);
		bw->WriteUINT16(u_0);
	}


};

struct UIDData3 {
	std::vector<char> data;
	Data3Header d3Header;
	void Read(BinaryReader& br) {
		br.Skip(sizeof(uint32_t)); // TODO: firstOffset
		// TODO: Will this be used eventually? uint32_t entriesCount = (firstOffset - br.Tell()) / 32;
		d3Header.Read(br);
	}

	void Write(BinaryWriter* bw) {
		(void)bw;
	}

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

	void Read(BinaryReader& br) {
		size1 = br.ReadINT32();
		size2 = br.ReadINT32();
		size3 = br.ReadINT32();
		u0 = br.ReadINT32();
		offset1 = br.ReadINT32();
		offset2 = br.ReadINT32();
		offset3 = br.ReadINT32();
		u1 = br.ReadINT32();
		frameLength = br.ReadFloat();
		width = br.ReadFloat();
		height = br.ReadFloat();
		u2 = br.ReadINT32();
	}

	void Write(BinaryWriter* bw) {
		bw->WriteINT32(size1);
		bw->WriteINT32(size2);
		bw->WriteINT32(size3);
		bw->WriteINT32(u0);
		bw->WriteINT32(offset1);
		bw->WriteINT32(offset2);
		bw->WriteINT32(offset3);
		bw->WriteINT32(u1);
		bw->WriteFloat(frameLength);
		bw->WriteFloat(width);
		bw->WriteFloat(height);
		bw->WriteINT32(u2);
	}

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
	int bool_82;
	int uint_83;
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

	std::vector<int> getOffsets() {
		std::vector<int> vec;
		if (data1Offset != 0) {
			vec.push_back(data1Offset);
		}
		if (data2Offset != 1) {
			vec.push_back(data2Offset);
		}
		if (data3Offset != 2) {
			vec.push_back(data3Offset);
		}

		return vec;
	}


	void Read(BinaryReader& br) {
		position.x = br.ReadFloat();
		position.y = br.ReadFloat();
		position.z = br.ReadFloat();
		rotation.x = br.ReadFloat();
		rotation.y = br.ReadFloat();
		rotation.z = br.ReadFloat();
		scale.x = br.ReadFloat();
		scale.y = br.ReadFloat();
		scale.z = br.ReadFloat();
		rgb.r = br.ReadFloat();
		rgb.g = br.ReadFloat();
		rgb.b = br.ReadFloat();
		rgb.a = br.ReadFloat();
		f0 = br.ReadFloat();
		f1 = br.ReadFloat();
		f2 = br.ReadFloat();
		f3 = br.ReadFloat();
		entry2_index = br.ReadINT32();
		entry2_id = br.ReadINT32();
		uint_2 = br.ReadINT32();
		uint_3 = br.ReadINT32();
		float_4 = br.ReadFloat();
		uint_5 = br.ReadINT32();
		float_6 = br.ReadFloat();
		int_7 = br.ReadINT32();
		bool_8 = br.ReadINT32();
		bool_9 = br.ReadINT32();
		bool_10 = br.ReadINT32();
		bool_11 = br.ReadINT32();
		bool_12 = br.ReadINT32();
		bool_13 = br.ReadINT32();
		bool_14 = br.ReadINT32();
		bool_15 = br.ReadINT32();
		null_16 = br.ReadINT32();
		bool_17 = br.ReadINT32();
		bool_18 = br.ReadINT32();
		float_19 = br.ReadFloat();
		uint_20 = br.ReadINT32();
		float_21 = br.ReadFloat();
		bool_22 = br.ReadINT32();
		float_23 = br.ReadFloat();
		uint_24 = br.ReadINT32();
		float_25 = br.ReadFloat();
		bool_26 = br.ReadINT32();
		float_27 = br.ReadFloat();
		bool_28 = br.ReadINT32();
		float_29 = br.ReadFloat();
		bool_30 = br.ReadINT32();
		float_31 = br.ReadFloat();
		uint_32 = br.ReadINT32();
		float_33 = br.ReadFloat();
		bool_34 = br.ReadINT32();
		float_35 = br.ReadFloat();
		uint_36 = br.ReadINT32();
		float_37 = br.ReadFloat();
		bool_38 = br.ReadINT32();
		float_39 = br.ReadFloat();
		uint_40 = br.ReadINT32();
		float_41 = br.ReadFloat();
		null_42 = br.ReadINT32();
		float_43 = br.ReadFloat();
		uint_44 = br.ReadINT32();
		float_45 = br.ReadFloat();
		null_46 = br.ReadINT32();
		float_47 = br.ReadFloat();
		uint_48 = br.ReadINT32();
		float_49 = br.ReadFloat();
		bool_50 = br.ReadINT32();
		null_51 = br.ReadINT32();
		null_52 = br.ReadINT32();
		null_53 = br.ReadINT32();
		null_54 = br.ReadINT32();
		null_55 = br.ReadINT32();
		null_56 = br.ReadINT32();
		null_57 = br.ReadINT32();
		null_58 = br.ReadINT32();
		null_59 = br.ReadINT32();
		null_60 = br.ReadINT32();
		null_61 = br.ReadINT32();
		null_62 = br.ReadINT32();
		float_63 = br.ReadFloat();
		uint_64 = br.ReadINT32();
		float_65 = br.ReadFloat();
		bool_66 = br.ReadINT32();
		null_67 = br.ReadINT32();
		null_68 = br.ReadINT32();
		null_69 = br.ReadINT32();
		null_70 = br.ReadINT32();
		float_71 = br.ReadFloat();
		uint_72 = br.ReadINT32();
		float_73 = br.ReadFloat();
		bool_74 = br.ReadINT32();
		float_75 = br.ReadFloat();
		uint_76 = br.ReadINT32();
		float_77 = br.ReadFloat();
		bool_78 = br.ReadINT32();
		float_79 = br.ReadFloat();
		uint_80 = br.ReadINT32();
		float_81 = br.ReadFloat();
		bool_82 = br.ReadINT32();
		uint_83 = br.ReadINT32();
		uint_84 = br.ReadINT32();
		uint_85 = br.ReadINT32();
		uint_86 = br.ReadINT32();
		data1Offset = br.ReadINT32();
		data2Offset = br.ReadINT32();
		data3Offset = br.ReadINT32();
		u4 = br.ReadINT32();

	}

	void Write(BinaryWriter* bw) {
		bw->WriteFloat(position.x);
		bw->WriteFloat(position.y);
		bw->WriteFloat(position.z);
		bw->WriteFloat(rotation.x);
		bw->WriteFloat(rotation.y);
		bw->WriteFloat(rotation.z);
		bw->WriteFloat(scale.x);
		bw->WriteFloat(scale.y);
		bw->WriteFloat(scale.z);
		bw->WriteFloat(rgb.r);
		bw->WriteFloat(rgb.g);
		bw->WriteFloat(rgb.b);
		bw->WriteFloat(rgb.a);
		bw->WriteFloat(f0);
		bw->WriteFloat(f1);
		bw->WriteFloat(f2);
		bw->WriteFloat(f3);
		bw->WriteINT32(entry2_index);
		bw->WriteINT32(entry2_id);
		bw->WriteINT32(uint_2);
		bw->WriteINT32(uint_3);
		bw->WriteFloat(float_4);
		bw->WriteINT32(uint_5);
		bw->WriteFloat(float_6);
		bw->WriteINT32(int_7);
		bw->WriteINT32(bool_8);
		bw->WriteINT32(bool_9);
		bw->WriteINT32(bool_10);
		bw->WriteINT32(bool_11);
		bw->WriteINT32(bool_12);
		bw->WriteINT32(bool_13);
		bw->WriteINT32(bool_14);
		bw->WriteINT32(bool_15);
		bw->WriteINT32(null_16);
		bw->WriteINT32(bool_17);
		bw->WriteINT32(bool_18);
		bw->WriteFloat(float_19);
		bw->WriteINT32(uint_20);
		bw->WriteFloat(float_21);
		bw->WriteINT32(bool_22);
		bw->WriteFloat(float_23);
		bw->WriteINT32(uint_24);
		bw->WriteFloat(float_25);
		bw->WriteINT32(bool_26);
		bw->WriteFloat(float_27);
		bw->WriteINT32(bool_28);
		bw->WriteFloat(float_29);
		bw->WriteINT32(bool_30);
		bw->WriteFloat(float_31);
		bw->WriteINT32(uint_32);
		bw->WriteFloat(float_33);
		bw->WriteINT32(bool_34);
		bw->WriteFloat(float_35);
		bw->WriteINT32(uint_36);
		bw->WriteFloat(float_37);
		bw->WriteINT32(bool_38);
		bw->WriteFloat(float_39);
		bw->WriteINT32(uint_40);
		bw->WriteFloat(float_41);
		bw->WriteINT32(null_42);
		bw->WriteFloat(float_43);
		bw->WriteINT32(uint_44);
		bw->WriteFloat(float_45);
		bw->WriteINT32(null_46);
		bw->WriteFloat(float_47);
		bw->WriteINT32(uint_48);
		bw->WriteFloat(float_49);
		bw->WriteINT32(bool_50);
		bw->WriteINT32(null_51);
		bw->WriteINT32(null_52);
		bw->WriteINT32(null_53);
		bw->WriteINT32(null_54);
		bw->WriteINT32(null_55);
		bw->WriteINT32(null_56);
		bw->WriteINT32(null_57);
		bw->WriteINT32(null_58);
		bw->WriteINT32(null_59);
		bw->WriteINT32(null_60);
		bw->WriteINT32(null_61);
		bw->WriteINT32(null_62);
		bw->WriteFloat(float_63);
		bw->WriteINT32(uint_64);
		bw->WriteFloat(float_65);
		bw->WriteINT32(bool_66);
		bw->WriteINT32(null_67);
		bw->WriteINT32(null_68);
		bw->WriteINT32(null_69);
		bw->WriteINT32(null_70);
		bw->WriteFloat(float_71);
		bw->WriteINT32(uint_72);
		bw->WriteFloat(float_73);
		bw->WriteINT32(bool_74);
		bw->WriteFloat(float_75);
		bw->WriteINT32(uint_76);
		bw->WriteFloat(float_77);
		bw->WriteINT32(bool_78);
		bw->WriteFloat(float_79);
		bw->WriteINT32(uint_80);
		bw->WriteFloat(float_81);
		bw->WriteINT32(bool_82);
		bw->WriteINT32(uint_83);
		bw->WriteINT32(uint_84);
		bw->WriteINT32(uint_85);
		bw->WriteINT32(uint_86);
		bw->WriteINT32(data1Offset);
		bw->WriteINT32(data2Offset);
		bw->WriteINT32(data3Offset);
		bw->WriteINT32(u4);
	}

	void readAdditionalData(BinaryReader& reader, std::unordered_map<int, int> offsetSizes) {
		if (data1Offset != 0) {
			reader.Seek(data1Offset);
			data1.Read(reader, offsetSizes[data1Offset]);
		}
		if (data2Offset != 0) {
			reader.Seek(data2Offset);
			data2.Read(reader, offsetSizes[data2Offset]);
		}
		if (data3Offset != 0) {
			reader.Seek(data3Offset);
			data3.Read(reader); // NOTE: offsetSizes[data3Offset] went unused
		}
	}

	void writeAdditionalData(BinaryWriter* writer) {
		if (data1.data.size() > 0) {
			data1.Write(writer);
		}
		if (data2.data.size() > 0) {
			data2.Write(writer);
		}
		if (data3.data.size() > 0) {
			data3.Write(writer);
		}
	}


};

struct UIDEntry2 {
	int id;
	int entry1_index;

	void Read(BinaryReader& br) {
		id = br.ReadINT32();
		entry1_index = br.ReadINT32();
	}

	void Write(BinaryWriter* bw) {
		bw->WriteINT32(id);
		bw->WriteINT32(entry1_index);
	}

};

struct UIDEntry3 {
	int entry2_id;
	float f0;
	float f1;
	int u1, u2, u3, u4, u5, u6, u7, u8, u9, u10, u11, u12, u13, u14; // lore

	void Read(BinaryReader& br) {
		entry2_id = br.ReadINT32();
		f0 = br.ReadFloat();
		f1 = br.ReadFloat();
		u1 = br.ReadUINT32();
		u2 = br.ReadUINT32();
		u3 = br.ReadUINT32();
		u4 = br.ReadUINT32();
		u5 = br.ReadUINT32();
		u6 = br.ReadUINT32();
		u7 = br.ReadUINT32();
		u8 = br.ReadUINT32();
		u9 = br.ReadUINT32();
		u10 = br.ReadUINT32();
		u11 = br.ReadUINT32();
		u12 = br.ReadUINT32();
		u13 = br.ReadFloat(); // TODO: What???
	}

	void Write(BinaryWriter* bw) {
		bw->WriteINT32(entry2_id);
		bw->WriteFloat(f0);
		bw->WriteFloat(f1);
		bw->WriteUINT32(u1);
		bw->WriteUINT32(u2);
		bw->WriteUINT32(u3);
		bw->WriteUINT32(u4);
		bw->WriteUINT32(u5);
		bw->WriteUINT32(u6);
		bw->WriteUINT32(u7);
		bw->WriteUINT32(u8);
		bw->WriteUINT32(u9);
		bw->WriteUINT32(u10);
		bw->WriteUINT32(u11);
		bw->WriteUINT32(u12);
		bw->WriteUINT32(u13);
	}

};