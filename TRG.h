#pragma once
#include <cstdint>
#include <string>
#include "BinaryHandler.h"
// so it begins...

struct cActSceneMovie {
	uint32_t phaseID; // Hex
	std::string subphaseID;
	uint32_t usmID; // Hex

	void Read(BinaryReader& br) {
		phaseID = br.ReadUINT32();
		subphaseID = br.ReadString(32);
		usmID = br.ReadUINT32();
	}

};

struct cActCameraAngle {
	float angle; // ????

	void Read(BinaryReader& br) {
		angle = br.ReadFloat();
	}
};

struct cActCameraAngleOff { }; // No Args

struct cActCodecStart {
	std::string codecID;

	void Read(BinaryReader& br) {
		codecID = br.ReadString(16);
	}

};

struct cActTutorialStart {
	uint32_t tutorialID;
	uint32_t unknown;

	void Read(BinaryReader& br) {
		tutorialID = br.ReadUINT32();
		unknown = br.ReadUINT32();
	}

};

struct cActTutorialEnd {}; // No Args