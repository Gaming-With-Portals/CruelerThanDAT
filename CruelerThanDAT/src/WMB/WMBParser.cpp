#include "pch.hpp"
#include "WMB/WMBParser.h"
#include "Log.h"


glm::vec4 DecodeWMB4TypeTangent(uint32_t packedTangent) {

	uint8_t tx = (packedTangent >> 0) & 0xFF;
	uint8_t ty = (packedTangent >> 8) & 0xFF;
	uint8_t tz = (packedTangent >> 16) & 0xFF;
	uint8_t tw = (packedTangent >> 24) & 0xFF;

	auto decodeByte = [](uint8_t v) -> float {
		return (static_cast<float>(v) - 127.0f) / 127.0f;
		};

	glm::vec4 tangent;
	tangent.x = decodeByte(tx);
	tangent.y = decodeByte(ty);
	tangent.z = decodeByte(tz);
	tangent.w = decodeByte(tw);

	return tangent;
}

glm::vec3 DecodeWMB4TypeNormal(uint32_t packed) {
	int nx = packed & ((1 << 11) - 1);
	int ny = (packed >> 11) & ((1 << 11) - 1);
	int nz = (packed >> 22) & ((1 << 10) - 1);

	if (nx & (1 << 10)) nx = -(int)((1 << 11) - nx);
	if (ny & (1 << 10)) ny = -(int)((1 << 11) - ny);
	if (nz & (1 << 9))  nz = -(int)((1 << 10) - nz);

	float fx = (float)nx / ((1 << 10) - 1);
	float fy = (float)ny / ((1 << 10) - 1);
	float fz = (float)nz / ((1 << 9) - 1);

	float len = sqrtf(fx * fx + fy * fy + fz * fz);
	return { fx / len, fy / len, fz / len };
}



// For Big Endian
std::vector<uint16_t> UnwindIndices(std::vector<uint16_t> indices) {
	std::vector<unsigned short> chain;
	std::vector<unsigned short> newIndices;
	bool reverse = false;
	for (unsigned short& indice : indices) {
		if (indice == 0xFFFF) { // flip direction
			chain.clear();
			reverse = false;
			continue;
		}
		chain.push_back(indice);
		if (chain.size() > 3) {
			chain.erase(chain.begin());
		}
		if (chain.size() == 3) {
			if (reverse) {
				std::reverse(chain.begin(), chain.end());
			}
			newIndices.insert(newIndices.end(), chain.begin(), chain.end());
			if (reverse) {
				std::reverse(chain.begin(), chain.end());
			}
			reverse = !reverse;

		}
	}

	return newIndices;
}



ParseResult<WmbModel> WMB4Parser::Parse(BinaryReader& reader)
{
	WmbModel model = WmbModel();

	reader.Seek(0); // confirm we are at the start

	if (reader.GetSize() <= 112) {
		return ParseError{ "File too small to faciliate a proper WMB4 header", 0 };
	}

	if (reader.ReadString(4) != "WMB4") {
		return ParseError{ "Magic mismatch, not a WMB4 file", 0 };
	}


	WMB4Header header = WMB4Header();
	header.Read(reader);

	if (header.cutdataOffset != 0) {
		model.has_cutdata = true;
	}

	reader.Seek(header.offsetVertexGroups);
	std::vector<WMB4VertexGroup> vertexGroups;
	for (uint32_t i = 0; i < header.numVertexGroups; i++) {
		WMB4VertexGroup vtxGroup;
		vtxGroup.Read(reader);
		vertexGroups.push_back(vtxGroup);
	}

	reader.Seek(header.offsetBatches);
	std::vector<WMB4Batch> batches;
	for (uint32_t i = 0; i < header.numBatches; i++) {
		WMB4Batch itm;
		itm.Read(reader);
		batches.push_back(itm);
	}

	reader.Seek(header.offsetBatchDescription);
	unsigned int offsetBatchData = reader.ReadUINT32();

	reader.Seek(offsetBatchData);
	std::vector<WMB4BatchData> batchDatas;
	for (uint32_t i = 0; i < header.numBatches; i++) {
		WMB4BatchData itm;
		itm.Read(reader);
		batchDatas.push_back(itm);
	}

	reader.Seek(header.offsetMeshes);
	std::vector<WMB4Mesh> meshes;
	for (uint32_t i = 0; i < header.numMeshes; i++) {
		WMB4Mesh itm;
		itm.Read(reader);
		meshes.push_back(itm);
	}

	reader.Seek(header.offsetTextures);
	std::vector<WMB4Texture> textures;
	for (uint32_t i = 0; i < header.numTextures; i++) {
		WMB4Texture itm;
		itm.Read(reader);
		textures.push_back(itm);
	}

	reader.Seek(header.offsetMaterials);
	// TODO - materials

	// also TODO - bones

	for (WMB4Mesh& mesh : meshes) {
		WmbMesh fmesh = WmbMesh();

		reader.Seek(mesh.offsetName);
		fmesh.name = reader.ReadNullTerminatedString();
		
		std::vector<unsigned short> BatchIds;

		reader.Seek(mesh.offsetBatches);
		BatchIds = reader.ReadUINT16Array(mesh.numBatches);

		for (unsigned short meshBatchId : BatchIds) {
			std::unique_ptr<WmbBatch> fbatch = std::make_unique<WmbBatch>();

			WMB4Batch& curBatch = batches[meshBatchId];
			WMB4BatchData& curBatchData = batchDatas[meshBatchId];
			WMB4VertexGroup& curVtxGroup = vertexGroups[curBatch.vertexGroupIndex];

			fbatch->index_count = curBatch.numIndices;
			fbatch->vertex_count = curBatch.numVertices;

			// -- Indices -- 
			reader.Seek(curVtxGroup.offsetIndexes + (curBatch.indexStart * 2));
			std::vector<uint16_t> indices = reader.ReadUINT16Array(curBatch.numIndices);

			if (reader.IsBig()) {
				indices = UnwindIndices(indices); // xbox 360 only optimization for some reason, you would think it would be on the pc version
			}

			std::vector<uint32_t> fixedWidthIndices;
			fixedWidthIndices.reserve(indices.size());

			for (uint16_t val : indices) {
				fixedWidthIndices.push_back(static_cast<uint32_t>(val));
			}

			fbatch->indexes = fixedWidthIndices;


			// -- Vertices --
			std::vector<CtdVertex> fvertices;
			fvertices.reserve(curBatch.numVertices);



			
		}



	}



	return model;
}
