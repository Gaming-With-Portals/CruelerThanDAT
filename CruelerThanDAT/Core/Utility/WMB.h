#pragma once
#include <cstdint>


#pragma pack(push, 1)

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
};

struct WMBVertex65847 {
    WMBVector position; // 3 floats
    WMBUV uv;
    uint32_t normals; // TODO: Deal with
    uint32_t tangents; // TODO: Ignore
    float  boneIndexes;
    float  boneWeights;
};

struct WMBVertex66311 {
    WMBVector position; // 3 floats
    WMBUV uv;
    uint32_t normals; // TODO: Deal with
    uint32_t tangents; // TODO: Ignore
    char color[4];
    WMBUV uv2;
    
};

struct WMBBatch {
    uint32_t vertexGroupIndex;
    int32_t vertexStart;
    int32_t indexStart;
    uint32_t numVertices;
    uint32_t numIndices;
};

struct WMBBatchData {
    uint32_t batchIndex;
    uint32_t meshIndex;
    uint32_t materialIndex;
    uint16_t boneSetsIndex;
    uint32_t u_a;
};

struct WMBBone {
    int16_t unk1;
    int16_t unk2;
    int16_t parentIndex;
    int16_t u_b;
    WMBVector relativePosition;
    WMBVector position;
};

struct WMBMaterial {
    uint32_t offsetShaderName;
    uint32_t offsetTextures;
    uint32_t u_a;
    uint32_t offsetParameters;
    uint16_t numTextures;
    uint16_t u_c;
    uint16_t u_d;
    uint16_t numParameters;
};

struct WMBTexture {
    uint32_t flags;
    uint32_t id;
};

struct WMBBoundingBox {
    float a;
    float b;
    float c;
    float d;
    float e;
    float f;
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
};

struct WMBVertexGroup {
    uint32_t offsetVertexes;
    uint32_t offsetVertexesExData;
    uint32_t u_a;
    uint32_t u_b;
    uint32_t numVertexes;
    uint32_t offsetIndexes;
    uint32_t numIndexes;
};

struct WMBHeader {
    char id[4];
    uint32_t u_a;
    uint32_t vertexFormat;
    uint16_t u_b;
    int16_t u_c;
    WMBVector pos1;
    WMBVector pos2;
    uint32_t offsetVertexGroups;
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

};
#pragma pack(pop)