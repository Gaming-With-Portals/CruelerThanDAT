#pragma once
#include "pch.hpp"
#include "WMB.h"

#include "BinaryHandler.h"

void WMBVertexA::Read(BinaryReader& br) {
	position.x = br.ReadFloat();
	position.y = br.ReadFloat();
	position.z = br.ReadFloat();
	uv.u = br.ReadUINT16();
	uv.v = br.ReadUINT16();
	normals = br.ReadUINT32();
	tangents = br.ReadUINT32();
}

void WMBVertex65847::Read(BinaryReader& br) {
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

void WMBVertex66311::Read(BinaryReader& br) {
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

void WMBVertex65799::Read(BinaryReader& br) {
	position.x = br.ReadFloat();
	position.y = br.ReadFloat();
	position.z = br.ReadFloat();
	uv.u = br.ReadUINT16();
	uv.v = br.ReadUINT16();
	normals = br.ReadUINT32(); // TODO: Deal with
	tangents = br.ReadUINT32(); // TODO: Ignore
	color = br.ReadUINT32();
}

void WMBBatch::Read(BinaryReader& br) {
	vertexGroupIndex = br.ReadUINT32();
	vertexStart = br.ReadINT32();
	indexStart = br.ReadINT32();
	numVertices = br.ReadUINT32();
	numIndices = br.ReadUINT32();
}

void WMBBatchData::Read(BinaryReader& br) {
	batchIndex = br.ReadUINT32();
	meshIndex = br.ReadUINT32();
	materialIndex = br.ReadUINT16();
	boneSetsIndex = br.ReadUINT16();
	u_a = br.ReadUINT32();
}

void WMBMaterial::Read(BinaryReader& br) {
	offsetShaderName = br.ReadUINT32();
	offsetTextures = br.ReadUINT32();
	u_a = br.ReadUINT32();
	offsetParameters = br.ReadUINT32();
	u_b = br.ReadUINT16();
	numTextures = br.ReadUINT16();
	u_d = br.ReadUINT16();
	numParameters = br.ReadUINT16();
}

void WMBTexture::Read(BinaryReader& br) {
	flag = br.ReadUINT32();
	id = br.ReadUINT32();
}

void WMBBoundingBox::Read(BinaryReader& br) {
	a = br.ReadFloat();
	b = br.ReadFloat();
	c = br.ReadFloat();
	d = br.ReadFloat();
	e = br.ReadFloat();
	f = br.ReadFloat();
}

void WMBMesh::Read(BinaryReader& br) {
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

void WMBVertexGroup::Read(BinaryReader& br) {
	offsetVertexes = br.ReadUINT32();
	offsetVertexesExData = br.ReadUINT32();
	u_a = br.ReadUINT32();
	u_b = br.ReadUINT32();
	numVertexes = br.ReadUINT32();
	offsetIndexes = br.ReadUINT32();
	numIndexes = br.ReadUINT32();
}

void WMBHeader::Read(BinaryReader& br) {
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
