#pragma once
#include <cstdint>
#include <BinaryHandler.h>

struct WMB4Vector {
    float x;
    float y;
    float z;
};


struct WMB4Batch {
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

struct WMB4BatchData {
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


struct WMB4Bone {
    int16_t unk1;
    int16_t unk2;
    int16_t parentIndex;
    int16_t u_b;
    WMB4Vector relativePosition;
    WMB4Vector position;
    // TODO: Big Endian

};

struct WMB4Material {
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

struct WMB4Texture {
    uint32_t flag;
    uint32_t id;

    void Read(BinaryReader& br) {
        flag = br.ReadUINT32();
        id = br.ReadUINT32();
    }

};

struct WMB4BoundingBox {
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

struct WMB4Mesh {
    uint32_t offsetName;
    WMB4BoundingBox boundingBox;
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

struct WMB4VertexGroup {
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

struct WMB4Header {
    char id[4];
    uint32_t u_a;
    uint32_t vertexFormat;
    uint16_t u_b;
    int16_t u_c;
    WMB4Vector pos1; // 24
    WMB4Vector pos2; // 36
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
    uint32_t cutdataOffset;

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
        cutdataOffset = br.ReadUINT32();
    }



};