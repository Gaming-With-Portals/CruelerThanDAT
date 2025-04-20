#pragma once
#include <stdint.h>
#include <DirectXMath.h>

namespace CruelerThanDAT {


	struct WMBHeader {
		char id[4];
		uint32_t u_a;
		uint32_t vertexFmt;
		uint32_t u_b;
		uint32_t u_c;
		DirectX::XMFLOAT3 pos1;
		DirectX::XMFLOAT3 pos2;
		uint32_t offsetVertexGroups;
		uint32_t numVertexGroups;
		uint32_t offsetBatches;
		uint32_t numBatches;
		uint32_t offsetBatchDescriptions;
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

	struct WMBVertexGroup {
		uint32_t offsetVertexes;
		uint32_t offsetVertexesExData;
		uint32_t unknownOffsetA;
		uint32_t unknownOffsetB;
		uint32_t numVertex;
		uint32_t offsetIndexes;
		uint32_t numIndexes;

	};

	struct WMBVertex {
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT2 texCoord;
		DirectX::XMFLOAT3 texCoord;

	};



}