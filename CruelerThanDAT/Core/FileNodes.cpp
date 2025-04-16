#include "FileNodes.h"

FileNode* FileNode::selectedNode = nullptr;

void closeNode(FileNode* target) {
    auto it = std::find(openFiles.begin(), openFiles.end(), target);
    if (it != openFiles.end()) { // Ensure it exists before erasing
        openFiles.erase(it);
    }
}


int IntLength(int value) {
	int length = 0;
	while (value > 0) {
		value >>= 1;
		length++;
	}
	return length;
}

int HelperFunction::Align(int value, int alignment) {
	return (value + (alignment - 1)) & ~(alignment - 1);
}