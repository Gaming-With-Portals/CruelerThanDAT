#include "pch.hpp"
#include "UID.h"

#include <imgui.h>
#include "CommonTypes.h"
#include "BinaryHandler.h"

std::string MGRUI::Entry1TypeFriendlyFormatted[10] = {
	"UIDrawLocator",
	"Image",
	"UIDraw9Grid",
	"MCD Text",
	"UIDrawString",
	"UIDrawMask",
	"UNKNOWN_6",
	"UNKNOWN_7",
	"Draw 3 Grid",
	"UIDrawHit",
};

void UIDData1Image::Read(BinaryReader& br) {
	spriteWidth = br.ReadFloat();
	spriteHeight = br.ReadFloat();
	u_0 = br.ReadUINT32();
	u_1 = br.ReadUINT32();
	u_2 = br.ReadUINT32();
	u_3 = br.ReadUINT32();
	u_4 = br.ReadUINT32();
	textureID = br.ReadUINT32();
	uvdID = br.ReadUINT32();
	bools = br.ReadUINT32();
	unused_2 = br.ReadUINT32();
	f_0 = br.ReadFloat();
	f_1 = br.ReadFloat();
	f_2 = br.ReadFloat();
	f_3 = br.ReadFloat();
}

void UIDData1Image::Write(BinaryWriter* bw) {
	bw->WriteFloat(spriteWidth);
	bw->WriteFloat(spriteHeight);
	bw->WriteUINT32(u_0);
	bw->WriteUINT32(u_1);
	bw->WriteUINT32(u_2);
	bw->WriteUINT32(u_3);
	bw->WriteUINT32(u_4);
	bw->WriteUINT32(textureID);
	bw->WriteUINT32(uvdID);
	bw->WriteUINT32(bools);
	bw->WriteUINT32(unused_2);
	bw->WriteFloat(f_0);
	bw->WriteFloat(f_1);
	bw->WriteFloat(f_2);
	bw->WriteFloat(f_3);

}

void UIDData1Image::Render() {
	ImGui::InputFloat("Sprite Width", &spriteWidth);
	ImGui::InputFloat("Sprite Height", &spriteHeight);
	ImGui::InputScalar("Texture ID", ImGuiDataType_U32, &textureID);
	ImGui::InputScalar("UVD ID", ImGuiDataType_U32, &uvdID);
	ImGui::InputFloat("f_0", &f_0);
	ImGui::InputFloat("f_1", &f_1);
	ImGui::InputFloat("f_2", &f_2);
	ImGui::InputFloat("f_3", &f_3);
}

void UIDData1Message::Read(BinaryReader& br) {
	f_0 = br.ReadFloat();
	f_1 = br.ReadFloat();
	u_0 = br.ReadUINT32();
	u_1 = br.ReadUINT32();
	mcdFile = br.ReadString(16).c_str();
	mcdHash = br.ReadUINT32();
	u_2 = br.ReadUINT32();
	f_2 = br.ReadFloat();
	f_3 = br.ReadFloat();
	f_4 = br.ReadFloat();
	bools = br.ReadUINT32();
	u_3 = br.ReadUINT32();
	u_4 = br.ReadUINT32();
	u_5 = br.ReadUINT32();
	f_5 = br.ReadFloat();
	f_6 = br.ReadFloat();
	f_7 = br.ReadFloat();
	f_8 = br.ReadFloat();
	f_9 = br.ReadFloat();
	f_10 = br.ReadFloat();
	f_11 = br.ReadFloat();
	f_12 = br.ReadFloat();
	f_13 = br.ReadFloat();
	u_6 = br.ReadUINT16();
	u_7 = br.ReadUINT16();
	f_14 = br.ReadFloat();
	f_15 = br.ReadFloat();
}

void UIDData1Message::Write(BinaryWriter* bw) {
	bw->WriteFloat(f_0);
	bw->WriteFloat(f_1);
	bw->WriteUINT32(u_0);
	bw->WriteUINT32(u_1);
	bw->WriteString(mcdFile);
	bw->WriteUINT32(mcdHash);
	bw->WriteUINT32(u_2);
	bw->WriteUINT32(f_2);
	bw->WriteUINT32(f_3);
	bw->WriteUINT32(f_4);
	bw->WriteUINT32(bools);
	bw->WriteUINT32(u_3);
	bw->WriteUINT32(u_4);
	bw->WriteUINT32(u_5);
	bw->WriteFloat(f_5);
	bw->WriteFloat(f_6);
	bw->WriteFloat(f_7);
	bw->WriteFloat(f_8);
	bw->WriteFloat(f_9);
	bw->WriteFloat(f_10);
	bw->WriteFloat(f_11);
	bw->WriteFloat(f_12);
	bw->WriteFloat(f_13);
	bw->WriteUINT16(u_6);
	bw->WriteUINT16(u_7);
	bw->WriteFloat(f_14);
	bw->WriteFloat(f_15);

}	

void UIDData1Message::Render() {

}

UIDData1::UIDData1() = default;

void UIDData1::Read(BinaryReader& br, int size, Entry1Type type) {
	dataType = type;

	size_t startPos = br.Tell();
	data = br.ReadBytes(size);
	br.Seek(startPos);

	if (type == Entry1Type::UIDrawImage) {
		dataStructure.img.Read(br);
	}
	else if (type == Entry1Type::UIDrawMessage) {
		dataStructure.mcd.Read(br);
	}

}

void UIDData1::Write(BinaryWriter* bw) {
	bw->WriteBytes(data);
}

void UIDData2::Read(BinaryReader& br, int size) {
	data = br.ReadBytes(size);
}

void UIDData2::Write(BinaryWriter* bw) {
	bw->WriteBytes(data);
}

void Data3Header::Read(BinaryReader& br) {
	beginOffset = br.ReadUINT32();
	size1 = br.ReadUINT32();
	propertyIndex = br.ReadUINT32();
	minValue = br.ReadFloat();
	maxValue = br.ReadFloat();
	startTime = br.ReadFloat();
	endTime = br.ReadFloat();
	u_3 = br.ReadUINT32();
}

void Data3Header::Write(BinaryWriter* bw) {
	bw->WriteUINT32(beginOffset);
	bw->WriteUINT32(size1);
	bw->WriteUINT32(propertyIndex);
	bw->WriteFloat(minValue);
	bw->WriteFloat(maxValue);
	bw->WriteFloat(startTime);
	bw->WriteFloat(endTime);
	bw->WriteUINT32(u_3);
}

void Data3Entry::Read(BinaryReader& br) {
	time = br.ReadFloat();
	value = br.ReadUINT16();
	u_0 = br.ReadUINT16();
}

void Data3Entry::Write(BinaryWriter* bw) {
	bw->WriteFloat(time);
	bw->WriteUINT16(value);
	bw->WriteUINT16(u_0);
}

void UIDData3::Read(BinaryReader& br) {
	br.Skip(sizeof(uint32_t)); // TODO: firstOffset
	// TODO: Will this be used eventually? uint32_t entriesCount = (firstOffset - br.Tell()) / 32;
	d3Header.Read(br);
}

void UIDData3::Write(BinaryWriter* bw) {
	(void)bw;
}

void UIDHeader::Read(BinaryReader& br) {
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

void UIDHeader::Write(BinaryWriter* bw) {
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

std::vector<int> UIDEntry1::getOffsets() {
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

void UIDEntry1::Read(BinaryReader& br) {
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
	uint_82 = br.ReadUINT32();
	data1Flag = static_cast<Entry1Type>(br.ReadINT32());
	uint_84 = br.ReadINT32();
	uint_85 = br.ReadINT32();
	uint_86 = br.ReadINT32();
	data1Offset = br.ReadINT32();
	data2Offset = br.ReadINT32();
	data3Offset = br.ReadINT32();
	u4 = br.ReadINT32();
}

void UIDEntry1::Write(BinaryWriter* bw) {
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
	bw->WriteINT32(uint_82);
	bw->WriteINT32(data1Flag);
	bw->WriteINT32(uint_84);
	bw->WriteINT32(uint_85);
	bw->WriteINT32(uint_86);
	bw->WriteINT32(data1Offset);
	bw->WriteINT32(data2Offset);
	bw->WriteINT32(data3Offset);
	bw->WriteINT32(u4);
}

void UIDEntry1::readAdditionalData(BinaryReader& reader, std::unordered_map<int, int> offsetSizes) {
	if (data1Offset != 0) {
		reader.Seek(data1Offset);
		data1.Read(reader, offsetSizes[data1Offset], data1Flag);
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

void UIDEntry1::writeAdditionalData(BinaryWriter* writer) {
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

void UIDEntry2::Read(BinaryReader& br) {
	id = br.ReadINT32();
	entry1_index = br.ReadINT32();
}

void UIDEntry2::Write(BinaryWriter* bw) {
	bw->WriteINT32(id);
	bw->WriteINT32(entry1_index);
}

void UIDEntry3::Read(BinaryReader& br) {
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

void UIDEntry3::Write(BinaryWriter* bw) {
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
