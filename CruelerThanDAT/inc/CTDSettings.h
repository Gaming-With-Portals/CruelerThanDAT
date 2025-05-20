#pragma once
#include "pch.hpp"

#include "BinaryHandler.h"
#include "FileUtils.h"

class CTDSettings {
public:
	bool AutomaticallyLoadTextures = false;
	bool ShowAllMeshesByDefault = true;

	void Write() {
		BinaryWriter rw = new BinaryWriter();
		rw.WriteString("CTDS");
		rw.WriteUINT16(AutomaticallyLoadTextures);
		rw.WriteUINT16(ShowAllMeshesByDefault);


		std::ofstream file("config.ctd", std::ios::binary);
		if (file) {
			file.write(rw.GetData().data(), (rw.GetData().size()));
			file.close();
		}
	}

	void Read() {
		std::vector<char> fileData;
		if (!ReadFileIntoVector("config.ctd", fileData)) {
			Write();
			return;
		}

		BinaryReader rr = BinaryReader(fileData);

		AutomaticallyLoadTextures = rr.ReadUINT16();
		ShowAllMeshesByDefault = rr.ReadUINT16();

	}

};