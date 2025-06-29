#include "pch.hpp"
#include "FileNodes.h"
const FbxSystemUnit fbxsdk::FbxSystemUnit::m(100.0);

FileNode* FileNode::selectedNode = nullptr;

void closeNode(FileNode* target) {
	FileNode::selectedNode = nullptr;
    auto it = std::find(openFiles.begin(), openFiles.end(), target);
    if (it != openFiles.end()) { // Ensure it exists before erasing
        openFiles.erase(it);
    }

	delete target;
}


int IntLength(int value) {
	int length = 0;
	while (value > 0) {
		value >>= 1;
		length++;
	}
	return length;
}


bool HelperFunction::WriteVectorToFile(const std::vector<char> dataVec, const std::string& filename) {
	std::string fullPath = filename;

	std::ofstream out(fullPath, std::ios::binary);
	if (!out) return false;

	out.write(dataVec.data(), dataVec.size());
	return out.good();
}

std::vector<std::string> BXMInternal::SplitString(const std::string& str, char delimiter) {
	std::vector<std::string> result;
	std::string current;

	for (char c : str) {
		if (c == delimiter) {
			if (!current.empty()) {
				result.push_back(current);
				current.clear();
			}
		}
		else {
			current += c;
		}
	}
	if (!current.empty()) {
		result.push_back(current);
	}

	return result;
}

int HelperFunction::Align(int value, int alignment) {
	return (value + (alignment - 1)) & ~(alignment - 1);
}

glm::vec4 HelperFunction::DecodeTangent(uint32_t packedTangent) {

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

MGRVector HelperFunction::DecodeNormal(uint32_t packed) {
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

float HelperFunction::HalfToFloat(uint16_t h) {
	uint16_t h_exp = (h & 0x7C00) >> 10;  // exponent
	uint16_t h_sig = h & 0x03FF;          // significand
	uint32_t f_sgn = (h & 0x8000) << 16;  // sign bit

	uint32_t f_exp, f_sig;

	if (h_exp == 0) {
		// Zero / Subnormal
		if (h_sig == 0) {
			f_exp = 0;
			f_sig = 0;
		}
		else {
			// Normalize it
			h_exp = 1;
			while ((h_sig & 0x0400) == 0) {
				h_sig <<= 1;
				h_exp--;
			}
			h_sig &= 0x03FF;
			f_exp = (127 - 15 + h_exp) << 23;
			f_sig = h_sig << 13;
		}
	}
	else if (h_exp == 0x1F) {
		// Inf / NaN
		f_exp = 0xFF << 23;
		f_sig = h_sig << 13;
	}
	else {
		f_exp = (h_exp - 15 + 127) << 23;
		f_sig = h_sig << 13;
	}

	uint32_t f = f_sgn | f_exp | f_sig;
	float result;
	std::memcpy(&result, &f, sizeof(result));
	return result;
}