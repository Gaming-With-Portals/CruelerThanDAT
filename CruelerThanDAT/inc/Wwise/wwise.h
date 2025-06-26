#pragma once
#ifndef WWISE_H
#define WWISE_H
#include "pch.hpp"
#include "../BinaryHandler.h"
#include "FileUtils.h"


namespace WWISE {
	class Data002BlobData {
	public:
		std::unordered_map<int, std::string> wwiseHircObjectIDs;

		void Load();


	};
}

#endif