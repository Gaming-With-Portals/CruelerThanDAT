#pragma once
#include "pch.hpp"

#include <d3dx9.h>

static bool SHOULD_UPDATE = false;
static int BUILD_NUMBER = 2;
extern LPDIRECT3DDEVICE9 g_pd3dDevice;
extern LPDIRECT3DVERTEXSHADER9 pSolidShaderVTX;
extern LPDIRECT3DPIXELSHADER9 pSolidShaderPX;
extern std::unordered_map<unsigned int, unsigned int> textureMap;
extern std::unordered_map<unsigned int, std::vector<char>> rawTextureInfo;
extern float globalProgress;