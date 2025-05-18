#pragma once
#include <wtypes.h>
struct CUSTOMVERTEX {
    float x, y, z;      // position
    DWORD color;        // optional, or use normals if you want lighting
    float u, v;         // texture coords
};

#define D3DFVF_CUSTOMVERTEX (D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1)