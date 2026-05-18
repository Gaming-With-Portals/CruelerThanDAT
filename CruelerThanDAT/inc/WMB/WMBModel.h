#pragma once
#include "WMBConstants.h"
#include "ErrorHandling.h"
#include "glm/glm.hpp"
#include <memory>
#include <vector>

enum PG_TEXTURE_TYPE {
	ALBEDO_0,
	ALBEDO_1,
	NORMAL,
	LIGHTMAP,
	SUBSURFACE_SCATTERING,
	CUBEMAP

};


class CtdVertex {
public:
	glm::vec3 position = { 0, 0, 0 };
	glm::vec2 uv_0 = { 0, 0 }; // main
	glm::vec2 uv_1 = { 0, 0 }; // mgrr: lightmap
	glm::vec2 uv_2 = { 0, 0 }; // mgrr: i have no clue but it supports it
	glm::vec4 tangent = { 0, 0, 0, 0 };
	glm::vec3 normal = { 0, 0, 0 };
	uint32_t color = 0;
	uint32_t color_2 = 0; // used for blending
	uint8_t bones[4] = { 0, 0, 0, 0 };
	uint8_t weights[4] = { 0, 0, 0, 0 };
};

class WmbMaterial {
	
};

class WmbBatch {
public:
	uint32_t material_id;
	uint32_t vertex_count;
	uint32_t index_count;
	std::vector<CtdVertex> vertexes;
	std::vector<uint32_t> indexes;
};

class WmbMesh {
public:
	std::string name;
	std::vector<std::unique_ptr<WmbBatch>> batches;
	bool visible = true;

};

class WmbCutData {
public:

};


class WmbModel {
private:
	std::vector<WmbMesh> meshes;
	std::vector<WmbMaterial> materials;
	WmbCutData cutdata; // only for MGR:R models

public:
	PG_MODEL_TYPE type;
	bool has_cutdata;


};