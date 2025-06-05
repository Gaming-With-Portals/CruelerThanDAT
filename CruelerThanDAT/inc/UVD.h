#pragma once
#include "pch.hpp"

#include "BinaryHandler.h"

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

	void Read(BinaryReader& br);
};
