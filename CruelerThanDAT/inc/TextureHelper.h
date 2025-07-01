#pragma once
#define GL_COMPRESSED_RGBA_S3TC_DXT1_EXT 0x83F1
#define GL_COMPRESSED_RGBA_S3TC_DXT3_EXT 0x83F2
#define GL_COMPRESSED_RGBA_S3TC_DXT5_EXT 0x83F3
#include "BinaryHandler.h"

enum WTB_EXDATA {
	EXDATA_XBOX,
	EXDATA_NONE
};

class TextureHelper {
public:
	static void LoadData(BinaryReader& wta, BinaryReader& wtp, std::unordered_map<unsigned int, unsigned int>& textureMap);
};