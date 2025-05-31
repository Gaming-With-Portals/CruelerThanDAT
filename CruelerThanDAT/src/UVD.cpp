#include "pch.hpp"
#include "UVD.h"

#include "BinaryHandler.h"

void UvdEntry::Read(BinaryReader& br) {
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
