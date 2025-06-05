#pragma once
#include "pch.hpp"

#include "BinaryHandler.h"

#pragma pack(push, 1)

struct CTDMaterial {
    std::string shader_name;
    std::map<unsigned int, unsigned int> texture_data;
};

struct WMBVector {
    float x;
    float y;
    float z;
};

struct WMBUV {
    uint16_t u;
    uint16_t v;
};

struct WMBVertexA {
    WMBVector position; // 3 floats
    WMBUV uv;
    uint32_t normals; // TODO: Deal with
    uint32_t tangents; // TODO: Ignore

    void Read(BinaryReader& br);
};

struct WMBVertex65847 {
    WMBVector position; // 3 floats
    WMBUV uv;
    uint32_t normals; // TODO: Deal with
    uint32_t tangents; // TODO: Ignore
    float  boneIndexes;
    float  boneWeights;

    void Read(BinaryReader& br);
};

struct WMBVertex66311 {
    WMBVector position; // 3 floats
    WMBUV uv;
    uint32_t normals; // TODO: Deal with
    uint32_t tangents; // TODO: Ignore
    uint32_t color;
    WMBUV uv2;
    
    void Read(BinaryReader& br);
};

struct WMBVertex65799 {
    WMBVector position; // 3 floats
    WMBUV uv;
    uint32_t normals; // TODO: Deal with
    uint32_t tangents; // TODO: Ignore
    uint32_t color;

    void Read(BinaryReader& br);
};

struct WMBBatch {
    uint32_t vertexGroupIndex;
    int32_t vertexStart;
    int32_t indexStart;
    uint32_t numVertices;
    uint32_t numIndices;

    void Read(BinaryReader& br);
};

struct WMBBatchData {
    uint32_t batchIndex;
    uint32_t meshIndex;
    uint16_t materialIndex;
    uint16_t boneSetsIndex;
    uint32_t u_a;

    void Read(BinaryReader& br);
};


struct WMBBone {
    int16_t unk1;
    int16_t unk2;
    int16_t parentIndex;
    int16_t u_b;
    WMBVector relativePosition;
    WMBVector position;
// TODO: Big Endian

};

struct WMBMaterial {
    uint32_t offsetShaderName;
    uint32_t offsetTextures;
    uint32_t u_a;
    uint32_t offsetParameters;
    uint16_t u_b;
    uint16_t numTextures;
    uint16_t u_d;
    uint16_t numParameters;

    void Read(BinaryReader& br);
};

struct WMBTexture {
    uint32_t flag;
    uint32_t id;

    void Read(BinaryReader& br);
};

struct WMBBoundingBox {
    float a;
    float b;
    float c;
    float d;
    float e;
    float f;

    void Read(BinaryReader& br);
};

struct WMBMesh {
    uint32_t offsetName;
    WMBBoundingBox boundingBox;
    uint32_t offsetBatches;
    uint32_t numBatches;
    uint32_t offsetBatches1;
    uint32_t numBatches1;
    uint32_t offsetBatches2;
    uint32_t numBatches2;
    uint32_t offsetBatches3;
    uint32_t numBatches3;
    uint32_t offsetMaterials;
    uint32_t numMaterials;

    void Read(BinaryReader& br);
};

struct WMBVertexGroup {
    uint32_t offsetVertexes;
    uint32_t offsetVertexesExData;
    uint32_t u_a;
    uint32_t u_b;
    uint32_t numVertexes;
    uint32_t offsetIndexes;
    uint32_t numIndexes;

    void Read(BinaryReader& br);
};

struct WMBHeader {
    char id[4];
    uint32_t u_a;
    uint32_t vertexFormat;
    uint16_t u_b;
    int16_t u_c;
    WMBVector pos1; // 24
    WMBVector pos2; // 36
    uint32_t offsetVertexGroups; // 48
    uint32_t numVertexGroups;
    uint32_t offsetBatches;
    uint32_t numBatches;
    uint32_t offsetBatchDescription;
    uint32_t offsetBones;
    uint32_t numBones;
    uint32_t offsetBoneIndexTranslateTable;
    uint32_t sizeBoneIndexTranslateTable;
    uint32_t offsetBoneSets;
    uint32_t numBoneSets;
    uint32_t offsetMaterials;
    uint32_t numMaterials;
    uint32_t offsetTextures;
    uint32_t numTextures;
    uint32_t offsetMeshes;
    uint32_t numMeshes;

    void Read(BinaryReader& br);
};
#pragma pack(pop)