#pragma once
#include <pch.hpp>
#include <FileNodes.h>

FileNode* HelperFunction::LoadNode(std::string fileName, const std::vector<char>& data, bool forceEndianess, bool bigEndian);
void DX9WTAWTPLoad(CruelerContext *ctx, BinaryReader& WTA, BinaryReader& WTP);