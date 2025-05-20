#pragma once
#include "pch.hpp"

struct MGRVector {
	float x;
	float y;
	float z;

	operator float* () {
		return &x;
	}
};

struct MGRColor {
	float r;
	float g;
	float b;
	float a;

	operator float* () {
		return &r;
	}
};