#pragma once
#include <FileNodes.h>
#include <string>

std::vector<FileNode*> openFiles;

FileNode* HelperFunction::LoadNode(std::string fileName, const std::vector<char>& data, bool forceEndianess, bool bigEndian);
void DX9WTAWTPLoad(BinaryReader& WTA, BinaryReader& WTP);