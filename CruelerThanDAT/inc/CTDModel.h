#pragma once
#include "pch.hpp"

struct CUSTOMVERTEX {
    float x, y, z;
    DWORD color;
    float u, v;
    float nx, ny, nz;
    float tx, ty, tz, tw;
    float u2, v2 = 0;
};

#define D3DFVF_CUSTOMVERTEX (D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1)