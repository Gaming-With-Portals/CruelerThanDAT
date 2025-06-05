#pragma once
#include "pch.hpp"

class CTDSettings {
public:
	bool AutomaticallyLoadTextures = false;
	bool ShowAllMeshesByDefault = true;

	void Write();
	void Read();
};