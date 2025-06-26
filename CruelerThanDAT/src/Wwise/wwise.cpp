#include "pch.hpp"
#include "Wwise/wwise.h"


void WWISE::Data002BlobData::Load()
{
	std::vector<char> fileData;
	if (!ReadFileIntoVector("Assets/System/data002.bin", fileData)) {
		return;
	}

	BinaryReader reader = BinaryReader(fileData);
	unsigned int count = reader.ReadUINT32();
	for (unsigned int i = 0; i < count; i++) {
		uint32_t uid = reader.ReadUINT32();
		uint32_t offset = reader.ReadUINT32();
		unsigned int position = reader.Tell();
		reader.Seek(offset);
		wwiseHircObjectIDs[uid] = reader.ReadNullTerminatedString();
		reader.Seek(position);
	}


}
