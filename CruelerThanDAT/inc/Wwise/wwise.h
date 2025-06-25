#pragma once
#include "pch.hpp"
#include "../BinaryHandler.h"



namespace WWISE {
	class Data002BlobData {
		std::unordered_map<int, std::string> wwiseHircObjectIDs;

		void Load();


	};
}