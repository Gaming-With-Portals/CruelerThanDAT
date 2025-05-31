#pragma once
#include "pch.hpp"
#include "CommonTypes.h"


struct UvdTexture {
	std::string name;
	uint32_t id;
};

struct UvdEntry {
	std::string name;
	uint32_t ID;
	uint32_t textureID;
	float x;
	float y;
	float width;
	float height;
	float widthInverse;
	float heightInverse;

	void Read(BinaryReader& br) {
		name = br.ReadString(64);
		ID = br.ReadUINT32();
		textureID = br.ReadUINT32();
		x = br.ReadFloat();
		y = br.ReadFloat();
		width = br.ReadFloat();
		height = br.ReadFloat();
		widthInverse = br.ReadFloat();
		heightInverse = br.ReadFloat();
	}

};


