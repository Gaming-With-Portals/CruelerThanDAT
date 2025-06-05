#include "pch.hpp"
#include "CTDSettings.h"

#include "BinaryHandler.h"
#include "FileUtils.h"

void CTDSettings::Write() {
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

void CTDSettings::Read() {
	std::vector<char> fileData;
	if (!ReadFileIntoVector("config.ctd", fileData)) {
		Write();
		return;
	}

	BinaryReader rr = BinaryReader(fileData);

	AutomaticallyLoadTextures = rr.ReadUINT16();
	ShowAllMeshesByDefault = rr.ReadUINT16();
}
