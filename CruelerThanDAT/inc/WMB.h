#pragma once
#include "pch.hpp"




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

    void Read(BinaryReader& br) {
        position.x = br.ReadFloat();
        position.y = br.ReadFloat();
        position.z = br.ReadFloat();
        uv.u = br.ReadUINT16();
        uv.v = br.ReadUINT16();
        normals = br.ReadUINT32();
        tangents = br.ReadUINT32();
    }

};

struct WMBVertex65847 {
    WMBVector position; // 3 floats
    WMBUV uv;
    uint32_t normals; // TODO: Deal with
    uint32_t tangents; // TODO: Ignore
    float  boneIndexes;
    float  boneWeights;

    void Read(BinaryReader& br) {
        position.x = br.ReadFloat();
        position.y = br.ReadFloat();
        position.z = br.ReadFloat();
        uv.u = br.ReadUINT16();
        uv.v = br.ReadUINT16();
        normals = br.ReadUINT32();
        tangents = br.ReadUINT32();
        boneIndexes = br.ReadFloat();
        boneWeights = br.ReadFloat();
    }

};

struct WMBVertex66311 {
    WMBVector position; // 3 floats
    WMBUV uv;
    uint32_t normals; // TODO: Deal with
    uint32_t tangents; // TODO: Ignore
    uint32_t color;
    WMBUV uv2;
    
    void Read(BinaryReader& br) {
        position.x = br.ReadFloat();
        position.y = br.ReadFloat();
        position.z = br.ReadFloat();
        uv.u = br.ReadUINT16();
        uv.v = br.ReadUINT16();
        normals = br.ReadUINT32();
        tangents = br.ReadUINT32();
        color = br.ReadUINT32();
        uv2.u = br.ReadUINT16();
        uv2.v = br.ReadUINT16();
    }

};

struct WMBVertex65799 {
    WMBVector position; // 3 floats
    WMBUV uv;
    uint32_t normals; // TODO: Deal with
    uint32_t tangents; // TODO: Ignore
    uint32_t color;

    void Read(BinaryReader& br) {
        position.x = br.ReadFloat();
        position.y = br.ReadFloat();
        position.z = br.ReadFloat();
        uv.u = br.ReadUINT16();
        uv.v = br.ReadUINT16();
        normals = br.ReadUINT32(); // TODO: Deal with
        tangents = br.ReadUINT32(); // TODO: Ignore
        color = br.ReadUINT32();
    }

};

struct WMBBatch {
    uint32_t vertexGroupIndex;
    int32_t vertexStart;
    int32_t indexStart;
    uint32_t numVertices;
    uint32_t numIndices;

    void Read(BinaryReader& br) {
        vertexGroupIndex = br.ReadUINT32();
        vertexStart = br.ReadINT32();
        indexStart = br.ReadINT32();
        numVertices = br.ReadUINT32();
        numIndices = br.ReadUINT32();
    }

};

struct WMBBatchData {
    uint32_t batchIndex;
    uint32_t meshIndex;
    uint16_t materialIndex;
    uint16_t boneSetsIndex;
    uint32_t u_a;

    void Read(BinaryReader& br) {
        batchIndex = br.ReadUINT32();
        meshIndex = br.ReadUINT32();
        materialIndex = br.ReadUINT16();
        boneSetsIndex = br.ReadUINT16();
        u_a = br.ReadUINT32();
    }

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

    void Read(BinaryReader& br) {
        offsetShaderName = br.ReadUINT32();
        offsetTextures = br.ReadUINT32();
        u_a = br.ReadUINT32();
        offsetParameters = br.ReadUINT32();
        u_b = br.ReadUINT16();
        numTextures = br.ReadUINT16();
        u_d = br.ReadUINT16();
        numParameters = br.ReadUINT16();
    }

};

struct WMBTexture {
    uint32_t flag;
    uint32_t id;

    void Read(BinaryReader& br) {
        flag = br.ReadUINT32();
        id = br.ReadUINT32();
    }

};

struct WMBBoundingBox {
    float a;
    float b;
    float c;
    float d;
    float e;
    float f;

    void Read(BinaryReader& br) {
        a = br.ReadFloat();
        b = br.ReadFloat();
        c = br.ReadFloat();
        d = br.ReadFloat();
        e = br.ReadFloat();
        f = br.ReadFloat();
    }

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

    void Read(BinaryReader& br) {
        offsetName = br.ReadUINT32();
        boundingBox.Read(br);
        offsetBatches = br.ReadUINT32();
        numBatches = br.ReadUINT32();
        offsetBatches1 = br.ReadUINT32();
        numBatches1 = br.ReadUINT32();
        offsetBatches2 = br.ReadUINT32();
        numBatches2 = br.ReadUINT32();
        offsetBatches3 = br.ReadUINT32();
        numBatches3 = br.ReadUINT32();
        offsetMaterials = br.ReadUINT32();
        numMaterials = br.ReadUINT32();
    }

};

struct WMBVertexGroup {
    uint32_t offsetVertexes;
    uint32_t offsetVertexesExData;
    uint32_t u_a;
    uint32_t u_b;
    uint32_t numVertexes;
    uint32_t offsetIndexes;
    uint32_t numIndexes;

    void Read(BinaryReader& br) {
        offsetVertexes = br.ReadUINT32();
        offsetVertexesExData = br.ReadUINT32();
        u_a = br.ReadUINT32();
        u_b = br.ReadUINT32();
        numVertexes = br.ReadUINT32();
        offsetIndexes = br.ReadUINT32();
        numIndexes = br.ReadUINT32();
    }

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

    void Read(BinaryReader& br) {
        
        br.ReadUINT32();
        u_a = br.ReadUINT32();
        vertexFormat = br.ReadUINT32();
        u_b = br.ReadUINT16();
        u_c = br.ReadINT16();
        pos1.x = br.ReadFloat();
        pos1.y = br.ReadFloat();
        pos1.z = br.ReadFloat();
        pos2.x = br.ReadFloat();
        pos2.y = br.ReadFloat();
        pos2.z = br.ReadFloat();
        offsetVertexGroups = br.ReadUINT32();
        numVertexGroups = br.ReadUINT32();
        offsetBatches = br.ReadUINT32();
        numBatches = br.ReadUINT32();
        offsetBatchDescription = br.ReadUINT32();
        offsetBones = br.ReadUINT32();
        numBones = br.ReadUINT32();
        offsetBoneIndexTranslateTable = br.ReadUINT32();
        sizeBoneIndexTranslateTable = br.ReadUINT32();
        offsetBoneSets = br.ReadUINT32();
        numBoneSets = br.ReadUINT32();
        offsetMaterials = br.ReadUINT32();
        numMaterials = br.ReadUINT32();
        offsetTextures = br.ReadUINT32();
        numTextures = br.ReadUINT32();
        offsetMeshes = br.ReadUINT32();
        numMeshes = br.ReadUINT32();

    }



};
#pragma pack(pop)