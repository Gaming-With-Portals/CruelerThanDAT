#pragma once
#include "BinaryHandler.h"

enum WTB_EXDATA {
	EXDATA_XBOX,
	EXDATA_XT1,
	EXDATA_NONE
};

class TextureHelper {
public:
	static void LoadData(BinaryReader& wta, BinaryReader& wtp, std::unordered_map<unsigned int, unsigned int>& textureMap);
};